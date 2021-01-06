#!/bin/bash

gcc gen_sfx.c -o gen_sfx
./gen_sfx < sfx.txt > p8_sfx.h
rm gen_sfx
