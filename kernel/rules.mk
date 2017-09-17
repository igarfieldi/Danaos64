

# Include the configuration for the current OS and the target ISA
include cfg/make/$(HOST)_config.mk
include cfg/make/ISA/$(ISA).mk

# Specify the directories of the project
SRCDIR			:= ./src
INCDIR			:= ./src
CFGDIR			:= ./cfg
OBJDIR			:= ./build/obj
BINDIR			:= ./build/bin
ISODIR			:= ./build/iso
DOCDIR			:= ./doc

# Name of the executable/iso file
BIN				:= $(BINDIR)/$(NAME)_$(TARGET)_$(VERSION).elf
ISO				:= $(ISODIR)/$(NAME)_$(TARGET)_$(VERSION).iso

# Scripts for the linker, the debugger, and the code formatter
LINKERSCRIPT	:= $(CFGDIR)/linker/linker-$(ISA).ld
DEBUGSCRIPT		:= $(CFGDIR)/debug/gdb-$(ISA).script
FORMATSCRIPT	:= $(CFGDIR)/format/clang-format-diff.py

ifeq ($(debug),)
	DEBUGFLAGS	:=
else
	DEBUGFLAGS	:= -ggdb
endif

# The flags for compilation. TODO: is it bad practice to define them so absolutely?
CCFLAGS			:= $(ISA_C_FLAGS) -ffreestanding -O0 -std=C11 -g -Wall -Wextra
CPPFLAGS		:= $(ISA_CPP_FLAGS) -ffreestanding -O0 -std=c++14 -fno-exceptions -fno-rtti -g -Wall -Wextra
ASMFLAGS		:= $(ISA_ASM_FLAGS)
LDFLAGS			:= $(ISA_LD_FLAGS) -ffreestanding -O0 -nostdlib -lgcc --disable-__cxa_atexit
EMUFLAGS		:= -no-kvm -net none -vga std -m 64 -serial file:serial.log
EMUDEBUG		:= -s -S
GDBFLAGS		:= -x $(DEBUGSCRIPT)

# Gcc-specific files responsible for (amongst other things) calling global con- and destructors
CRTBEGIN		:= $(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND			:= $(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTI			:= $(OBJDIR)/crti.o
CRTN			:= $(OBJDIR)/crtn.o

# Scan the source directory for C, C++ and assembly files (marked by the endings .c, .cc, and .s!)
SRC				:= $(shell $(FIND) -name "*.s")
SRC				:= $(SRC) $(shell $(FIND) -name "*.c")
SRC				:= $(SRC) $(shell $(FIND) -name "*.cc")
# Remove all files from the arch directory
SRC				:= $(patsubst $(SRCDIR)/arch/%,,$(SRC))
# Add the sources from the relevant arch directory again
SRC				:= $(SRC) $(shell $(FIND) $(SRCDIR)/arch/$(ISA) -name "*.s")
SRC				:= $(SRC) $(shell $(FIND) $(SRCDIR)/arch/$(ISA) -name "*.c")
SRC				:= $(SRC) $(shell $(FIND) $(SRCDIR)/arch/$(ISA) -name "*.cc")
# Sort the source list and remove duplicates with that
SRC				:= $(sort $(SRC))

ARCHINC			:= $(SRCDIR)/arch/$(ISA)

# Get the corresponding object files by substituting the endings and the directory
OBJ				:= $(notdir $(SRC))
OBJ				:= $(addprefix $(OBJDIR)/,$(OBJ))
OBJ				:= $(subst .s,.o,$(OBJ))
OBJ				:= $(subst .cc,.o,$(OBJ))
OBJ				:= $(subst .c,.o,$(OBJ))
OBJ				:= $(patsubst %/crti.o,,$(OBJ))
OBJ				:= $(patsubst %/crtn.o,,$(OBJ))
# Dependencies for the build process
DEP				:= $(patsubst %.o,%.d,$(OBJ))
# Tell make where to look for the files in the rules
VPATH			:= $(dir $(SRC))

.PHONY: all build clean debug doc format iso main-build pre-build post-build run 


all: post-build

pre-build:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)
	@echo "Platform: $(PLATFORM)"
	@echo "(BUILD)"

post-build: main-build
	@echo "(BUILD)  done"
	
main-build: pre-build
	@$(MAKE) --no-print-directory build

build: $(BIN)
# The binary depends on the object files which have sources and the crti etc. files from gcc (the link order is important!)
$(BIN): $(CRTI) $(CRTBEGIN) $(OBJ) $(CRTEND) $(CRTN)
ifdef VERBOSE
	@echo "    (LD)  Linking program..."
endif
	@$(LD) -o $(BIN) -T $(LINKERSCRIPT) $(LDFLAGS) $^

# Include the dependency rules (if present; if not, we have to build the obj file anyway)
-include $(DEP)

# Assembly rule
$(OBJDIR)/%.o : %.s Makefile
ifdef VERBOSE
	@echo "    (ASM)  $< --> $@"
endif
	@$(ASM) -MD $(patsubst %.o,%.d,$@) $< -o $@ -I $(INCDIR) -I $(ARCHINC) $(ASMFLAGS) $(DEBUGFLAGS)

# C rule
$(OBJDIR)/%.o : %.c Makefile
ifdef VERBOSE
	@echo "    (C)  $< --> $@"
endif
	@$(CC) -MD -c $< -o $@ -I $(INCDIR) -I $(ARCHINC) $(CCFLAGS) $(DEBUGFLAGS)

# C++ rule
$(OBJDIR)/%.o : %.cc Makefile
ifdef VERBOSE
	@echo "    (CC)  $< --> $@"
endif
	@$(CPP) -MD -c $< -o $@ -I $(INCDIR) -I $(ARCHINC) $(CPPFLAGS) $(DEBUGFLAGS)
	
checkmb:
	@grub-file --is-x86-multiboot $(BIN)
	
checkmb2:
	@grub-file --is-x86-multiboot2 $(BIN)

clean:
	@echo "(CLEAN)"
	
ifdef VERBOSE
	@echo "    (CLEAN)  Deleting binary..."
endif
	-@rm -rf $(BINDIR)/*
ifdef VERBOSE
	@echo "    (CLEAN)  Deleting object files..."
endif
	-@rm -rf $(OBJDIR)/*
ifdef VERBOSE
	@echo "    (CLEAN)  Deleting documentation..."
endif
	-@rm -f $(DOCDIR)/*
ifdef VERBOSE
	@echo "    (CLEAN)  Deleting ISO directory..."
endif
	-@rm -rf $(ISODIR)/*
	
	@echo "(CLEAN)  done"

# TODO: Currenty, debug symbols are linked into the obj files even if we don't want to debug
debug: $(ISO)
	@$(OBJCOPY) --only-keep-debug $(BIN) $(BINDIR)/kernel_symbols.sym
	@echo "(DEBUG)"
	@$(EMU) $(EMUFLAGS) -cdrom $< $(EMUDEBUG) -daemonize && $(GDB) $< $(GDBFLAGS)
#	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(IMG) -S -s -daemonize & $(GDB) $(BINDIR)/$(SYMBOLS) -x $(DEBUGSCRIPT)
#	@rm -f $(BINDIR)/$(SYMBOLS)
	@echo "(DEBUG)  done"
	
doc:
	@echo "(DOC)"
	@rm -r $(DOCDIR)
	@mkdir $(DOCDIR)
	@doxygen $(DOCDIR)/Doxyfile
	@echo "(DOC)  done"

# Use clang-format to automatically apply the style guide to all dirty git files
format:
	@echo "(FORMATTING)"
	@git diff -U0 HEAD^ | python $(FORMATSCRIPT) -i -p2
	@echo "(FORMATTING)  done"

# Create a bootable iso file with grub-mkrescue
iso: $(ISO)

$(ISO): post-build
	@echo "(ISO)"
ifdef VERBOSE
	@echo "    (ISO)  Creating directories..."
endif
	@mkdir -p $(ISODIR)/isodir
	@mkdir -p $(ISODIR)/isodir/boot
	@mkdir -p $(ISODIR)/isodir/boot/grub
ifdef VERBOSE
	@echo "    (ISO)  Copying binaries..."
endif
	@cp $(BIN) $(ISODIR)/isodir/boot/$(notdir $(BIN))
ifdef VERBOSE
	@echo "    (ISO)  Configuring..."
endif
	@echo "set timeout=15\nset default=0\n\nmenuentry \"$(basename $(notdir $(BIN)))\" {\n   multiboot2 /boot/$(notdir $(BIN))\n   boot\n}"\
		> $(ISODIR)/isodir/boot/grub/grub.cfg
	@$(GRUB) --output $(ISO) $(ISODIR)/isodir
	
	@echo "(ISO)  done"

run: $(ISO)
	@echo "(RUN)"
	@$(EMU) $(EMUFLAGS) -cdrom $<
	@echo "(RUN)  done"

