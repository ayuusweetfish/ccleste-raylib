#!/bin/bash

# convert pico-8_font_020.png rgba:- | \
#   xxd -p -c 32 | \
#   sed s/020408ff/0/g | \
#   sed s/fff1e8ff/1/g | \
#   tr -d '\n'

gcc gen_font.c -o gen_font
convert pico-8_font_020.png rgba:- | ./gen_font > p8_font.h
rm gen_font
