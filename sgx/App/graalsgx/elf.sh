#!/bin/bash

rm sym.elf
readelf -s main_out.o &> sym.elf
gedit sym.elf
