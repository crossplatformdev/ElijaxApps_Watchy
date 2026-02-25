# Exactly-Words

A watch face for Watchy that give you an exact time in minutes written in words. Updated every minute to give the new time. 

An example is that it is "Almost twenty-five past five in afternoon" or 17:24

![alt text](https://raw.githubusercontent.com/bmsleight/Exactly-Words/main/screenshots/watch.jpg)

Some example screenshots are below.

![alt text](https://raw.githubusercontent.com/bmsleight/Exactly-Words/main/screenshots/watchfaces.gif)

### Bottom Lines on Screen
The bottom lines on the screen give the date, battery percentage and steps recorded in the last 7 days. Not the battery percentage is untested. 

The steps are recorded in thousands of steps. For today and yesterday it includes one decimal place. (E.g. 10,0 would be 10,023 steps). For the 2nd-7th oldest days it just displays thousands of steps.

#### Notes

* Hardest part was word wrapping the text. 
* Logic is form my other project for watches - https://github.com/bmsleight/wasp-os/blob/master/wasp/apps/word_clock.py 
* C is not my preferred language so contains not ideal code


#### Thanks

*  https://github.com/peerdavid/Watchy/blob/master/examples/WatchFaces/David/Watchy_Base.cpp Used battery example   