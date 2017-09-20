NAME		:= DanaOS
VERSION		:= 0.0.1
HOST		:= unknown

# Check the platform to include the correct program variables (see cfg/make)
ifeq ($(OS),Windows_NT)
	HOST	:= windows
endif

ifeq ($(shell uname -s), Linux)
	HOST	:= linux
endif

# Default target
ifeq ($(ISA),)
	ISA		:= x86_64
endif

# Check for valid ISA
ifeq ($(or $(filter i386,$(ISA)),$(filter x86_64,$(ISA))),)
    $(error '$(ISA)' is not a supported target)
endif

TARGET		:= $(ISA)-elf

# Include the configuration for the current OS and the target ISA
#include cfg/make/$(HOST)_config.mk
#include cfg/make/ISA/$(ISA).mk

SRCDIR		:= src
OBJDIR		:= build/$(ISA)/obj
BINDIR		:= build/$(ISA)/bin
CFGDIR		:= cfg

MBRDIR		:= mbr
BOOTDIR		:= bootloader
KERNELDIR	:= kernel

IMG			:= $(BINDIR)/$(NAME).img

CCFLAGS		+= -ffreestanding -O0 -std=C11 -ggdb -Wall -Wextra
CPPFLAGS	+= -ffreestanding -O0 -std=c++14 -fno-exceptions -fno-rtti -ggdb -Wall -Wextra
ASMFLAGS	+= -ggdb
LDFLAGS		+= -ffreestanding -O0 -nostdlib -lgcc --disable-__cxa_atexit -ggdb
QEMUFLAGS	+= -no-kvm -net none -vga std -m 64 -serial file:serial.log

include	$(CFGDIR)/make/$(HOST)_config.mk
include	$(CFGDIR)/make/ISA/$(ISA).mk

DEBUGSCRIPT	:= $(CFGDIR)/debug/gdb-$(ISA).script

.PHONY: all build-all clean clean-mbr clean-bootloader clean-kernel build-mbr build-bootloader build-kernel

all: build

include $(MBRDIR)/rules.mk
include $(BOOTDIR)/rules.mk
include $(KERNELDIR)/rules.mk

build: dir build-mbr build-bootloader build-kernel

clean: clean-bootloader clean-mbr clean-kernel
	@rm -rf $(BINDIR)/*

dir:
	@mkdir -p $(BINDIR)
	@mkdir -p $(MBRDIR)/$(BINDIR)
	@mkdir -p $(MBRDIR)/$(OBJDIR)
	@mkdir -p $(BOOTDIR)/$(BINDIR)
	@mkdir -p $(BOOTDIR)/$(OBJDIR)
	@mkdir -p $(KERNELDIR)/$(BINDIR)
	@mkdir -p $(KERNELDIR)/$(OBJDIR)
	
iso: $(IMG)
	@$(CAT) $(MBRBIN) $(BOOTBIN) $(KERNELBIN) > $(IMG)

$(IMG): build
	@$(CAT) $(MBRBIN) $(BOOTBIN) $(KERNELBIN) > $(IMG)

debug: iso
	# WIP... needs its own script I fear (if that is even enough)
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(IMG) -S -s -daemonize && $(GDB) $(MBRSYMBOLS) -x $(DEBUGSCRIPT)
	
run: iso
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(IMG)
