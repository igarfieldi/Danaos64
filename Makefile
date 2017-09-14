TARGET		:= $(ISA)-elf

# Default target
ifeq ($(ISA),)
	ISA		:= x86_64
endif

.PHONY: all

all:
	@$(MAKE) -C mbr
	@$(MAKE) -C kernel

