#!/bin/bash

rm sym.elf
readelf -s main_in.o &> sym.elf
gedit sym.elf
