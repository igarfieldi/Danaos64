TARGET		:= $(ISA)-elf
NAME		:= DanaOS

# Default target
ifeq ($(ISA),)
	ISA		:= x86_64
endif

MBRDIR		:= mbr
BOOTDIR		:= bootloader
KERNELDIR	:= kernel
BINDIR		:= build/bin

MBR			:= $(MBRDIR)/$(BINDIR)/mbr.bin
BOOTLOADER	:= $(BOOTDIR)/$(BINDIR)/bootloader.bin
KERNEL		:= $(KERNELDIR)/$(BINDIR)/kernel.bin
IMG			:= $(BINDIR)/$(NAME).img

CAT			:= cat
DD			:= dd
OBJCOPY		:= $(TARGET)-objcopy
GDB			:= gdb
QEMU		:= qemu-system-$(ISA)


QEMUFLAGS	:= -no-kvm -net none -vga std -m 64

.PHONY: all build mbr bootloader kernel clean debug iso run

all: iso
build: mbr bootloader kernel

mbr: $(MBR)
	
bootloader: $(BOOTLOADER)
	
kernel: $(KERNEL)
	
$(MBR):
	@$(MAKE) -C mbr --no-print-directory build
$(BOOTLOADER):
	@$(MAKE) -C bootloader --no-print-directory build
$(KERNEL):
	@$(MAKE) -C kernel --no-print-directory build
	
$(IMG): mbr bootloader
	@mkdir -p build/bin
	@$(CAT) $(MBR) $(BOOTLOADER) > $(IMG)
	
clean:
	@$(MAKE) -C mbr --no-print-directory clean
	@$(MAKE) -C bootloader --no-print-directory clean
	@$(MAKE) -C kernel --no-print-directory clean
	@rm -rf build/bin/*

debug: $(IMG)
	@$(OBJCOPY) --only-keep-debug $(ELF) $(BINDIR)/$(SYMBOLS)
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(IMG) -S -s -daemonize && $(GDB) $(BINDIR)/$(SYMBOLS) -x $(DEBUGSCRIPT)
	@rm -f $(BINDIR)/$(SYMBOLS)
	
iso: $(IMG)
	
run: $(IMG)
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(IMG)
