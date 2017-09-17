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

ASMFLAGS	:= -ggdb
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

build: $(BIN)
	
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

