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

witdh  =  256
height =  256

################################## Functions ###################################


## Format data to output a string for C array declaration.
# @param data   Array containing binary values
# @return A string containing the array formated for C code.
def output():
    # Create a file.c for output
    output_f = open("include/GlossPow.h", "w")
	
    # Generate the output with hexadecimal values
    s = "const unsigned char " + "GlossPow" + "[] PROGMEM = {" + '\n'
    for j in range(height):
        for i in range(witdh):
            s += format(int(pow(i / 255.0, (j + 1) * 0.5) * 255), '#04x') + ", "
            if ( (j * witdh + i)%16 == 15):
                s += '\n'
    s = s[:-3] + '\n};'

    output_f.write(s)
	
    return s


#################################### Main ######################################

if __name__ == '__main__':
    output()
    print(">>>>DONE!<<<<<")
