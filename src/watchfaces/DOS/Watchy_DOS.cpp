#include "Watchy_DOS.h"
#include "Px437_IBM_BIOS5pt7b.h"

void WatchyDOS::drawWatchFace(Watchy &watch){
    char time[6];
    time[0] = '0' + ((watch.currentTime.Hour/10)%10);
    time[1] = '0' + (watch.currentTime.Hour%10);
    time[2] = ':';
    time[3] = '0' + ((watch.currentTime.Minute/10)%10);
    time[4] = '0' + (watch.currentTime.Minute%10);
    time[5] = 0;
    Watchy::display.setFont(&Px437_IBM_BIOS5pt7b);
    Watchy::display.setCursor(0, 24);
    Watchy::display.println("WATCHY-DOS 1.1.8");
    Watchy::display.println("Copyright (c) 2020");
    Watchy::display.println(" ");
    Watchy::display.print("AUTOEXEC BAT ");
    Watchy::display.println(time);
    Watchy::display.print("COMMAND  COM ");
    Watchy::display.println(time);
    Watchy::display.print("CONFIG   SYS ");
    Watchy::display.println(time);
    Watchy::display.print("ESPTOOL  PY  ");
    Watchy::display.println(time);
    Watchy::display.println(" ");
    Watchy::display.println("  4 files 563 bytes");
    Watchy::display.println("  2048 bytes free");
    Watchy::display.println(" ");
    Watchy::display.println("<C:\\>esptool");
}
