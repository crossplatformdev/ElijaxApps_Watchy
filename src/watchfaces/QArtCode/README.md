# QArtCode

Displays a new QR code every minute, which when scanned will take you to a random collection object from the Victoria and Albert museum in London, UK.

***Note that this watchface is fairly impractical for telling the time, unless you have very good eyesight!***

The reason for this is that the time display in the centre is extremely small: partly because I needed to keep the QR code relatively unobstructed to get it to scan correctly, but mainly because it amused me to do so.

It needs WiFi to work, and if you see [this image](broken.png) that is the watch's way of telling you that there is a problem connecting. To get more debug data, start Serial Monitor from the Arduino IDE.

It relies on a remote server to generate the necessary bytes that represent the 40,000 pixels for the Watchy display. By default it retrieves this from https://qartcode.herokuapp.com/ but that source code is available for you to modify in the [Python](../../Python/) directory of this repo.

