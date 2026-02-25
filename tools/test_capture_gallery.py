import binascii
import contextlib
import io
import json
import re
import tempfile
import unittest
from collections import Counter
from pathlib import Path

from tools import capture_gallery, gallery_catalog


def read_sources(repository, *relative_paths):
    return "\n".join((repository / path).read_text() for path in relative_paths)


def protocol_stream(scene_ids, bitmaps, *, width=8, height=1, corrupt_crc=False):
    stream = bytearray(b"ESP-ROM boot message\r\n@WATCHY_GALLERY 1\r\n")
    for sequence, (scene_id, bitmap) in enumerate(zip(scene_ids, bitmaps), 1):
        checksum = binascii.crc32(bitmap) & 0xFFFFFFFF
        if corrupt_crc and sequence == 1:
            checksum ^= 1
        stream.extend(
            f"@WATCHY_FRAME 1 {sequence} {width} {height} "
            f"{len(bitmap)} {checksum:08X} {scene_id}\n".encode()
        )
        stream.extend(bitmap)
        stream.extend(f"\n@WATCHY_END {sequence}\n".encode())
    stream.extend(f"@WATCHY_DONE {len(scene_ids)} {len(scene_ids)}\n".encode())
    return bytes(stream)


class CaptureGalleryTests(unittest.TestCase):
    def parse(self, payload, scene_ids):
        reader = capture_gallery.TimedReader(io.BytesIO(payload), False, 1.0)
        return capture_gallery.parse_capture(reader, scene_ids, 8, 1)

    def test_valid_stream_writes_inverted_pbm_png_and_manifest(self):
        scene_ids = ("demo/black/light", "demo/white/light")
        frames = self.parse(protocol_stream(scene_ids, (b"\x00", b"\xFF")), scene_ids)
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "gallery"
            capture_gallery.write_gallery(frames, output)
            pbm = (output / "pbm/demo/black/light.pbm").read_bytes()
            png = (output / "png/demo/black/light.png").read_bytes()
            manifest = json.loads((output / "manifest.json").read_text())
            self.assertTrue(pbm.endswith(b"\xFF"))
            self.assertTrue(png.startswith(b"\x89PNG\r\n\x1a\n"))
            self.assertEqual(manifest["frame_count"], 2)
            self.assertEqual(manifest["frames"][1]["id"], scene_ids[1])

    def test_corrupt_frame_is_rejected(self):
        with self.assertRaisesRegex(capture_gallery.CaptureError, "CRC32 mismatch"):
            self.parse(protocol_stream(("demo/frame/light",), (b"\xAA",), corrupt_crc=True),
                       ("demo/frame/light",))

    def test_battery_contract_uses_configured_range_and_capacity(self):
        repository = Path(__file__).resolve().parents[1]
        config = (repository / "src/sdk/include/System/config.h").read_text()
        watchface = (
            repository / "src/watchfaces/7_SEG/Watchy_7_SEG.cpp"
        ).read_text()

        definitions = dict(
            re.findall(
                r"#define\s+(WATCHY_(?:DEFAULT_BATTERY_CAPACITY_MAH|"
                r"BATTERY_(?:EMPTY|FULL)_VOLTAGE))\s+([0-9.]+)f?",
                config,
            )
        )
        self.assertEqual(definitions["WATCHY_DEFAULT_BATTERY_CAPACITY_MAH"], "200.0")
        self.assertEqual(definitions["WATCHY_BATTERY_EMPTY_VOLTAGE"], "2.65")
        self.assertEqual(definitions["WATCHY_BATTERY_FULL_VOLTAGE"], "3.95")
        self.assertIn("WatchyBattery::estimate(watch.getBatteryVoltage())", watchface)

    def test_bma423_background_modes_follow_low_power_contract(self):
        repository = Path(__file__).resolve().parents[1]
        watchy = (repository / "src/sdk/WatchyCore.cpp").read_text()
        fall_detection = (
            repository / "src/app/FallDetection.cpp"
        ).read_text()
        heart_rate = (repository / "src/app/ShowHeartRate.cpp").read_text()
        sensor_manager = (repository / "src/app/SensorManager.cpp").read_text()

        self.assertIn("BMA4_OUTPUT_DATA_RATE_50HZ", sensor_manager)
        self.assertIn("BMA4_CIC_AVG_MODE", sensor_manager)
        self.assertIn("bma4_set_advance_power_save(BMA4_ENABLE", sensor_manager)
        self.assertIn("configureAccelFifo(true, 1)", sensor_manager)
        self.assertIn("bma4_set_advance_power_save(BMA4_DISABLE", sensor_manager)
        self.assertIn("BMA4_OUTPUT_DATA_RATE_100HZ", sensor_manager)
        self.assertIn("WatchySensor::Mode::ForegroundHeartRate", heart_rate)
        self.assertIn("bma423_feature_enable(BMA423_STEP_CNTR", sensor_manager)
        self.assertNotIn("enableFeature(BMA423_TILT, true)", watchy)
        self.assertNotIn("enableStepCountInterrupt()", watchy)
        self.assertNotIn("enableTiltInterrupt()", watchy)

    def test_power_architecture_avoids_periodic_idle_work(self):
        repository = Path(__file__).resolve().parents[1]
        watchy = (repository / "src/sdk/WatchyCore.cpp").read_text()
        display = (repository / "src/Display.cpp").read_text()
        ui = (repository / "src/sdk/WatchyUi.cpp").read_text()
        heart_rate = (repository / "src/app/ShowHeartRate.cpp").read_text()
        sensor_manager = (repository / "src/app/SensorManager.cpp").read_text()
        time_tools = read_sources(
            repository,
            "src/sdk/tools/Time/TimeSupport.cpp",
            "src/sdk/tools/Time/DailyAlarm.cpp",
            "src/sdk/tools/Time/Metronome.cpp",
        )

        self.assertNotIn("initWatchy(wakeup_reason", watchy)
        self.assertIn("if (!display.epd2.initializedThisWake())", watchy)
        self.assertIn("Watchy::ensureDisplayInitialized();", ui)
        self.assertNotIn("SPI.begin", display.split("WatchyDisplay::WatchyDisplay()", 1)[1]
                         .split("void WatchyDisplay::initWatchy", 1)[0])
        self.assertIn("bcgFifoWatermarkBytes", sensor_manager)
        self.assertIn("BMA4_FIFO_WM_INT", sensor_manager)
        self.assertNotIn("HEART_RATE_BACKGROUND_WAKE_SECONDS", watchy)
        self.assertIn("uint8_t secondsToNextWake = 60 - second", watchy)
        boot_runtime = watchy.split("void beginBootRuntime()", 1)[1].split(
            "void normalizeMenuStateAfterBoot", 1
        )[0]
        self.assertIn("RUNTIME_CPU_FREQUENCY_MHZ = 240", watchy)
        self.assertIn("setCpuFrequencyMhz(RUNTIME_CPU_FREQUENCY_MHZ)",
                      boot_runtime)
        self.assertNotIn("setLowPowerCpuFrequency()", boot_runtime)
        cold_boot = watchy.split("void handleColdBoot(String datetime) {", 1)[1].split(
            "void stopRuntimeForDeepSleep", 1
        )[0]
        self.assertLess(cold_boot.index("showColdBootWatchFace();"),
                        cold_boot.index("bool sensorReady = bmaConfig();"))
        self.assertLess(cold_boot.index("vibMotor(75, 4);"),
                        cold_boot.index("bool sensorReady = bmaConfig();"))
        self.assertNotIn("connectWiFi();\n    WiFi.disconnect", watchy)
        self.assertNotIn("Input::poll()", time_tools)
        self.assertNotIn("deepSleepDelay(20)", time_tools)

    def test_back_interrupts_application_delays(self):
        repository = Path(__file__).resolve().parents[1]
        header = (repository / "src/sdk/include/Sdk/WatchyUi.h").read_text()
        config = (repository / "src/sdk/include/System/config.h").read_text()
        watchy = (repository / "src/sdk/WatchyCore.cpp").read_text()
        ui = (repository / "src/sdk/WatchyUi.cpp").read_text()
        healthcare = (
            repository / "src/sdk/tools/Healthcare/HealthcareSupport.cpp"
        ).read_text()
        reminders = (repository / "src/app/ShowHealthReminders.cpp").read_text()
        themes = (repository / "src/app/ShowThemeColours.cpp").read_text()
        watchfaces = (repository / "src/sdk/os/ShowWatchFaceSelector.cpp").read_text()
        delay = ui.split("WakeupReason deepSleepDelay", 1)[1].split(
            "Event Input::poll", 1
        )[0]
        poll = ui.split("Event Input::poll", 1)[1].split(
            "Event Input::wait", 1
        )[0]

        self.assertIn("BACK_PRESSED", header)
        self.assertIn("enum class Event : uint64_t", header)
        self.assertIn("UP = UP_BTN_MASK", header)
        self.assertIn("DOWN = DOWN_BTN_MASK", header)
        self.assertIn("MENU = MENU_BTN_MASK", header)
        self.assertIn("BACK = BACK_BTN_MASK", header)
        self.assertIn("NONE = 0", header)
        self.assertIn("#define MENU_BTN_MASK (BIT64(MENU_BTN_PIN))", config)
        self.assertIn("#define BACK_BTN_MASK (BIT64(BACK_BTN_PIN))", config)
        self.assertIn("#define UP_BTN_MASK   (BIT64(UP_BTN_PIN))", config)
        self.assertIn("#define DOWN_BTN_MASK (BIT64(DOWN_BTN_PIN))", config)
        self.assertIn(
            "wake.ext1Bits = esp_sleep_get_ext1_wakeup_status();", watchy
        )
        self.assertIn(
            "if (wake.cause == ESP_SLEEP_WAKEUP_EXT1) {", watchy
        )
        self.assertIn("handleButtonWake(wake.ext1Bits);", watchy)
        self.assertIn("enableBackWakeup()", delay)
        self.assertIn("ESP_SLEEP_WAKEUP_GPIO", delay)
        self.assertIn("waitForBackNotification(durationMs)", delay)
        self.assertIn("notifyBackFromIsr, &context, FALLING", ui)
        self.assertIn("return WakeupReason::BACK_PRESSED;", ui)
        self.assertIn("notifyBackFromIsr", ui)
        self.assertIn("rememberDelayedBack()", delay)
        self.assertIn("xTaskGetCurrentTaskHandle() != inputTask", delay)
        core = (repository / "src/sdk/WatchyCore.cpp").read_text()
        button_press = core.split("void handleButtonPress(uint64_t wakeupBits)", 1)[1].split(
            "void handleButtonEvent", 1
        )[0]
        button_handler = core.split(
            "void handleButtonEvent(WatchyUi::Event event) {", 1
        )[1].split(
            "void selectMenuEntry", 1
        )[0]
        self.assertIn("esp_sleep_get_ext1_wakeup_status()", button_press)
        self.assertIn("readButtonEvent(wakeupBits)", button_press)
        self.assertIn("Input::wait(remaining)", button_press)
        self.assertIn("handleButtonPress(wakeupBits);", core)
        self.assertIn("if (guiState == WATCHFACE_STATE)", button_handler)
        self.assertIn("showMenu(menuIndex, false);", button_handler)
        self.assertIn(
            "if (guiState == APP_STATE || guiState == FW_UPDATE_STATE)",
            button_handler,
        )
        self.assertIn("if (guiState != MAIN_MENU_STATE) return;", button_handler)
        self.assertNotIn("Event nextEvent = readButtonEvent()", button_handler)
        self.assertIn("consumeDelayedBack()", poll)
        self.assertIn("debounceWake == WakeupReason::BACK_PRESSED", poll)
        self.assertIn("WakeupReason::BACK_PRESSED", healthcare)
        self.assertEqual(reminders.count("WakeupReason::BACK_PRESSED"), 2)
        self.assertIn("WakeupReason::BACK_PRESSED", themes)
        self.assertIn("WakeupReason::BACK_PRESSED", watchfaces)

    def test_display_busy_wait_cannot_hide_the_driver_timeout(self):
        repository = Path(__file__).resolve().parents[1]
        ui = (repository / "src/sdk/WatchyUi.cpp").read_text()
        display = (repository / "src/Display.cpp").read_text()
        busy_wait = ui.split("void Power::waitForDisplayReady()", 1)[1].split(
            "WakeupReason deepSleepDelay", 1
        )[0]

        self.assertIn("displayBusyWaitSliceMs = 50", ui)
        self.assertIn("bool Power::waitForDisplayReady(uint32_t timeoutMs)", busy_wait)
        self.assertIn("pdMS_TO_TICKS(displayBusyWaitSliceMs)", busy_wait)
        self.assertIn("return false", busy_wait)
        self.assertNotIn("pdMS_TO_TICKS(3000)", busy_wait)
        self.assertNotIn("esp_sleep_enable_timer_wakeup", busy_wait)
        watchy = (repository / "src/sdk/WatchyCore.cpp").read_text()
        display_wait = watchy.split(
            "bool waitForDisplayIdleBeforeDeepSleep()", 1
        )[1].split("void hibernateDisplayForDeepSleep", 1)[0]
        self.assertIn("WatchyDisplay::full_refresh_time", display_wait)
        self.assertIn("static_cast<int32_t>(deadline - now) > 0", display_wait)
        self.assertIn("WatchyUi::Power::waitForDisplayReady(remaining);", display_wait)

    def test_s3_runtime_buttons_restore_gpio_mux(self):
        repository = Path(__file__).resolve().parents[1]
        ui = (repository / "src/sdk/WatchyUi.cpp").read_text()
        begin = ui.split("void Input::begin()", 1)[1].split(
            "void Input::setAuxiliaryWakeSource", 1
        )[0]

        self.assertIn("inputTask = xTaskGetCurrentTaskHandle();", begin)
        self.assertIn("rtc_gpio_deinit(pin);", begin)
        self.assertLess(begin.index("rtc_gpio_deinit(pin);"),
                        begin.index("pinMode(pin, INPUT_PULLUP);"))

    def test_usb_presence_blocks_sleep_and_keeps_buttons_serviced(self):
        repository = Path(__file__).resolve().parents[1]
        ui = (repository / "src/sdk/WatchyUi.cpp").read_text()
        idle = ui.split("void Power::idle", 1)[1].split(
            "bool Power::usbPluggedIn", 1
        )[0]
        self.assertIn("if (usbPluggedIn())", idle)
        self.assertIn("usbPollIntervalMs = 100", idle)
        self.assertIn("durationMs == UINT32_MAX", idle)
        self.assertIn("waitForButtonNotification(waitDurationMs);", idle)
        self.assertIn("gpio_wakeup_enable(static_cast<gpio_num_t>(USB_DET_PIN)", ui)

        core = (repository / "src/sdk/WatchyCore.cpp").read_text()
        runtime = core.split("void serviceUsbRuntime()", 1)[1].split(
            "void deepSleep()", 1
        )[0]
        deep_sleep = core.split("void deepSleep()", 1)[1]
        self.assertIn("Input::waitScheduled(100)", runtime)
        self.assertIn("serviceUsbRuntime();", deep_sleep)
        self.assertIn("configureUsbWakeForDeepSleep()", core)
        self.assertIn(
            "configureRtcWakeInput(static_cast<gpio_num_t>(USB_DET_PIN))",
            core,
        )
        self.assertIn("configureRtcWakeInput(static_cast<gpio_num_t>(ACC_INT_2_PIN))",
                  core)
        self.assertIn("restoreRuntimeWakePins();", deep_sleep)
        self.assertIn("if (configureWakeSourcesForDeepSleep())", deep_sleep)

    def test_sensor_wake_uses_the_configured_interrupt_two_line(self):
        repository = Path(__file__).resolve().parents[1]
        fall = (repository / "src/app/FallDetection.cpp").read_text()
        heart_rate = (repository / "src/app/ShowHeartRate.cpp").read_text()
        core = (repository / "src/sdk/WatchyCore.cpp").read_text()

        fall_mask = fall.split("uint64_t wakeMask()", 1)[1].split(
            "void exportTraces", 1
        )[0]
        heart_mask = heart_rate.split(
            "uint64_t watchfaceHeartRateWakeMask()", 1
        )[1].split("void showHeartRateImpl", 1)[0]
        self.assertIn("ACC_INT_2_MASK", fall_mask)
        self.assertIn("ACC_INT_2_MASK", heart_mask)
        self.assertIn("ACC_INT_MASK | ACC_INT_2_MASK", core)

    def test_every_app_dispatch_initializes_semantic_input(self):
        repository = Path(__file__).resolve().parents[1]
        watchy = (repository / "src/sdk/WatchyCore.cpp").read_text()
        dispatch = watchy.split("void selectMenuEntry() {", 1)[1].split(
            "void leaveMenuLevel() {", 1
        )[0]
        action = dispatch.index("MenuAction action =")
        begin = dispatch.index("WatchyUi::Input::begin();")
        switch = dispatch.index("switch (action)")

        self.assertLess(action, begin)
        self.assertLess(begin, switch)

        menu = (repository / "src/sdk/os/ShowMenu.cpp").read_text()
        show_menu = menu.split("void showMenuImpl", 1)[1].split(
            "void showFastMenuImpl", 1
        )[0]
        present = show_menu.index("WatchyUi::Screen::present")
        reset = show_menu.index("WatchyUi::Input::begin();")
        self.assertLess(present, reset)

    def test_ble_scans_are_bounded_and_back_cancelable(self):
        repository = Path(__file__).resolve().parents[1]
        bluetooth = (
            repository / "src/sdk/tools/Bluetooth/BluetoothSupport.cpp"
        ).read_text()
        scan = bluetooth.split("ScanOutcome scanNearby", 1)[1].split(
            "int limitedCount", 1
        )[0]

        self.assertIn("scanDeadlineMs = 5000", bluetooth)
        self.assertIn("scan->start(scanDurationSeconds, scanComplete", scan)
        self.assertIn("Input::waitNotified", scan)
        self.assertIn("Event::BACK", scan)
        self.assertIn("ScanOutcome::Cancelled", scan)
        self.assertIn("scan->stop()", scan)
        self.assertNotIn("return scan->start(3, false)", bluetooth)

    def test_wifi_survey_is_bounded_and_back_cancelable(self):
        repository = Path(__file__).resolve().parents[1]
        survey = (repository / "src/app/ShowWifiSurvey.cpp").read_text()

        self.assertIn("wifiScanDeadlineMs = 8000", survey)
        self.assertIn("WiFi.scanNetworks(true", survey)
        self.assertIn("WiFi.scanComplete()", survey)
        self.assertIn("WatchyUi::Input::wait(wifiScanPollMs)", survey)
        self.assertIn("WatchyUi::Event::BACK", survey)
        self.assertIn("esp_wifi_scan_stop()", survey)
        self.assertNotIn("WiFi.scanNetworks(false", survey)

    def test_astronomy_visual_summaries_preserve_detail_data(self):
        repository = Path(__file__).resolve().parents[1]
        sun = (repository / "src/app/ShowSunRise.cpp").read_text()
        moon_rise = (repository / "src/app/ShowMoonRise.cpp").read_text()
        moon_phase = (repository / "src/app/ShowMoonPhase.cpp").read_text()

        self.assertIn("drawSunRiseDetails", sun)
        self.assertIn("sun.queryTime", sun)
        self.assertIn("sun.riseAz", sun)
        self.assertIn("sun.setAz", sun)
        self.assertIn("sun.isVisible", sun)
        self.assertIn("drawMoonRiseDetails", moon_rise)
        self.assertIn("moon.queryTime", moon_rise)
        self.assertIn("moon.riseAz", moon_rise)
        self.assertIn("moon.setAz", moon_rise)
        self.assertIn("moon.isVisible", moon_rise)
        self.assertIn("drawMoonPhaseDetails", moon_phase)
        for field in (
            "moon.jDate",
            "moon.phase",
            "moon.age",
            "moon.fraction",
            "moon.distance",
            "moon.latitude",
            "moon.longitude",
            "moon.phaseName",
            "moon.zodiacName",
        ):
            self.assertIn(field, moon_phase)

    def test_release_build_enables_strict_warnings(self):
        repository = Path(__file__).resolve().parents[1]
        platformio = (repository / "platformio.ini").read_text()
        self.assertNotIn("\n    -flto", platformio)
        self.assertIn("LTO is incompatible", platformio)
        self.assertIn("build_src_flags", platformio)
        self.assertIn("-Wall", platformio)
        self.assertIn("-Wextra", platformio)
        self.assertIn("-Wshadow", platformio)
        self.assertIn("-Werror=return-type", platformio)
        self.assertIn("extra_scripts = pre:tools/platformio_linker_map.py", platformio)

    def test_power_diagnostics_are_optional_and_rtc_only(self):
        repository = Path(__file__).resolve().parents[1]
        header = (
            repository / "src/sdk/include/Sdk/WatchyPowerDiagnostics.h"
        ).read_text()
        implementation = (
            repository / "src/sdk/WatchyPowerDiagnostics.cpp"
        ).read_text()
        platformio = (repository / "platformio.ini").read_text()

        self.assertIn("#ifdef WATCHY_POWER_DIAGNOSTICS", header)
        self.assertIn("RTC_DATA_ATTR Counters counters", implementation)
        self.assertNotIn("Preferences", implementation)
        self.assertNotIn("WatchyStorage", implementation)
        self.assertIn("[env:power-diagnostics]", platformio)
        self.assertIn("-DWATCHY_POWER_DIAGNOSTICS=1", platformio)
        self.assertIn("lightSleepMs", implementation)
        self.assertIn("equivalentFullRefreshMilli", implementation)
        self.assertIn("minimumHeartRateStackWords", implementation)
        self.assertIn("bcgResults", implementation)
        self.assertIn("bcg_results=%lu", implementation)
        self.assertIn("minimumNetworkHeap", implementation)
        self.assertIn("minimumNetworkLargestBlock", implementation)
        self.assertIn("maximumNetworkHeapLoss", implementation)

    def test_network_heap_checkpoints_cover_resource_lifecycle(self):
        repository = Path(__file__).resolve().parents[1]
        network = (repository / "src/app/NetworkAppCommon.cpp").read_text()
        diagnostics = (repository / "src/sdk/WatchyPowerDiagnostics.cpp").read_text()
        for checkpoint in ("BeforeRadio", "Connected", "Downloaded", "Parsed", "AfterRadio"):
            self.assertIn(checkpoint, diagnostics + network)
        self.assertIn("HeapCheckpoint::Downloaded", network)
        self.assertIn("HeapCheckpoint::Parsed", network)

    def test_saved_wifi_connection_waits_for_events_not_poll_ticks(self):
        repository = Path(__file__).resolve().parents[1]
        source = (repository / "src/sdk/os/ShowConnectWifi.cpp").read_text()
        connect = source.split("bool connectSavedWiFi", 1)[1].split(
            "void beginWiFiScreen", 1
        )[0]
        self.assertIn("WiFi.onEvent", connect)
        self.assertIn("ARDUINO_EVENT_WIFI_STA_GOT_IP", connect)
        self.assertIn("xTaskNotifyGive", connect)
        self.assertIn("Input::waitNotified(remaining)", connect)
        self.assertIn("WiFi.removeEvent", connect)
        self.assertNotIn("Input::wait(100)", connect)

    def test_dns_waits_on_bounded_socket_without_heap_polling(self):
        repository = Path(__file__).resolve().parents[1]
        source = (repository / "src/app/NetworkAppCommon.cpp").read_text()
        dns = source.split("bool dnsQuery", 1)[1].split(
            "EchoResult sendEcho", 1
        )[0]
        self.assertIn("SO_RCVTIMEO", dns)
        self.assertIn("lwip_recvfrom", dns)
        self.assertIn("lwip_close(socketHandle)", dns)
        self.assertNotIn("WiFiUDP", dns)
        self.assertNotIn("parsePacket", dns)
        self.assertNotIn("deepSleepDelay(10)", dns)

    def test_screen_supports_bounded_multi_region_invalidation(self):
        repository = Path(__file__).resolve().parents[1]
        header = (repository / "src/sdk/include/Sdk/WatchyUi.h").read_text()
        implementation = (repository / "src/sdk/WatchyUi.cpp").read_text()
        self.assertIn("maximumDirtyRegions = 4", header)
        self.assertIn("static void invalidate", header)
        self.assertIn("static void presentDirty", header)
        self.assertIn("normalizeDirtyBounds", implementation)
        self.assertIn("left &= ~7", implementation)
        self.assertIn("transactionAreaPenalty = 512", implementation)
        self.assertIn("recordPartialRefresh(boundsArea(bounds))", implementation)

    def test_list_selection_updates_only_rows_or_list_body(self):
        repository = Path(__file__).resolve().parents[1]
        ui = (repository / "src/sdk/WatchyUi.cpp").read_text()
        selector = (repository / "src/sdk/os/ShowWatchFaceSelector.cpp").read_text()
        themes = (repository / "src/app/ShowThemeColours.cpp").read_text()
        update = ui.split("void ListView::presentSelectionChange", 1)[1].split(
            "uint8_t ListView::previous", 1
        )[0]
        self.assertIn("listRowBounds(previous", update)
        self.assertIn("listRowBounds(selected", update)
        self.assertIn("bodyBounds", update)
        self.assertIn("Screen::presentDirty", update)
        self.assertIn("updateWatchfaceSelector", selector)
        self.assertIn("updateThemeColours", themes)
        menu = (repository / "src/sdk/os/ShowMenu.cpp").read_text()
        watchy = (repository / "src/sdk/WatchyCore.cpp").read_text()
        fast_menu = menu.split("void showFastMenuImpl", 1)[1].split(
            "} // namespace", 1
        )[0]
        self.assertIn("updateMenuSelection", fast_menu)
        self.assertIn("MAIN_MENU_STATE", menu)
        self.assertIn("showFastMenu(menuIndex, previousIndex)", watchy)
        healthcare = (
            repository / "src/sdk/tools/Healthcare/HealthcareSupport.cpp"
        ).read_text()
        rss = (repository / "src/app/ShowRssFeed.cpp").read_text()
        time_tools = (repository / "src/sdk/tools/Time/Metronome.cpp").read_text()
        self.assertIn("updateProfileEditor", healthcare)
        self.assertIn("updateNewsSources", rss)
        self.assertIn("updateMetronomeSelector", time_tools)

    def test_metronome_cadence_is_task_owned_and_drift_free(self):
        repository = Path(__file__).resolve().parents[1]
        time_tools = (repository / "src/sdk/tools/Time/Metronome.cpp").read_text()
        engine = (repository / "src/app/MetronomeEngine.cpp").read_text()
        timing = (repository / "src/sdk/include/App/MetronomeTiming.h").read_text()
        run = time_tools.split("void runMetronome()", 1)[1].split(
            "\n}\n\n} // namespace", 1
        )[0]

        self.assertIn("esp_timer_get_time()", engine)
        self.assertIn("xTaskCreatePinnedToCore", engine)
        self.assertIn("skipToLatestDue(nowUs)", engine)
        self.assertIn("schedule.advance()", engine)
        self.assertIn("digitalWrite(VIB_MOTOR_PIN, HIGH)", engine)
        self.assertIn("digitalWrite(VIB_MOTOR_PIN, LOW)", engine)
        self.assertIn("minuteUs / bpm_", timing)
        self.assertIn("periodRemainder_", timing)
        self.assertIn("WatchyMetronome::stop();", run)
        self.assertIn("WatchyUi::Input::waitScheduled", run)
        self.assertIn("saveMetronomeSettings", run)
        self.assertIn("updateMetronomeRunning", run)
        self.assertIn("Screen::present(metronomeStatusBounds)", time_tools)
        self.assertNotIn("pulseMotor(", run)
        self.assertNotIn("nextBeat", run)

        health_support = (
            repository / "src/app/ShowHealthSupport.cpp"
        ).read_text()
        cpr = health_support.split("void runCprMetronome()", 1)[1].split(
            "void drawPain", 1
        )[0]
        self.assertIn("WatchyMetronome::PulseStyle::Uniform", cpr)
        self.assertIn("WatchyUi::Input::waitScheduled", cpr)
        self.assertIn("WatchyMetronome::stop();", cpr)
        self.assertNotIn("pulseMotor(", cpr)
        self.assertNotIn("nextBeat", cpr)

    def test_accelerometer_uses_visible_state_dirty_regions(self):
        repository = Path(__file__).resolve().parents[1]
        source = (repository / "src/sdk/os/ShowAccelerometer.cpp").read_text()
        production_source = source.split("#ifdef WATCHY_DETERMINISTIC_GALLERY", 1)[0]
        self.assertIn("valuesChanged", source)
        self.assertIn("Screen::invalidate(valuesBounds)", source)
        self.assertIn("Screen::invalidate(directionBounds)", source)
        self.assertIn("Screen::presentDirty", source)
        self.assertIn("Screen::liveViewRefreshIntervalMs", source)
        self.assertNotIn("display.display(true)", production_source)

    def test_spirit_level_refreshes_only_visible_changes(self):
        repository = Path(__file__).resolve().parents[1]
        source = (repository / "src/sdk/tools/Sensors/SpiritLevel.cpp").read_text()
        run_level = source.split("void runSpiritLevel()", 1)[1].split(
            "#ifdef WATCHY_DETERMINISTIC_GALLERY", 1
        )[0]
        self.assertIn("same(visible, current)", run_level)
        self.assertIn("Screen::invalidate(oldDot)", run_level)
        self.assertIn("Screen::invalidate(newDot)", run_level)
        self.assertIn("Screen::invalidate(textBounds)", run_level)
        self.assertIn("Screen::presentDirty", run_level)

    def test_pong_live_frames_use_actor_dirty_regions(self):
        repository = Path(__file__).resolve().parents[1]
        source = (repository / "src/app/ShowPong.cpp").read_text()
        delta = source.split("void drawPongDelta", 1)[1].split(
            "void adjustBounce", 1
        )[0]
        loop = source.split("void showPongImpl(Watchy *watchy)", 1)[1]
        loop = loop.split("void Watchy::showPong()", 1)[0]
        self.assertIn("previousBall", delta)
        self.assertIn("previousPlayer", delta)
        self.assertIn("previousComputer", delta)
        self.assertIn("Screen::invalidate", delta)
        self.assertIn("drawPongField", delta)
        self.assertIn("Screen::presentDirty", delta)
        self.assertIn("drawPongDelta(previous, state)", loop)
        self.assertNotIn("drawPong(state)", loop.split("while (true)", 1)[1])

    def test_snake_live_moves_use_head_tail_dirty_cells(self):
        repository = Path(__file__).resolve().parents[1]
        source = (repository / "src/app/ShowSnake.cpp").read_text()
        delta = source.split("void drawSnakeMoveDelta", 1)[1].split(
            "MoveResult moveSnake", 1
        )[0]
        loop = source.split("void showSnakeImpl(Watchy *watchy)", 1)[1]
        loop = loop.split("void Watchy::showSnake()", 1)[0]
        self.assertIn("SnakeVisualSnapshot", source)
        self.assertIn("snakeCellBounds(current.bodyX[0]", delta)
        self.assertIn("snakeCellBounds(previous.tailX", delta)
        self.assertIn("Screen::invalidate(head)", delta)
        self.assertIn("Screen::invalidate(tail)", delta)
        self.assertIn("Screen::presentDirty", delta)
        self.assertIn("drawSnakeMoveDelta(previous, state, result)", loop)

    def test_apps_can_wake_for_fall_detection(self):
        repository = Path(__file__).resolve().parents[1]
        ui_header = (repository / "src/sdk/include/Sdk/WatchyUi.h").read_text()
        ui_implementation = (repository / "src/sdk/WatchyUi.cpp").read_text()
        watchy = (repository / "src/sdk/WatchyCore.cpp").read_text()
        self.assertIn("setAuxiliaryWakeSource", ui_header)
        self.assertIn("gpio_wakeup_enable", ui_implementation)
        self.assertIn("auxiliaryWakeHandler();", ui_implementation)
        self.assertIn("ACC_INT_2_PIN, ACTIVE_LOW, serviceFallWake", watchy)

    def test_heart_rate_synchronization_has_bounded_static_ownership(self):
        repository = Path(__file__).resolve().parents[1]
        heart_rate = (repository / "src/app/ShowHeartRate.cpp").read_text()
        self.assertIn("xEventGroupCreateStatic", heart_rate)
        self.assertIn("xSemaphoreCreateMutexStatic", heart_rate)
        self.assertNotIn("callbackMutex", heart_rate)
        self.assertNotIn("xSemaphoreTake(measurementMutex, portMAX_DELAY)", heart_rate)

    def test_sensor_manager_owns_fall_hardware_configuration(self):
        repository = Path(__file__).resolve().parents[1]
        manager = (repository / "src/app/SensorManager.cpp").read_text()
        fall_detection = (repository / "src/app/FallDetection.cpp").read_text()
        self.assertIn("Mode::FallMonitoring", manager)
        self.assertIn("FALL_MONITORING_ANY_MOTION_THRESHOLD", manager)
        self.assertIn("WatchySensor::Mode::FallMonitoring", fall_detection)
        disable_bcg = fall_detection.index("setWatchfaceHeartRateMonitoring(false)")
        arm_fall = fall_detection.index("WatchySensor::Mode::FallMonitoring")
        self.assertLess(disable_bcg, arm_fall)
        self.assertNotIn("sensor.setAccelConfig", fall_detection)
        self.assertNotIn("sensor.configureAccelFifo", fall_detection)
        self.assertNotIn("sensor.mapInterrupt", fall_detection)

    def test_sensor_hardware_configuration_has_one_owner(self):
        repository = Path(__file__).resolve().parents[1]
        manager_path = repository / "src/app/SensorManager.cpp"
        forbidden = (
            "sensor.setAccelConfig",
            "sensor.setAdvancedPowerSave",
            "sensor.configureAccelFifo",
            "sensor.setFifoWatermark",
            "sensor.setInterruptMode",
            "sensor.mapInterrupt",
        )
        offenders = []
        for source_path in (repository / "src").rglob("*.cpp"):
            if source_path == manager_path:
                continue
            source = source_path.read_text()
            for call in forbidden:
                if call in source:
                    offenders.append(f"{source_path.relative_to(repository)}: {call}")
        self.assertEqual(offenders, [])

    def test_step_odr_candidate_is_isolated_from_production(self):
        repository = Path(__file__).resolve().parents[1]
        manager = (repository / "src/app/SensorManager.cpp").read_text()
        platformio = (repository / "platformio.ini").read_text()
        self.assertIn("WATCHY_BASELINE_ACCEL_ODR", manager)
        self.assertIn("BMA4_OUTPUT_DATA_RATE_50HZ", manager)
        self.assertIn("[env:step-odr-25]", platformio)
        candidate = platformio.split("[env:step-odr-25]", 1)[1]
        self.assertIn("BMA4_OUTPUT_DATA_RATE_25HZ", candidate)

    def test_live_acceleration_consumers_restore_background_mode(self):
        repository = Path(__file__).resolve().parents[1]
        sources = (
            repository / "src/sdk/os/ShowAccelerometer.cpp",
            repository / "src/app/ShowMiniGames.cpp",
            repository / "src/sdk/tools/Sensors/SensorSupport.cpp",
        )
        for source_path in sources:
            source = source_path.read_text()
            self.assertIn("Mode::LiveAcceleration", source)
            self.assertIn("acquireForeground", source)
            self.assertIn("releaseForeground", source)

    def test_sensor_cleanup_cannot_abort_mode_configuration(self):
        repository = Path(__file__).resolve().parents[1]
        source = (repository / "src/app/SensorManager.cpp").read_text()
        cleanup = source.split("bool clearExclusiveFeatures()", 1)[1].split(
            "bool configureAcceleration", 1
        )[0]
        self.assertIn("bma4_set_advance_power_save(BMA4_DISABLE", cleanup)
        self.assertIn("configureAccelFifo(false, 0)", cleanup)
        self.assertIn("return result == BMA4_OK;", cleanup)
        self.assertIn("clearExclusiveFeatures() && configureAcceleration", source)

    def test_bcg_refreshes_only_pixel_changed_heart_rate_region(self):
        repository = Path(__file__).resolve().parents[1]
        watchy = (repository / "src/sdk/WatchyCore.cpp").read_text()
        heart_rate = (repository / "src/app/ShowHeartRate.cpp").read_text()
        seven_segment = (
            repository / "src/watchfaces/7_SEG/Watchy_7_SEG.cpp"
        ).read_text()
        diagnostics = (
            repository / "src/sdk/WatchyPowerDiagnostics.cpp"
        ).read_text()

        self.assertIn("previousBpm != heartRateBpm", heart_rate)
        self.assertIn("previousValid != heartRateValid", heart_rate)
        self.assertIn("recordBcgResult(visibleChanged || heartIconChanged)", heart_rate)
        self.assertIn("refreshWatchFaceHeartRate()", watchy)
        refresh_method = seven_segment.split(
            "void Watchy7SEG::refreshHeartRate(Watchy &watch)", 1
        )[1].split("void Watchy7SEG::drawTime()", 1)[0]
        self.assertIn("Screen::present(", refresh_method)
        self.assertIn("heartRateBox.left", refresh_method)
        self.assertNotIn("showWatchFace", refresh_method)
        self.assertIn("skippedDisplayUpdates", diagnostics)
        self.assertIn("dirtyPixels", diagnostics)

    def test_dirty_refresh_never_expands_to_an_incomplete_full_frame(self):
        repository = Path(__file__).resolve().parents[1]
        source = (repository / "src/sdk/WatchyUi.cpp").read_text()
        present_dirty = source.split("void Screen::presentDirty", 1)[1].split(
            "void Screen::present(int", 1
        )[0]
        self.assertIn("setPartialWindow", present_dirty)
        self.assertIn("display(true)", present_dirty)
        self.assertNotIn("display(false)", present_dirty)
        self.assertNotIn("if (fullRefreshDue())", present_dirty)

    def test_deep_sleep_uses_bounded_worker_shutdown(self):
        repository = Path(__file__).resolve().parents[1]
        watchy = (repository / "src/sdk/WatchyCore.cpp").read_text()
        heart_rate = (repository / "src/app/ShowHeartRate.cpp").read_text()

        deep_sleep = watchy.split("void stopRuntimeForDeepSleep()", 1)[1].split(
            "void servicePendingWatchfaceWakeBeforeDeepSleep", 1
        )[0]
        self.assertIn("waitForHeartRateMeasurement(3000)", deep_sleep)
        self.assertIn("recordWorkerStopTimeout()", deep_sleep)
        self.assertIn("abortHeartRateMeasurement()", deep_sleep)
        self.assertIn("vTaskDelete(task)", heart_rate)
        self.assertIn("WatchySensor::releaseForeground", heart_rate)
        heart_header = (repository / "src/sdk/include/App/HeartRate.h").read_text()
        self.assertIn("timeoutMs = 3000", heart_header)
        wait_function = heart_rate.split(
            "bool waitForHeartRateMeasurement", 1
        )[1].split("void abortHeartRateMeasurement", 1)[0]
        self.assertNotIn("portMAX_DELAY", wait_function)

    def test_wifi_portal_has_absolute_radio_on_timeout(self):
        repository = Path(__file__).resolve().parents[1]
        source = (repository / "src/sdk/os/ShowConnectWifi.cpp").read_text()
        config = (repository / "src/sdk/include/System/config.h").read_text()
        portal = source.split("void setupWifiImpl(Watchy *watchy)", 1)[1]
        portal = portal.split("void Watchy::setupWifi()", 1)[0]
        self.assertIn("WIFI_AP_ABSOLUTE_TIMEOUT 600", config)
        self.assertIn("portalStartedAt", portal)
        self.assertIn("WIFI_AP_ABSOLUTE_TIMEOUT * 1000UL", portal)
        self.assertIn("wifiManager.stopConfigPortal()", portal)
        self.assertIn("WIFI_PORTAL_SERVICE_INTERVAL_MS 50", config)
        self.assertIn(
            "deepSleepDelay(WIFI_PORTAL_SERVICE_INTERVAL_MS)", portal
        )
        self.assertNotIn("deepSleepDelay(10)", portal)

    def test_foreground_heart_rate_keeps_scheduler_running(self):
        repository = Path(__file__).resolve().parents[1]
        heart_rate = (repository / "src/app/ShowHeartRate.cpp").read_text()
        ui = (repository / "src/sdk/WatchyUi.cpp").read_text()
        measurement_loop = heart_rate.split(
            "while (isHeartRateMeasurementRunning())", 1
        )[1].split(
            "    stopHeartRateMeasurement();\n    if (!waitForHeartRateMeasurement",
            1,
        )[0]
        scheduled_wait = ui.split("Event Input::waitScheduled", 1)[1]
        scheduled_wait = scheduled_wait.split("Event Input::waitNotified", 1)[0]

        self.assertIn("Input::waitScheduled(serialServiceIntervalMs)", measurement_loop)
        self.assertIn("waitForButtonNotification(remaining)", scheduled_wait)
        self.assertNotIn("Power::idle", scheduled_wait)
        self.assertNotIn("esp_light_sleep_start", scheduled_wait)

    def test_foreground_heart_rate_skips_identical_and_uses_small_regions(self):
        repository = Path(__file__).resolve().parents[1]
        source = (repository / "src/app/ShowHeartRate.cpp").read_text()
        draw = source.split("void drawAppHeartRate", 1)[1].split(
            "bool ensureSynchronizationObjects", 1
        )[0]
        self.assertIn("appRenderedState", draw)
        self.assertIn("recordSkippedDisplayUpdate", draw)
        self.assertIn("Screen::invalidate(appHeartBounds)", draw)
        self.assertIn("Screen::invalidate(appBpmBounds)", draw)
        self.assertIn("Screen::invalidate(appStatusBounds)", draw)
        self.assertIn("Screen::presentDirty", draw)
        self.assertNotIn("displayWindow", draw)

    def test_partial_refresh_debt_scales_with_dirty_area(self):
        repository = Path(__file__).resolve().parents[1]
        ui = (repository / "src/sdk/WatchyUi.cpp").read_text()

        self.assertIn("RTC_DATA_ATTR uint32_t partialRefreshDebtPixels", ui)
        self.assertIn("maximumPartialRefreshDebtPixels", ui)
        self.assertIn("59UL * fullScreenPixels", ui)
        self.assertIn("maximumConsecutivePartialRefreshes = 240", ui)
        self.assertIn("recordPartialRefresh(fullScreenPixels)", ui)
        self.assertIn("recordPartialRefresh(boundsArea(bounds))", ui)
        self.assertIn("partialRefreshDebtPixels = 0", ui)
        toast = ui.split("void Feedback::toast", 1)[1].split(
            "uint16_t ScrollableTextView::lineCount", 1
        )[0]
        self.assertIn("Screen::present(toastBounds, guiState)", toast)
        self.assertNotIn("setPartialWindow", toast)

    def test_daily_alarm_has_no_periodic_unchanged_refresh(self):
        repository = Path(__file__).resolve().parents[1]
        source = (repository / "src/sdk/tools/Time/DailyAlarm.cpp").read_text()
        run_alarm = source.split("void runDailyAlarm()", 1)[1].split(
            "#ifdef WATCHY_DETERMINISTIC_GALLERY", 1
        )[0]
        self.assertIn("WatchyUi::Input::wait()", run_alarm)
        self.assertNotIn("Event::NONE", run_alarm)
        self.assertNotIn("liveViewRefreshIntervalMs", run_alarm)

    def test_gray8_bayer_dithering_is_complete_and_spatially_stable(self):
        repository = Path(__file__).resolve().parents[1]
        ui_header = (repository / "src/sdk/include/Sdk/WatchyUi.h").read_text()
        ui = (repository / "src/sdk/WatchyUi.cpp").read_text()
        matrix_block = re.search(
            r"bayer16\[16 \* 16\].*?=\s*\{(.*?)\};", ui, re.DOTALL
        )
        self.assertIsNotNone(matrix_block)
        thresholds = [int(value) for value in re.findall(r"\d+", matrix_block.group(1))]

        self.assertEqual(len(thresholds), 256)
        self.assertEqual(set(thresholds), set(range(256)))
        for gray in (0, 1, 64, 128, 192, 254, 255):
            rendered = [gray == 255 or (gray != 0 and value < gray)
                        for value in thresholds]
            expected_white = 256 if gray == 255 else gray
            self.assertEqual(sum(rendered), expected_white)

        def packed_region(left, top, width, height, gray):
            self.assertEqual(width % 8, 0)
            output = bytearray(width * height // 8)
            for local_y in range(height):
                for local_x in range(width):
                    absolute_x = left + local_x
                    absolute_y = top + local_y
                    threshold = thresholds[(absolute_y & 15) * 16 +
                                           (absolute_x & 15)]
                    white = gray == 255 or (gray != 0 and threshold < gray)
                    if white:
                        offset = local_y * (width // 8) + local_x // 8
                        output[offset] |= 0x80 >> (local_x & 7)
            return bytes(output)

        first = packed_region(0, 0, 200, 200, 128)
        second = packed_region(0, 0, 200, 200, 128)
        self.assertEqual(first, second)
        partial = packed_region(32, 40, 96, 80, 128)
        extracted = b"".join(
            first[row * 25 + 4:row * 25 + 16]
            for row in range(40, 120)
        )
        self.assertEqual(partial, extracted)
        self.assertIn("static_cast<uint16_t>(y) & 15U", ui)
        self.assertIn("static_cast<uint16_t>(x) & 15U", ui)
        self.assertNotIn("millis()", ui.split("bool GrayPaint::pixelIsWhite", 1)[1]
                         .split("void GrayPaint::pixel", 1)[0])
        self.assertIn("using Gray8 = uint8_t", ui_header)
        self.assertIn("static void bitmap8", ui_header)
        self.assertNotIn("Gray8 framebuffer", ui)
        fill_rect = ui.split("void GrayPaint::fillRect", 1)[1].split(
            "void GrayPaint::fillRoundRect", 1
        )[0]
        bitmap = ui.split("void GrayPaint::bitmap8", 1)[1].split(
            "void GrayPaint::gradient", 1
        )[0]
        self.assertIn("gray == GRAY_BLACK || gray == GRAY_WHITE", fill_rect)
        self.assertIn("Watchy::display.fillRect", fill_rect)
        self.assertIn("sourceLeft", bitmap)
        self.assertIn("sourceRight", bitmap)
        self.assertIn("sourceTop", bitmap)
        self.assertIn("sourceBottom", bitmap)

        watchfaces = (repository / "src/watchfaces").rglob("*.cpp")
        self.assertTrue(all("GrayPaint" not in path.read_text() for path in watchfaces))

        checkbox = ui.split("void Widget::checkbox", 1)[1].split(
            "void Widget::radio", 1
        )[0]
        radio = ui.split("void Widget::radio", 1)[1].split(
            "void Widget::toggle", 1
        )[0]
        for control in (checkbox, radio):
            self.assertIn("ToneRole::Disabled", control)
            self.assertNotIn("[DISABLED]", control)

    def test_runtime_resource_paths_restore_low_power_baseline(self):
        repository = Path(__file__).resolve().parents[1]
        bluetooth = (
            repository / "src/sdk/tools/Bluetooth/BluetoothSupport.cpp"
        ).read_text()
        safety = (repository / "src/sdk/tools/Safety/SosBle.cpp").read_text()
        wifi = (repository / "src/sdk/os/ShowConnectWifi.cpp").read_text()

        self.assertGreaterEqual(bluetooth.count("BLEDevice::deinit(false)"), 2)
        self.assertGreaterEqual(bluetooth.count("endBleSession()"), 2)
        self.assertIn("BLEDevice::deinit(false)", safety)
        self.assertIn("endBleSession()", safety)
        self.assertIn("WiFi.disconnect(true, false)", wifi)
        self.assertIn("endWifiSession()", wifi)
        self.assertIn("setLowPowerCpuFrequency()", wifi)

    def test_conventional_os_screens_use_shared_screen_lifecycle(self):
        repository = Path(__file__).resolve().parents[1]
        for relative in (
            "src/sdk/os/ShowAbout.cpp",
            "src/sdk/os/ShowBuzz.cpp",
            "src/sdk/os/ShowSyncNTP.cpp",
        ):
            source = (repository / relative).read_text()
            runtime = source.split("#ifdef WATCHY_DETERMINISTIC_GALLERY", 1)[0]
            self.assertIn("WatchyUi::Screen::beginCanvas()", runtime)
            self.assertIn("WatchyUi::Screen::present(APP_STATE)", runtime)
            self.assertNotIn("display.display(true)", runtime)
        wifi = (repository / "src/sdk/os/ShowConnectWifi.cpp").read_text()
        wifi_runtime = wifi.split("#ifdef WATCHY_DETERMINISTIC_GALLERY", 1)[0]
        watchy_core = (repository / "src/sdk/WatchyCore.cpp").read_text()
        portal_callback = watchy_core.split("void configModeCallback", 1)[1].split(
            "void drawDefaultWatchFace", 1
        )[0]
        self.assertIn("WatchyUi::Screen::beginCanvas()", wifi_runtime)
        self.assertNotIn("display.display(true)", wifi_runtime)
        self.assertIn("WatchyUi::Screen::present(APP_STATE)", portal_callback)

    def test_set_time_is_event_driven_without_epaper_blinking(self):
        repository = Path(__file__).resolve().parents[1]
        source = (repository / "src/sdk/os/SetTime.cpp").read_text()
        runtime = source.split("#ifdef WATCHY_DETERMINISTIC_GALLERY", 1)[0]
        self.assertIn("setTimeSelectionBounds", runtime)
        self.assertIn("WatchyUi::Canvas::outline", runtime)
        self.assertIn("WatchyUi::Input::wait()", runtime)
        self.assertIn("WatchyUi::Screen::present(APP_STATE)", runtime)
        self.assertNotIn("Input::wait(700)", runtime)
        self.assertNotIn("blink", runtime)

    def test_unexpected_scene_is_rejected(self):
        with self.assertRaisesRegex(capture_gallery.CaptureError, "scene 1 mismatch"):
            self.parse(protocol_stream(("demo/wrong/light",), (b"\xAA",)),
                       ("demo/expected/light",))

    def test_serial_handshake_requests_capture(self):
        class Duplex(io.BytesIO):
            def __init__(self):
                super().__init__(b"@WATCHY_READY 1\r\n")
                self.sent = bytearray()

            def write(self, value):
                self.sent.extend(value)
                return len(value)

            def flush(self):
                pass

        stream = Duplex()
        reader = capture_gallery.TimedReader(stream, False, 1.0)
        self.assertFalse(capture_gallery.request_serial_capture(reader, stream))
        self.assertEqual(stream.sent, b"@WATCHY_CAPTURE 1\n")

    def test_serial_stream_reconnects_after_transient_cdc_failure(self):
        class Stream:
            def __init__(self, payload=b"", failure=None):
                self.payload = payload
                self.failure = failure

            def read(self, size):
                if self.failure is not None:
                    failure, self.failure = self.failure, None
                    raise failure
                value, self.payload = self.payload[:size], self.payload[size:]
                return value

            def close(self):
                pass

        class SerialModule:
            SerialException = OSError

            def __init__(self):
                self.streams = [Stream(failure=OSError("CDC reset")), Stream(b"R")]

            def Serial(self, port, baud, timeout):
                self.assertions = (port, baud, timeout)
                return self.streams.pop(0)

        module = SerialModule()
        with capture_gallery.ReconnectingSerial(
            module, "COM3", 115200, 0.1, 1.0
        ) as stream:
            self.assertEqual(stream.read(1), b"R")
        self.assertEqual(module.assertions, ("COM3", 115200, 0.1))

    def test_catalog_matches_firmware_emission_order(self):
        repository = Path(__file__).resolve().parents[1]
        applications = gallery_catalog.load_app_catalog(
            repository / "src/sdk/demo/GalleryAppCatalog.inc"
        )
        firmware_scene_ids = gallery_catalog.ordered_scene_ids(applications)
        catalog = capture_gallery.load_catalog(repository / "tools/gallery_scene_ids.txt")
        self.assertEqual(len(applications), 142)
        self.assertEqual(tuple(firmware_scene_ids), catalog)

    def test_menu_registry_contains_every_action_once(self):
        repository = Path(__file__).resolve().parents[1]
        model = (repository / "src/sdk/include/App/MenuModel.h").read_text()
        menu = (repository / "src/sdk/os/ShowMenu.cpp").read_text()
        enum_block = re.search(
            r"enum MenuAction\s*:\s*uint8_t\s*\{(.*?)\};", model, re.DOTALL
        )
        registry_block = re.search(
            r"const MenuItem clocksSkyItems\[\].*?"
            r"const MenuCategory categories\[\]",
            menu,
            re.DOTALL,
        )
        category_block = re.search(
            r"const MenuCategory categories\[\]\s*=\s*\{(.*?)\};",
            menu,
            re.DOTALL,
        )
        self.assertIsNotNone(enum_block)
        self.assertIsNotNone(registry_block)
        self.assertIsNotNone(category_block)

        enum_actions = re.findall(r"MENU_ACTION_[A-Z0-9_]+", enum_block.group(1))
        registry_actions = re.findall(
            r"MENU_ACTION_[A-Z0-9_]+", registry_block.group(0)
        )
        self.assertEqual(len(enum_actions), 142)
        self.assertEqual(Counter(registry_actions), Counter(enum_actions))
        self.assertTrue(all(count == 1 for count in Counter(registry_actions).values()))

        categories = re.findall(r'\{"([^"]+)",\s*\w+Items', category_block.group(1))
        self.assertTrue(all(len(category) <= 15 for category in categories))
        self.assertEqual(
            categories,
            [
                "Clocks & Sky",
                "Timers & Focus",
            "Health & Care",
            "Safety & Aid",
            "Sensors & Steps",
                "Everyday Tools",
                "Games & Puzzles",
                "Network & Web",
                "Bluetooth",
                "Watch & System",
            ],
        )

    def test_full_catalog_replay_publishes_every_scene(self):
        repository = Path(__file__).resolve().parents[1]
        catalog_path = repository / "tools/gallery_scene_ids.txt"
        catalog = capture_gallery.load_catalog(catalog_path)
        bitmaps = tuple(
            bytes([sequence % 256]) * 5000 for sequence in range(len(catalog))
        )
        payload = protocol_stream(catalog, bitmaps, width=200, height=200)
        with tempfile.TemporaryDirectory() as temporary:
            input_path = Path(temporary) / "capture.bin"
            output_path = Path(temporary) / "gallery"
            input_path.write_bytes(payload)
            with contextlib.redirect_stdout(io.StringIO()):
                result = capture_gallery.main(
                    (
                        "--input",
                        str(input_path),
                        "--catalog",
                        str(catalog_path),
                        "--output",
                        str(output_path),
                    )
                )
            manifest = json.loads((output_path / "manifest.json").read_text())
            self.assertEqual(result, 0)
            self.assertEqual(manifest["frame_count"], len(catalog))
            self.assertEqual(
                len(tuple((output_path / "png").rglob("*.png"))), len(catalog)
            )
            self.assertEqual(
                len(tuple((output_path / "pbm").rglob("*.pbm"))), len(catalog)
            )

    def test_readme_publishes_every_catalog_scene_once(self):
        repository = Path(__file__).resolve().parents[1]
        readme = (repository / "readme.md").read_text()
        catalog = capture_gallery.load_catalog(
            repository / "tools/gallery_scene_ids.txt"
        )
        expected_paths = tuple(
            f"docs/gallery/png/{scene_id}.png" for scene_id in catalog
        )
        published_paths = re.findall(
            r'!\[[^\]]*\]\((docs/gallery/png/[^)]+\.png)\)', readme
        )
        application_rows = re.findall(
            r"^\| \*\*(?!Total\*\*)[^|]+\*\* \|", readme, re.MULTILINE
        )

        self.assertCountEqual(published_paths, expected_paths)
        self.assertEqual(len(set(published_paths)), len(catalog))
        self.assertEqual(len(application_rows), 142)
        self.assertEqual(
            readme.count(
                "| Application | What it does | Screenshot 1 | Screenshot 2 | "
                "Screenshot 3 | Screenshot 4 | Screenshot 5 |"
            ),
            10,
        )

    def test_checked_in_gallery_matches_catalog(self):
        repository = Path(__file__).resolve().parents[1]
        gallery = repository / "docs/gallery"
        manifest = json.loads((gallery / "manifest.json").read_text())
        catalog = capture_gallery.load_catalog(
            repository / "tools/gallery_scene_ids.txt"
        )

        self.assertEqual(manifest["frame_count"], len(catalog))
        self.assertEqual(
            tuple(frame["id"] for frame in manifest["frames"]), catalog
        )
        for scene_id in catalog:
            self.assertTrue((gallery / "png" / f"{scene_id}.png").is_file())
            self.assertTrue((gallery / "pbm" / f"{scene_id}.pbm").is_file())

    def test_watchfaces_match_independent_pixel_golden_hashes(self):
        repository = Path(__file__).resolve().parents[1]
        manifest = json.loads(
            (repository / "docs/gallery/manifest.json").read_text()
        )
        golden = json.loads(
            (repository / "tools/watchface_golden_hashes.json").read_text()
        )
        actual = {
            frame["id"]: frame["png_sha256"]
            for frame in manifest["frames"]
            if frame["id"].startswith("watchfaces/")
        }
        self.assertEqual(len(golden), 16)
        self.assertEqual(actual, golden)

    def test_captured_views_are_nonblank_and_distinct_per_app(self):
        repository = Path(__file__).resolve().parents[1]
        gallery = repository / "docs/gallery"
        manifest = json.loads((gallery / "manifest.json").read_text())
        frames = {frame["id"]: frame for frame in manifest["frames"]}
        applications = gallery_catalog.load_app_catalog(
            repository / "src/sdk/demo/GalleryAppCatalog.inc"
        )

        for frame in manifest["frames"]:
            pbm = (gallery / frame["pbm"]).read_bytes()
            payload = pbm.split(b"\n", 2)[2]
            self.assertNotEqual(len(set(payload)), 1, f"blank scene: {frame['id']}")

        for application in applications:
            scene_ids = application.scene_ids()
            hashes = tuple(frames[scene_id]["png_sha256"] for scene_id in scene_ids)
            self.assertEqual(
                len(set(hashes)), len(hashes),
                f"duplicate views for {application.prefix}",
            )


if __name__ == "__main__":
    unittest.main()