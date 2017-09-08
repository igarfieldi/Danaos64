------- README -------

This project represents an attempt at creating an x86 operating system. Due to the nature
of working this close to the hardware, possible mishaps may result in permanent damage!
Use this code at your own risk!

The OS is/will be coded primarily in C++ with a sprinkle of assembly where needed. To compile
it you'll need a C++ cross-compiler; I only tested GCC 6.1 with binutils 2.26 for i386-elf.
To compile, debug or execute using the provided Makefile either QEMU or manual editing is required.