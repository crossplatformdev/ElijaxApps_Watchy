import unittest

from tools import analyze_linker_map


class LinkerMapTests(unittest.TestCase):
    def test_aggregates_project_flash_and_ram(self):
        contributions = analyze_linker_map.parse_map(
            [
                ".flash.text 0x0000000042000020 0x100",
                " .text.foo 0x0000000042000020 0x20 .pio/build/env/src/app/Foo.cpp.o",
                ".flash.rodata 0x000000003c000020 0x80",
                " .rodata.foo 0x000000003c000020 0x10 .pio/build/env/src/app/Foo.cpp.o",
                ".dram0.bss 0x000000003fca0000 0x40",
                " .bss.foo 0x000000003fca0000 0x08 .pio/build/env/src/app/Foo.cpp.o",
                " .bss.lib 0x000000003fca0008 0x04 .pio/build/env/lib/libBar.a(Bar.o)",
            ]
        )
        project = contributions[".pio/build/env/src/app/Foo.cpp.o"]
        self.assertEqual(project["flash"], 0x30)
        self.assertEqual(project["ram"], 0x08)
        self.assertTrue(
            analyze_linker_map.is_project_object(
                ".pio/build/env/src/app/Foo.cpp.o"
            )
        )

    def test_ignores_relaxation_annotations(self):
        self.assertIsNone(
            analyze_linker_map.normalize_object("0x20 (size before relaxing)")
        )


if __name__ == "__main__":
    unittest.main()