NAME		:= DanaOS
VERSION		:= 0.0.1
HOST		:= unknown

# Check the platform to include the correct program variables (see cfg/make)
ifeq ($(OS),Windows_NT)
	HOST		:= windows
endif

ifeq ($(shell uname -s), Linux)
	HOST		:= linux
endif

# Default target
ifeq ($(ISA),)
	ISA			:= x86_64
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
OBJDIR		:= build/obj
BINDIR		:= build/bin
CFGDIR		:= cfg

MBRDIR		:= mbr
BOOTDIR		:= bootloader
KERNELDIR	:= kernel

IMG			:= $(BINDIR)/$(NAME).img

ASM			:= $(TARGET)-as
CC			:= $(TARGET)-gcc
CPP			:= $(TARGET)-g++
LD			:= $(TARGET)-ld
OBJCOPY		:= $(TARGET)-objcopy
CAT			:= cat
DD			:= dd
FIND		:= find
GDB			:= gdb
QEMU		:= qemu-system-$(ISA)

ASMFLAGS	:= -ggdb
QEMUFLAGS	:= -no-kvm -net none -vga std -m 64

DEBUGSCRIPT	:= $(CFGDIR)/debug/gdb.script


.PHONY: all build-all clean clean-mbr clean-bootloader clean-kernel build-mbr build-bootloader build-kernel

include $(MBRDIR)/rules.mk
include $(BOOTDIR)/rules.mk

all: build

build: dir build-mbr build-bootloader

clean: clean-bootloader clean-mbr
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

$(IMG): build
	@echo "$(MBRBIN) $(BOOTBIN)"
	@$(CAT) $(MBRBIN) $(BOOTBIN) > $(IMG)

debug: iso
	# WIP... needs its own script I fear (if that is even enough)
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(IMG) -S -s -daemonize && $(GDB) $(MBRDIR)/$(BINDIR)/$(MBRSYMBOLS) -x $(DEBUGSCRIPT)
	
run: iso
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(IMG)
