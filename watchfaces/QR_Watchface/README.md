# QR watch face
A watch face displaying the time in a QR Code

![Screenshot](./qr_watchface_screen.png)


## Origin
A watch loving friend was annoying with his love of weird time displays : binary, decimal, in reverse…  
So I wanted to annoy him back with a watchface which would display the time (and date) as a QR code.

## Features
Display the time and the date in a QR code. You can either decode it in your head (I tried, without success), or use a code scanner to see the time and date.

But if you are like me, you like to cheat, so you can press down (bottom right button) to switch on or off the display of the time and date string, in tiny font above the QR code.

Both options being not optimal, either trying to decoding the QR code, or trying to see speedily the tiny font of the EZ mode (yeah, writing the time in full characters is the easy mode ;) ), I thought something might help.  
So I added a Morse mode. Press the up button, and the watch will vibrate in Morse the time (not the date, it would be a bit too long).  
Decoding Morse time is relatively easy (compared to the QR code at least), and doesn't use your eyes, so might be helpful on some situations.

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
