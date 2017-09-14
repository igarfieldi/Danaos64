ISA			:= x86_64
TARGET		:= $(ISA)-elf
NAME		:= bootloader
SYMBOLS		:= $(NAME)-symbols.sym

ASM			:= $(TARGET)-as
LD			:= $(TARGET)-ld
OBJCOPY		:= $(TARGET)-objcopy
DD			:= dd
FIND		:= find
GDB			:= gdb
QEMU		:= qemu-system-$(ISA)


ifeq ($(debug),)
	DEBUGFLAGS	:=
else
	DEBUGFLAGS	:= -ggdb
endif

ASMFLAGS	:=
QEMUFLAGS	:= -no-kvm -net none -vga std -m 64

SRCDIR		:= ./src
INCDIR		:= ./src
CFGDIR		:= ./cfg
OBJDIR		:= ./build/obj
BINDIR		:= ./build/bin

LDSCRIPT	:= $(CFGDIR)/linker/ld.script
DEBUGSCRIPT	:= $(CFGDIR)/debug/gdb.script

SRC			:= $(shell $(FIND) -name "*.s")
OBJ			:= $(notdir $(SRC))
OBJ			:= $(subst .s,.o,$(OBJ))
OBJ			:= $(addprefix $(OBJDIR)/,$(OBJ))
ELF			:= $(BINDIR)/$(NAME).elf
BIN			:= $(BINDIR)/$(NAME).bin
IMG			:= $(BINDIR)/$(NAME).img

DEP          := $(patsubst %.o,%.d,$(OBJ))
VPATH        := $(dir $(SRC))

.PHONY: all clean debug run

all:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)
	@$(MAKE) --no-print-directory build

build: $(IMG)

$(IMG): $(BIN)
	@$(DD) if=$(BIN) bs=512 of=$(IMG)
	
$(BIN): $(ELF)
	@$(OBJCOPY) -O binary --strip-all $(ELF) $(BIN)

$(ELF): $(OBJ)
	@$(LD) -T $(LDSCRIPT) -o $(ELF) $(OBJ)
	
# Assembly rule
$(OBJDIR)/%.o : %.s Makefile
	@$(ASM) -MD $(patsubst %.o,%.d,$@) $< -o $@ -I $(INCDIR) $(ASMFLAGS) $(DEBUGFLAGS)

# Include the dependency rules (if present; if not, we have to build the obj file anyway)
-include $(DEP)

clean:
	@rm -rf $(OBJDIR)/*
	@rm -rf $(BINDIR)/*

debug: $(IMG)
	@$(OBJCOPY) --only-keep-debug $(ELF) $(BINDIR)/$(SYMBOLS)
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(IMG) -S -s -daemonize && $(GDB) $(BINDIR)/$(SYMBOLS) -x $(DEBUGSCRIPT)
	@rm -f $(BINDIR)/$(SYMBOLS)
	
run: $(IMG)
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(IMG)

