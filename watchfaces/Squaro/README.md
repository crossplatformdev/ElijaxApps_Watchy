# Squaro watch face
A simplistic watch face displaying the time in big blocks.

Here is a screenshot showing 19:53.

![Screenshot](./screenshot.png)

## Origin
This watch face is partially inspired by a blog post I stumbled upon : https://blog.tokyoflash.com/2015/01/21/square-watch-uses-block-graphic-digits/

I adapted it a bit to increase readability while trying to keep it as simple and brutalistic as possible.

## Features
Display the time. That's simple.

Light and dark theme. Use the bottom (bottom right) button to alternate between them.

For funnsies and because I already had the code,  I added a Morse mode. Press the up button, and the watch will vibrate in Morse the time (not the date, it would be a bit too long).  
Decoding Morse time is relatively easy with a bit of practise, and doesn't use your eyes, so might be helpful on some situations.

As a reminder, here is the Morse code used :

* 0 : — — — — —
* 1 : • — — — —
* 2 : • • — — —
* 3 : • • • — —
* 4 : • • • • —
* 5 : • • • • •
* 6 : — • • • •
* 7 : — — • • •
* 8 : — — — • •
* 9 : — — — — •

The time will be vibrating as "XX XX", always two block of two digits, with a space (silent) in between.  
Usually, Morse code is sent at the speed of ~60ms per dot (a dash being three dots). I choose 100ms instead, to facilitate comprehension. Don't thank me :) In any case, it can be easily edited in the code.

## How to use/install
I used PlatformIO.  
You can use `pio run -t upload` to compile and upload the project to your Watchy.

If you don't use PlatformIO, look at the `platformio.ini` to have a list of libs necessary for compilation.

