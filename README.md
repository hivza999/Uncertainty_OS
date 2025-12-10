# Welcome to Uncertainty OS

A 32 bit Unix-like OS

This is what i do with my insanity

## Requirements

### To compile

- Nasm
- [i686 cross-compiler](https://wiki.osdev.org/GCC_Cross-Compiler)

### To run

#### To Emulate

- Qemu

#### TO run on real hardware

- x86
- bios

To make a bootable usb / disk

**Warning**\
Data will mostlikely be erased

```bash
sudo dd if=bin/os.img of=/dev/sdX
# SDX is the device to write to
```

## Building the project

To build the os:

```bash
./build
```

A disk image will be made at `bin/os.img`\
Qemu will atomaticly be launched

## Project tree

```
Uncertainty_OS/
├─ bin/                 Compiled stuff
│  ├─ tmp/              Tempory files
│  └─ os.img            OS image
├─ src/
│  ├─ cpu/              CPU stuff
│  │  ├─ interrupts/    Interrupt handeling
│  │  └─ boot.s         bootloader
│  ├─ drivers/          Drivers
│  ├─ kernel_entry.s    Where the kernel start execution
│  └─ kernel.c          The main kernel file
├─ .gitignore
├─ build                Use ths to build this project
├─ memory_map.md        Memory map of the OS
├─ README.md
└─ TODO.md
```
