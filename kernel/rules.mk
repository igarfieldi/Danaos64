KERNELNAME			:= kernel

KERNELSYMBOLS		:= $(KERNELDIR)/$(BINDIR)/$(KERNELNAME)-symbols.sym
KERNELBIN			:= $(KERNELDIR)/$(BINDIR)/$(KERNELNAME)-no-debug.elf
KERNELELF			:= $(KERNELDIR)/$(BINDIR)/$(KERNELNAME).elf
KERNELISO			:= $(KERNELDIR)/$(BINDIR)/$(KERNELNAME).iso
KERNELINCDIR		:= $(KERNELDIR)/$(SRCDIR)


# Scripts for the linker, the debugger, and the code formatter
KERNELLDSCRIPT		:= $(KERNELDIR)/$(CFGDIR)/linker/linker-$(ISA).ld
KERNELDEBUGSCRIPT	:= $(KERNELDIR)/$(CFGDIR)/debug/gdb-$(ISA).script
KERNELFORMATSCRIPT	:= $(KERNELDIR)/$(CFGDIR)/format/clang-format-diff.py

# GCC-specific files responsible for (amongst other things) calling global con- and destructors
KERNELCRTBEGIN		:= $(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
KERNELCRTEND		:= $(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
KERNELCRTI			:= $(KERNELDIR)/$(OBJDIR)/crti.o
KERNELCRTN			:= $(KERNELDIR)/$(OBJDIR)/crtn.o

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
KERNELOBJ			:= $(notdir $(KERNELSRC))
KERNELOBJ			:= $(addprefix $(KERNELDIR)/$(OBJDIR)/,$(KERNELOBJ))
KERNELOBJ			:= $(subst .s,.o,$(KERNELOBJ))
KERNELOBJ			:= $(subst .cc,.o,$(KERNELOBJ))
KERNELOBJ			:= $(subst .c,.o,$(KERNELOBJ))
KERNELOBJ			:= $(patsubst %/crti.o,,$(KERNELOBJ))
KERNELOBJ			:= $(patsubst %/crtn.o,,$(KERNELOBJ))
# Dependencies for the build process
KERNELDEP			:= $(patsubst %.o,%.d,$(KERNELOBJ))
VPATH				+= $(dir $(KERNELSRC))


build-kernel: $(KERNELBIN) $(KERNELSYMBOLS)
	@echo "    (MAKE)    Building kernel..."
	

$(KERNELSYMBOLS): $(KERNELELF)
	@echo "    (OBJCOPY) Creating symbol file..."
	@$(OBJCOPY) --only-keep-debug $(KERNELELF) $(KERNELSYMBOLS)
	
$(KERNELBIN): $(KERNELELF)
	@echo "    (OBJCOPY) Stripping debug info..."
	@$(OBJCOPY) --strip-debug $(KERNELELF) $(KERNELBIN)

# The binary depends on the object files which have sources and the crti etc. files from gcc (the link order is important!)
$(KERNELELF): $(KERNELCRTI) $(KERNELCRTBEGIN) $(KERNELOBJ) $(KERNELCRTEND) $(KERNELCRTN)
	@echo "    (LD)      Linking..."
	@$(LD) -o $(KERNELELF) -T $(KERNELLDSCRIPT) $(LDFLAGS) $^

# Include the dependency rules (if present; if not, we have to build the obj file anyway)
-include $(KERNELDEP)

# Assembly rule
$(KERNELDIR)/$(OBJDIR)/%.o : %.s Makefile
ifdef VERBOSE
	@echo "    (ASM)     $< --> $@"
endif
	@$(ASM) -MD $(patsubst %.o,%.d,$@) $< -o $@ -I $(KERNELINCDIR) -I $(KERNELARCHINC) $(ASMFLAGS)

# C rule
$(KERNELDIR)/$(OBJDIR)/%.o : %.c Makefile
ifdef VERBOSE
	@echo "    (C)       $< --> $@"
endif
	@$(CC) -MD -c $< -o $@ -I $(KERNELINCDIR) -I $(KERNELARCHINC) $(CCFLAGS)

# C++ rule
$(KERNELDIR)/$(OBJDIR)/%.o : %.cc Makefile
ifdef VERBOSE
	@echo "    (CC)      $< --> $@"
endif
	@$(CPP) -MD -c $< -o $@ -I $(KERNELINCDIR) -I $(KERNELARCHINC) $(CPPFLAGS)

clean-kernel:
	@echo "    (MAKE)    Cleaning kernel..."
	-@rm -rf $(KERNELDIR)/$(BINDIR)/*
	-@rm -rf $(KERNELDIR)/$(OBJDIR)/*

debug-kernel: $(KERNELISO)
	@echo "    (DEBUG)"
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(KERNELISO) -S -s -daemonize && $(GDB) $(KERNELSYMBOLS) -x $(KERNELDEBUGSCRIPT)
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

iso-kernel: $(KERNELISO)


$(KERNELISO): build-kernel
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
	@cp $(KERNELBIN) $(KERNELDIR)/$(BINDIR)/isodir/boot/$(notdir $(BIN))
ifdef VERBOSE
	@echo "    (ISO)     Configuring..."
endif
	@echo "set timeout=0\nset default=0\n\nmenuentry \"$(basename $(notdir $(KERNELBIN)))\" {\n   multiboot2 /boot/$(notdir $(KERNELBIN))\n   boot\n}"\
		> $(KERNELDIR)/$(BINDIR)/isodir/boot/grub/grub.cfg
	@$(GRUB) --output $(KERNELISO) $(KERNELDIR)/$(BINDIR)/isodir
	
	@echo "    (ISO)     done"

run-kernel: $(KERNELISO)
	@echo "    (RUN)"
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(KERNELISO)
	@echo "    (RUN)     done"

