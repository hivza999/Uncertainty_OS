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

#### To run on real hardware

- x86
- bios

To make a bootable usb / disk

**Warning**\
Data will mostlikely be erased

```bash
sudo dd if=bin/Uncertainty.img of=/dev/sdX
# sdX is the device to write to
```

## Building the project

To build all of the OS + tools:

```bash
./build a
```

`./build t` to build tools\
`./build b` to build bootloader\
`./build k` to build kernel\
`./build h` to show help

Add `q` as q 2nd argument to launch qemu

```bash
./build a q
```

A disk image will be made at `bin/os.img`\
Qemu will atomaticly be launched

## Project tree

```
Uncertainty_OS/
├── bin/                   Where compiled files of the OS goes
│   ├── fs/                Filesystem of the OS
│   ├── part/              Part of the final image of the OS
│   ├── tmp/               Temporary files
│   └── Uncertainty.img    Image of the OS
├── build                  Script to build the OS
├── memory_map.md          How is arrange the first 1MiB of memory
├── README.md              Should be easy enough to guess what it is
├── src/                   Source files for the OS
│   ├── cpu/               For functions of the OS close to the CPU
│   │   ├── boot.s         The bootloader
│   │   └── interrupts/    To handle the interrupts
│   ├── drivers/           Basic drivers
│   ├── kernel.c           The main kernel file
│   └── kernel_entry.s     Where the kernel start
├── TODO.md                Things to do maybe one day
├── tools/                 Tools that help to make the OS
│   └── mkpart_fat32       Used to make the FAT32 image
└── tools_src/             Source file for the tools
    ├── make_filesystem    File used to say how is the fat32 partition
    └── mkpart_fat32.c     Used to make the FAT32 image
```
