#!/bin/bash

gcc gen_gfx.c -o gen_gfx
./gen_gfx < gfx.txt > p8_gfx.h
rm gen_gfx
