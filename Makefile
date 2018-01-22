NAME			:= DanaOS
VERSION			:= 0.0.1
HOST			:= unknown

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

# Check for selected bootloader
ifeq ($(LOADER),)
	LOADER		:= CUSTOM
endif

ifeq ($(or $(filter CUSTOM,$(LOADER)),$(filter GRUB2,$(LOADER))),)
    $(error '$(LOADER)' is not a supported bootloader)
endif

TARGET			:= $(ISA)-elf

SRCDIR			:= src
OBJDIR			:= build/$(ISA)/obj
BINDIR			:= build/$(ISA)/bin
CFGDIR			:= cfg

MBRDIR			:= mbr
BOOTDIR			:= bootloader
KERNELDIR		:= kernel
IMAGEMKDIR		:= imagemk

IMAGEMK			:= $(IMAGEMKDIR)/imagemk

IMGCUSTOM		:= $(BINDIR)/$(NAME).img
IMGGRUB			:= $(BINDIR)/$(NAME)-grub.img
TARGETIMG		:= $(IMGCUSTOM)
ifeq ($(LOADER),GRUB2)
	TARGETIMG	:= $(IMGGRUB)
endif

# Include the configuration for the current OS and the target ISA
include	$(CFGDIR)/make/$(HOST)_config.mk
include	$(CFGDIR)/make/flags/isa_flags.mk

CCFLAGS			+= -ffreestanding -O0 -std=c11 -ggdb -Wall -Wextra
CPPFLAGS		+= -ffreestanding -O0 -std=c++17 -fno-exceptions -fno-rtti -ggdb -Wall -Wextra
ASMFLAGS		+= -ggdb
LDFLAGS			+= -nostdlib -lgcc --disable-__cxa_atexit
TARGETCCFLAGS	:= $(CCFLAGS) $($(ISA)-CCFLAGS)
TARGETCPPFLAGS	:= $(CPPFLAGS) $($(ISA)-CPPFLAGS)
TARGETASMFLAGS	:= $(ASMFLAGS) $($(ISA)-ASMFLAGS)
TARGETLDFLAGS	:= $(LDFLAGS) $($(ISA)-LDFLAGS)
QEMUFLAGS		+= -no-kvm -net none -vga std -m 64 -serial file:serial.log

DEBUGSCRIPT		:= $(CFGDIR)/debug/gdb-$(ISA).script

.PHONY: all build-all clean clean-mbr clean-bootloader clean-kernel build-mbr build-bootloader build-kernel


all: build

include $(MBRDIR)/rules.mk
include $(BOOTDIR)/rules.mk
include $(KERNELDIR)/rules.mk
include $(IMAGEMKDIR)/rules.mk

build: dir build-mbr build-bootloader build-kernel build-imagemk

clean: clean-bootloader clean-mbr clean-kernel clean-imagemk
	@rm -rf $(BINDIR)/*

dir: dir-mbr dir-bootloader dir-kernel dir-imagemk
	@mkdir -p $(BINDIR)
	@mkdir -p $(MBRDIR)/$(BINDIR)
	@mkdir -p $(MBRDIR)/$(OBJDIR)
	@mkdir -p $(BOOTDIR)/$(BINDIR)
	@mkdir -p $(BOOTDIR)/$(OBJDIR)
	@mkdir -p $(KERNELDIR)/$(BINDIR)
	@mkdir -p $(KERNELDIR)/$(OBJDIR)
	
# TODO: Write the size of the kernel into the bootloader
# TODO: differentiate between HDD, CD etc.!
img: $(TARGETIMG)

$(IMGCUSTOM): build $(MBRBIN) $(BOOTBIN) $(KERNELELFSTRIPPED) $(IMAGEMKBIN)
	@$(IMAGEMKBIN) -m $(MBRBIN) -b $(BOOTBIN) 506 507 -k $(KERNELELFSTRIPPED) 508 509 -s 512 $(IMGCUSTOM)

$(IMGGRUB): $(KERNELIMG)
	@$(CP) -f $(KERNELIMG) $(IMGGRUB)

debug: img
	# WIP... needs its own script I fear (if that is even enough)
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(TARGETIMG) -S -s -daemonize && $(GDB) $(MBRSYMBOLS) -x $(DEBUGSCRIPT)
	
run: img
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(TARGETIMG)
