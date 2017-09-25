KERNELNAME			:= kernel

KERNELSYMBOLS		:= $(KERNELDIR)/$(BINDIR)/$(KERNELNAME)-symbols.sym
KERNELELF			:= $(KERNELDIR)/$(BINDIR)/$(KERNELNAME).elf
KERNELELFSTRIPPED	:= $(KERNELDIR)/$(BINDIR)/$(KERNELNAME)-nodebug.elf
KERNELIMG			:= $(KERNELDIR)/$(BINDIR)/$(KERNELNAME).img
KERNELINCDIR		:= $(KERNELDIR)/$(SRCDIR)


# Scripts for the linker, the debugger, and the code formatter
KERNELLDSCRIPT		:= $(KERNELDIR)/$(CFGDIR)/linker/linker-$(ISA).ld
KERNELDEBUGSCRIPT	:= $(KERNELDIR)/$(CFGDIR)/debug/gdb-$(ISA).script
KERNELFORMATSCRIPT	:= $(KERNELDIR)/$(CFGDIR)/format/clang-format-diff.py

# Scan the source directory for C, C++ and assembly files (marked by the endings .c, .cc, and .s!)
KERNELSRC			:= $(shell $(FIND) $(KERNELDIR) -name "*.s")
KERNELSRC			+= $(shell $(FIND) $(KERNELDIR) -name "*.c")
KERNELSRC			+= $(shell $(FIND) $(KERNELDIR) -name "*.cc")
# Remove all files from the arch directory
KERNELSRC			:= $(patsubst $(KERNELDIR)/$(SRCDIR)/arch/%,,$(KERNELSRC))
# Add the sources from the relevant arch directory again
KERNELSRC			+= $(shell $(FIND) $(KERNELDIR)/$(SRCDIR)/arch/$(ISA) -name "*.s")
KERNELSRC			+= $(shell $(FIND) $(KERNELDIR)/$(SRCDIR)/arch/$(ISA) -name "*.c")
KERNELSRC			+= $(shell $(FIND) $(KERNELDIR)/$(SRCDIR)/arch/$(ISA) -name "*.cc")
# Sort the source list and remove duplicates with that
KERNELSRC			:= $(sort $(KERNELSRC))

KERNELARCHINC		:= $(KERNELDIR)/$(SRCDIR)/arch/$(ISA)

# Get the corresponding object files by substituting the endings and the directory
KERNELOBJ			:= $(patsubst $(KERNELDIR)/$(SRCDIR)/%,$(KERNELDIR)/$(OBJDIR)/%,$(KERNELSRC))
KERNELOBJ			:= $(subst .s,.o,$(KERNELOBJ))
KERNELOBJ			:= $(subst .cc,.o,$(KERNELOBJ))
KERNELOBJ			:= $(subst .c,.o,$(KERNELOBJ))
KERNELOBJ			:= $(patsubst %/crti.o,,$(KERNELOBJ))
KERNELOBJ			:= $(patsubst %/crtn.o,,$(KERNELOBJ))

# GCC-specific files responsible for (amongst other things) calling global con- and destructors
KERNELCRTBEGIN		:= $(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
KERNELCRTEND		:= $(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
KERNELCRTI			:= $(patsubst $(KERNELDIR)/$(SRCDIR)/%,$(KERNELDIR)/$(OBJDIR)/%,$(shell $(FIND) $(KERNELDIR)/$(SRCDIR)/arch/$(ISA) -name "crti.s"))
KERNELCRTN			:= $(patsubst $(KERNELDIR)/$(SRCDIR)/%,$(KERNELDIR)/$(OBJDIR)/%,$(shell $(FIND) $(KERNELDIR)/$(SRCDIR)/arch/$(ISA) -name "crtn.s"))
KERNELCRTI			:= $(subst .s,.o,$(KERNELCRTI))
KERNELCRTN			:= $(subst .s,.o,$(KERNELCRTN))

# Dependencies for the build process
KERNELDEP			:= $(patsubst %.o,%.d,$(KERNELOBJ))

dir-kernel:
	@mkdir -p $(KERNELDIR)/$(BINDIR)
	@mkdir -p $(KERNELDIR)/$(OBJDIR)

build-kernel: dir-kernel $(KERNELELFSTRIPPED) $(KERNELSYMBOLS)
	@echo "    (MAKE)    Building kernel..."
	
$(KERNELSYMBOLS): $(KERNELELF)
	@echo "    (OBJCOPY) Creating symbol file..."
	@$(OBJCOPY) --only-keep-debug $(KERNELELF) $(KERNELSYMBOLS)
	
$(KERNELELFSTRIPPED): $(KERNELELF)
	@echo "    (OBJCOPY) Stripping debug info..."
	@$(OBJCOPY) --strip-debug $(KERNELELF) $(KERNELELFSTRIPPED)

# The binary depends on the object files which have sources and the crti etc. files from gcc (the link order is important!)
$(KERNELELF): $(KERNELCRTI) $(KERNELCRTBEGIN) $(KERNELOBJ) $(KERNELCRTEND) $(KERNELCRTN)
	@echo "    (LD)      Linking..."
	@$(LD) -o $(KERNELELF) -T $(KERNELLDSCRIPT) $(TARGETLDFLAGS) $^

# Include the dependency rules (if present; if not, we have to build the obj file anyway)
-include $(KERNELDEP)

# Assembly rule
$(KERNELDIR)/$(OBJDIR)/%.o : $(KERNELDIR)/$(SRCDIR)/%.s Makefile
ifdef VERBOSE
	@echo "    (ASM)     $< --> $@"
endif
	@mkdir -p $(dir $@)
	@$(ASM) -MD $(patsubst %.o,%.d,$@) $< -o $@ -I $(KERNELINCDIR) -I $(KERNELARCHINC) $(TARGETASMFLAGS)

# C rule
$(KERNELDIR)/$(OBJDIR)/%.o : $(KERNELDIR)/$(SRCDIR)/%.c Makefile
ifdef VERBOSE
	@echo "    (C)       $< --> $@"
endif
	@mkdir -p $(dir $@)
	@$(CC) -MD -c $< -o $@ -I $(KERNELINCDIR) -I $(KERNELARCHINC) $(TARGETCCFLAGS)

# C++ rule
$(KERNELDIR)/$(OBJDIR)/%.o : $(KERNELDIR)/$(SRCDIR)/%.cc Makefile
ifdef VERBOSE
	@echo "    (CC)      $< --> $@"
endif
	@mkdir -p $(dir $@)
	@$(CPP) -MD -c $< -o $@ -I $(KERNELINCDIR) -I $(KERNELARCHINC) $(TARGETCPPFLAGS)

clean-kernel:
	@echo "    (MAKE)    Cleaning kernel..."
	-@rm -rf $(KERNELDIR)/$(BINDIR)/*
	-@rm -rf $(KERNELDIR)/$(OBJDIR)/*

debug-kernel: $(KERNELIMG)
	@echo "    (DEBUG)"
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(KERNELIMG) -S -s -daemonize && $(GDB) $(KERNELSYMBOLS) -x $(KERNELDEBUGSCRIPT)
	@echo "    (DEBUG)   done"
	
doc-kernel:
	@echo "    (DOC)"
	@rm -r $(KERNELDIR)/$(DOCDIR)
	@mkdir $(KERNELDIR)/$(DOCDIR)
	@doxygen $(KERNELDIR)/$(DOCDIR)/Doxyfile
	@echo "    (DOC)     done"

# Use clang-format to automatically apply the style guide to all dirty git files
format-kernel:
	@echo "    (FORMAT)"
	@git diff -U0 HEAD^ | python $(KERNELFORMATSCRIPT) -i -p2
	@echo "    (FORMAT)  done"

img-kernel: $(KERNELIMG)


$(KERNELIMG): build-kernel
	@echo "(ISO)"
ifdef VERBOSE
	@echo "    (ISO)     Creating directories..."
endif
	@mkdir -p $(KERNELDIR)/$(BINDIR)/isodir
	@mkdir -p $(KERNELDIR)/$(BINDIR)/isodir/boot
	@mkdir -p $(KERNELDIR)/$(BINDIR)/isodir/boot/grub
ifdef VERBOSE
	@echo "    (ISO)     Copying binaries..."
endif
	@cp $(KERNELELFSTRIPPED) $(KERNELDIR)/$(BINDIR)/isodir/boot/$(notdir $(BIN))
ifdef VERBOSE
	@echo "    (ISO)     Configuring..."
endif
	@echo "set timeout=0\nset default=0\n\nmenuentry \"$(basename $(notdir $(KERNELELFSTRIPPED)))\" {\n   multiboot2 /boot/$(notdir $(KERNELELFSTRIPPED))\n   boot\n}"\
		> $(KERNELDIR)/$(BINDIR)/isodir/boot/grub/grub.cfg
	@$(GRUB) --output $(KERNELIMG) $(KERNELDIR)/$(BINDIR)/isodir
	
	@echo "    (ISO)     done"

run-kernel: $(KERNELIMG)
	@echo "    (RUN)"
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(KERNELIMG)
	@echo "    (RUN)     done"

