#include "WatchfaceRegistry.h"

#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"

namespace WatchfaceRegistryDraw {
void draw7Seg(Watchy &watchy);
void draw7SegLight(Watchy &watchy);
void drawAnalog(Watchy &watchy);
void drawBadForEye(Watchy &watchy);
void drawBahn(Watchy &watchy);
void drawBasic(Watchy &watchy);
void drawBCD(Watchy &watchy);
void drawBeastie(Watchy &watchy);
void drawBigTime(Watchy &watchy);
void drawBinary(Watchy &watchy);
void drawBinaryBlocks(Watchy &watchy);
void drawBotWatchy(Watchy &watchy);
void drawBrainwork(Watchy &watchy);
void drawBTTF(Watchy &watchy);
void drawCalculator_Watchy(Watchy &watchy);
void drawCalculateur(Watchy &watchy);
void drawCalendar_WatchFace(Watchy &watchy);
void drawCaptn_Wednesday(Watchy &watchy);
void drawCastleOfWatchy(Watchy &watchy);
void drawChaosLorenzAttractor(Watchy &watchy);
void drawChronometer(Watchy &watchy);
void drawCityWeather(Watchy &watchy);
void drawDali(Watchy &watchy);
void drawDkTime(Watchy &watchy);
void drawDigdug(Watchy &watchy);
void drawDos(Watchy &watchy);
void drawErika_Type(Watchy &watchy);
void drawExactlyWords(Watchy &watchy);
void drawHobbitTime(Watchy &watchy);
void drawJarvis(Watchy &watchy);
void drawKaveWatchy(Watchy &watchy);
void drawKeen(Watchy &watchy);
void drawKitty(Watchy &watchy);
void drawLastLaugh(Watchy &watchy);
void drawLCARS(Watchy &watchy);
void drawLine(Watchy &watchy);
void drawMacPaint(Watchy &watchy);
void drawMario(Watchy &watchy);
void drawMarquee(Watchy &watchy);
void drawMaze(Watchy &watchy);
void drawMetaBall(Watchy &watchy);
void drawMickey(Watchy &watchy);
void drawMulti_face_Watchy(Watchy &watchy);
void drawOrbital(Watchy &watchy);
void drawPipBoy(Watchy &watchy);
void drawPokemon(Watchy &watchy);
void drawPoe(Watchy &watchy);
void drawPxl999(Watchy &watchy);
void drawQLock(Watchy &watchy);
void drawQrWatchface(Watchy &watchy);
void drawReDub(Watchy &watchy);
void drawRevolution(Watchy &watchy);
void drawS2Analog(Watchy &watchy);
void drawShadowClock(Watchy &watchy);
void drawSWWatchy(Watchy &watchy);
void drawShijian(Watchy &watchy);
void drawSkully(Watchy &watchy);
void drawSkykid(Watchy &watchy);
void drawSlacker(Watchy &watchy);
void drawSmartWatchy(Watchy &watchy);
void drawSpiralWatchy(Watchy &watchy);
void drawSquarbital(Watchy &watchy);
void drawSquaro(Watchy &watchy);
void drawStarWarsAurebesh(Watchy &watchy);
void drawStarryHorizon(Watchy &watchy);
void drawStationaryText(Watchy &watchy);
void drawSteps(Watchy &watchy);
void drawSundial(Watchy &watchy);
void drawTetris(Watchy &watchy);
void drawBlob(Watchy &watchy);
void drawTriangle(Watchy &watchy);
void drawTypoStyle(Watchy &watchy);
void drawWatchyAkira(Watchy &watchy);
void drawPowerShell(Watchy &watchy);
void drawWatchySevenSegment(Watchy &watchy);
void drawXMarksTheSpot(Watchy &watchy);
} // namespace WatchfaceRegistryDraw

namespace WatchfaceRegistry {

// Keep this list alphabetically sorted. The selector uses this exact order.
const uint8_t kWatchfaceCount = 76;

// Menu items used by WatchFaceSelectorApp.
const UIMenuItemSpec kWatchfaceMenuItems[kWatchfaceCount] = {
  {"7_SEG"},
  {"7_SEG_LIGHT"},
  {"Analog"},
  {"Bad_For_Eye"},
  {"Bahn"},
  {"Basic"},
  {"BCD"},
  {"beastie"},
  {"Big_Time"},
  {"Binary"},
  {"BinaryBlocks"},
  {"BotWatchy"},
  {"Brainwork"},
  {"BTTF"},
  {"Calculator_Watchy"},
  {"Calculateur"},
  {"Calendar_WatchFace"},
  {"Captn_Wednesday"},
  {"Castle_of_Watchy"},
  {"Chaos_-_Lorenz_Attractor"},
  {"Chronometer"},
  {"CityWeather"},
  {"Dali"},
  {"dkTime"},
  {"Digdug_Watch"},
  {"DOS"},
  {"erika_Type"},
  {"Exactly-Words"},
  {"Hobbit_Time"},
  {"Jarvis"},
  {"Kave_Watchy"},
  {"Keen"},
  {"Kitty"},
  {"Last_Laugh"},
  {"LCARS"},
  {"Line"},
  {"MacPaint"},
  {"Mario"},
  {"Marquee"},
  {"Maze"},
  {"MetaBall"},
  {"Mickey"},
  {"Multi_face_Watchy"},
  {"Orbital"},
  {"Pip-Boy"},
  {"Pokemon"},
  {"Poe"},
  {"pxl999"},
  {"QLock"},
  {"QR_Watchface"},
  {"Re-Dub"},
  {"Revolution"},
  {"S2Analog"},
  {"Shadow_Clock"},
  {"Shijian"},
  {"Skully"},
  {"Skykid_Watch"},
  {"Slacker"},
  {"SmartWatchy"},
  {"Spiral_Watchy"},
  {"Squarbital"},
  {"Squaro"},
  {"Star_Wars_Aurebesh"},
  {"StarryHorizon"},
  {"Stationary_Text"},
  {"Steps"},
  {"SW_Watchy"},
  {"Sundial"},
  {"Tetris"},
  {"The_Blob"},
  {"Triangle"},
  {"TypoStyle"},
  {"Watchy_Akira"},
  {"Watchy_PowerShell"},
  {"WatchySevenSegment"},
  {"X_marks_the_spot"},
};

// Watchface draw dispatch table. Only a subset is wired up today; the rest
// are wired up; the selector uses this table as-is.
const Entry kWatchfaces[kWatchfaceCount] = {
  {"7_SEG", &WatchfaceRegistryDraw::draw7Seg},
  {"7_SEG_LIGHT", &WatchfaceRegistryDraw::draw7SegLight},
  {"Analog", &WatchfaceRegistryDraw::drawAnalog},
  {"Bad_For_Eye", &WatchfaceRegistryDraw::drawBadForEye},
  {"Bahn", &WatchfaceRegistryDraw::drawBahn},
  {"Basic", &WatchfaceRegistryDraw::drawBasic},
  {"BCD", &WatchfaceRegistryDraw::drawBCD},
  {"beastie", &WatchfaceRegistryDraw::drawBeastie},
  {"Big_Time", &WatchfaceRegistryDraw::drawBigTime},
  {"Binary", &WatchfaceRegistryDraw::drawBinary},
  {"BinaryBlocks", &WatchfaceRegistryDraw::drawBinaryBlocks},
  {"BotWatchy", &WatchfaceRegistryDraw::drawBotWatchy},
  {"Brainwork", &WatchfaceRegistryDraw::drawBrainwork},
  {"BTTF", &WatchfaceRegistryDraw::drawBTTF},
  {"Calculator_Watchy", &WatchfaceRegistryDraw::drawCalculator_Watchy},
  {"Calculateur", &WatchfaceRegistryDraw::drawCalculateur},
  {"Calendar_WatchFace", &WatchfaceRegistryDraw::drawCalendar_WatchFace},
  {"Captn_Wednesday", &WatchfaceRegistryDraw::drawCaptn_Wednesday},
  {"Castle_of_Watchy", &WatchfaceRegistryDraw::drawCastleOfWatchy},
  {"Chaos_-_Lorenz_Attractor", &WatchfaceRegistryDraw::drawChaosLorenzAttractor},
  {"Chronometer", &WatchfaceRegistryDraw::drawChronometer},
  {"CityWeather", &WatchfaceRegistryDraw::drawCityWeather},
  {"Dali", &WatchfaceRegistryDraw::drawDali},
  {"dkTime", &WatchfaceRegistryDraw::drawDkTime},
  {"Digdug_Watch", &WatchfaceRegistryDraw::drawDigdug},
  {"DOS", &WatchfaceRegistryDraw::drawDos},
  {"erika_Type", &WatchfaceRegistryDraw::drawErika_Type},
  {"Exactly-Words", &WatchfaceRegistryDraw::drawExactlyWords},
  {"Hobbit_Time", &WatchfaceRegistryDraw::drawHobbitTime},
  {"Jarvis", &WatchfaceRegistryDraw::drawJarvis},
  {"Kave_Watchy", &WatchfaceRegistryDraw::drawKaveWatchy},
  {"Keen", &WatchfaceRegistryDraw::drawKeen},
  {"Kitty", &WatchfaceRegistryDraw::drawKitty},
  {"Last_Laugh", &WatchfaceRegistryDraw::drawLastLaugh},
  {"LCARS", &WatchfaceRegistryDraw::drawLCARS},
  {"Line", &WatchfaceRegistryDraw::drawLine},
  {"MacPaint", &WatchfaceRegistryDraw::drawMacPaint},
  {"Mario", &WatchfaceRegistryDraw::drawMario},
  {"Marquee", &WatchfaceRegistryDraw::drawMarquee},
  {"Maze", &WatchfaceRegistryDraw::drawMaze},
  {"MetaBall", &WatchfaceRegistryDraw::drawMetaBall},
  {"Mickey", &WatchfaceRegistryDraw::drawMickey},
  {"Multi_face_Watchy", &WatchfaceRegistryDraw::drawMulti_face_Watchy},
  {"Orbital", &WatchfaceRegistryDraw::drawOrbital},
  {"Pip-Boy", &WatchfaceRegistryDraw::drawPipBoy},
  {"Pokemon", &WatchfaceRegistryDraw::drawPokemon},
  {"Poe", &WatchfaceRegistryDraw::drawPoe},
  {"pxl999", &WatchfaceRegistryDraw::drawPxl999},
  {"QLock", &WatchfaceRegistryDraw::drawQLock},
  {"QR_Watchface", &WatchfaceRegistryDraw::drawQrWatchface},
  {"Re-Dub", &WatchfaceRegistryDraw::drawReDub},
  {"Revolution", &WatchfaceRegistryDraw::drawRevolution},
  {"S2Analog", &WatchfaceRegistryDraw::drawS2Analog},
  {"Shadow_Clock", &WatchfaceRegistryDraw::drawShadowClock},
  {"Shijian", &WatchfaceRegistryDraw::drawShijian},
  {"Skully", &WatchfaceRegistryDraw::drawSkully},
  {"Skykid_Watch", &WatchfaceRegistryDraw::drawSkykid},
  {"Slacker", &WatchfaceRegistryDraw::drawSlacker},
  {"SmartWatchy", &WatchfaceRegistryDraw::drawSmartWatchy},
  {"Spiral_Watchy", &WatchfaceRegistryDraw::drawSpiralWatchy},
  {"Squarbital", &WatchfaceRegistryDraw::drawSquarbital},
  {"Squaro", &WatchfaceRegistryDraw::drawSquaro},
  {"Star_Wars_Aurebesh", &WatchfaceRegistryDraw::drawStarWarsAurebesh},
  {"StarryHorizon", &WatchfaceRegistryDraw::drawStarryHorizon},
  {"Stationary_Text", &WatchfaceRegistryDraw::drawStationaryText},
  {"Steps", &WatchfaceRegistryDraw::drawSteps},
  {"SW_Watchy", &WatchfaceRegistryDraw::drawSWWatchy},
  {"Sundial", &WatchfaceRegistryDraw::drawSundial},
  {"Tetris", &WatchfaceRegistryDraw::drawTetris},
  {"The_Blob", &WatchfaceRegistryDraw::drawBlob},
  {"Triangle", &WatchfaceRegistryDraw::drawTriangle},
  {"TypoStyle", &WatchfaceRegistryDraw::drawTypoStyle},
  {"Watchy_Akira", &WatchfaceRegistryDraw::drawWatchyAkira},
  {"Watchy_PowerShell", &WatchfaceRegistryDraw::drawPowerShell},
  {"WatchySevenSegment", &WatchfaceRegistryDraw::drawWatchySevenSegment},
  {"X_marks_the_spot", &WatchfaceRegistryDraw::drawXMarksTheSpot},
};

} // namespace WatchfaceRegistry
