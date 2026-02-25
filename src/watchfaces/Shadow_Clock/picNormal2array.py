#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Picture to OLED screen (pic2oled)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Convert an image to a hexadecimal array for OLED screens.
Usage:
   >>> python pic2oled.py <filename>

The script outputs a C declaration of a static array. Code output may be easily
included in source code managed by the Arduino IDE.

See full documentation in README.md
:copyright: (c) 2015 by Jean-Yves VET.
:license: MIT, see LICENSE for more details.
"""

from __future__ import print_function
import sys, re, os
from PIL import Image


############################### Global Variables ###############################

watchyWitdh  =  200
watchyHeight =  200

################################## Functions ###################################

## Check arguments and image dimensions.
# @return An image object if nothing went wrong
def checkArgs():
    # Check number of arguments
    if len(sys.argv) == 3:
        # Try to open the image
        try:
            im = Image.open(sys.argv[1])
        except:
            print("Error: unable to open", sys.argv[1], file=sys.stderr)
            exit(-1)

        # Check image dimensions
        width,height = im.size
        if (width != watchyWitdh or height != watchyHeight):
            print("Error: invalid picture dimensions (",
                width, "x", height, "), expected", watchyWitdh,
                "x", watchyHeight, file=sys.stderr)
            exit(-1)
        
        return im
    else :
        print("Error: invalid number of arguments", file=sys.stderr)
        print("Usage:")
        print("python " + sys.argv[0] + " <filename>")
        exit(-1)


# @param im   A picture opened with PIL.
# @return A binary array
def getData(im):
    # Allocate array to hold binary values
    binary = [[[0 for x in range(3)] for x in range(watchyHeight)] for x in range(watchyWitdh)]

    # Convert to binary values by using threshold
    for j in range(watchyHeight):
        for i in range(watchyWitdh):
            binary[i][j][0] = im.getpixel((i, j))[0]
            binary[i][j][1] = im.getpixel((i, j))[1]
            binary[i][j][2] = im.getpixel((i, j))[2]

    return binary

def hex2(n):
    return "0x%x"%(n&0xffffffff)

## Format data to output a string for C array declaration.
# @param data   Array containing binary values
# @return A string containing the array formated for C code.
def output(data):
    # Retrieve filename without the extension
    filename = os.path.basename(sys.argv[1])
    filename = os.path.splitext(filename)[0]
    filename = re.sub('[ :,\r?]', '', filename)

    # Create a file.c for output
    output_f = open(sys.argv[2] + "/" + filename + ".h", "w")
	
    # Generate the output with hexadecimal values
    s = '#pragma once\n'
    s += "const signed char " + filename + "[] PROGMEM = {" + '\n'
    for j in range(watchyHeight):
        for i in range(watchyWitdh):
            s += hex((255 - data[i][j][0]) - 128) + ", " + hex(data[i][j][1] - 128) + ", " + hex(data[i][j][2] - 128) + ", "
            if ( (j * watchyWitdh + i)%16 == 15):
                s += '\n'
    s = s[:-3] + '\n};'

    output_f.write(s)
	
    return s


#################################### Main ######################################

if __name__ == '__main__':
    image = checkArgs()
    data = getData(image)
    output(data)
    print(">>>>DONE!<<<<<")
