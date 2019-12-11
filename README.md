# Cogip AVR firmwares

## Build status
TODO

## Dependencies

To install AVR toolchain on debian/ubuntu:

```bash
$ sudo apt-get install avrdude gcc-avr binutils-avr avr-libc
```

To install AVR toolchain on Arch Linux:

```bash
$ pacman -Sy avr-binutils avr-gcc avr-libc avrdude
```

## Build

```bash
$ make defconfig
$ make
```

Then, simply run `make` to build the firmware.

## Install firmware in Flash memory

Assuming you dispose of an AVR Isp mkII programmer, you can use following to program the flash memory:

```bash
$ sudo avrdude -v -v -p atxmega32a4 -c avrispmkII -U flash:w:lidar.hex:i
```
