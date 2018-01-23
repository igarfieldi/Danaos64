BOOTNAME		:= bootloader

BOOTSYMBOLS		:= $(BOOTDIR)/$(BINDIR)/$(BOOTNAME)-symbols.sym
BOOTBIN			:= $(BOOTDIR)/$(BINDIR)/$(BOOTNAME).bin
BOOTELF			:= $(BOOTDIR)/$(BINDIR)/$(BOOTNAME).elf
BOOTIMG			:= $(BOOTDIR)/$(BINDIR)/$(BOOTNAME).img
BOOTSRCDIR		:= $(BOOTDIR)/$(SRCDIR)
BOOTSRC			:= $(shell $(FIND) $(BOOTSRCDIR) -name "*.s")
BOOTSRC			+= $(shell $(FIND) $(BOOTSRCDIR) -name "*.c")
BOOTSRC			+= $(shell $(FIND) $(BOOTSRCDIR) -name "*.cc")
# Remove all files from the arch directory
BOOTSRC			:= $(patsubst $(BOOTSRCDIR)/arch/%,,$(BOOTSRC))
# Add the sources from the relevant arch directory again
BOOTSRC			+= $(shell $(FIND) $(BOOTSRCDIR)/arch/$(ISA) -name "*.s")
BOOTSRC			+= $(shell $(FIND) $(BOOTSRCDIR)/arch/$(ISA) -name "*.c")
BOOTSRC			+= $(shell $(FIND) $(BOOTSRCDIR)/arch/$(ISA) -name "*.cc")
BOOTOBJ			:= $(patsubst $(BOOTDIR)/$(SRCDIR)/%,$(BOOTDIR)/$(OBJDIR)/%,$(BOOTSRC))
BOOTOBJ			:= $(subst .s,.o,$(BOOTOBJ))
BOOTOBJ			:= $(subst .cc,.o,$(BOOTOBJ))
BOOTOBJ			:= $(subst .c,.o,$(BOOTOBJ))
BOOTINCDIR		:= $(BOOTSRCDIR)
BOOTARCHINCDIR	:= $(BOOTSRCDIR)/arch/$(ISA)

BOOTLDSCRIPT	:= $(BOOTDIR)/$(CFGDIR)/linker/ld.script
BOOTDEBUGSCRIPT	:= $(BOOTDIR)/$(CFGDIR)/debug/gdb-$(ISA).script

BOOTDEP			:= $(patsubst %.o,%.d,$(BOOTOBJ))

# TODO: properly configure this makefile!

dir-bootloader:
	@mkdir -p $(BOOTDIR)/$(BINDIR)
	@mkdir -p $(BOOTDIR)/$(OBJDIR)

build-bootloader: dir-bootloader $(BOOTBIN) $(BOOTSYMBOLS)
	@echo "    (MAKE)    Building bootloader..."

$(BOOTSYMBOLS): $(BOOTELF)
	@echo "    (OBJCOPY) Creating symbol file..."
	@i386-elf-objcopy --only-keep-debug $(BOOTELF) $(BOOTSYMBOLS)

$(BOOTBIN): $(BOOTELF)
	@echo "    (OBJCOPY) Stripping debug info..."
	@i386-elf-objcopy -O binary --strip-all $(BOOTELF) $(BOOTBIN)

$(BOOTELF): $(BOOTOBJ)
ifdef VERBOSE
	@echo "    (LD)      Linking..."
endif
	@i386-elf-g++ -T $(BOOTLDSCRIPT) -o $(BOOTELF) $(LDFLAGS) $(i386-LDFLAGS) $^ $(TARGETLIBGCC)

# Include the dependency rules (if present; if not, we have to build the obj file anyway)
-include $(BOOTDEP)

# Assembly rule
$(BOOTDIR)/$(OBJDIR)/%.o : $(BOOTDIR)/$(SRCDIR)/%.s
ifdef VERBOSE
	@echo "    (ASM)     $< --> $@"
endif
	@mkdir -p $(dir $@)
	@i386-elf-as -MD $(patsubst %.o,%.d,$@) $< -o $@ -I $(BOOTINCDIR) -I $(BOOTARCHINCDIR) $(ASMFLAGS) $(i386-ASMFLAGS)

# C rule
$(BOOTDIR)/$(OBJDIR)/%.o : $(BOOTDIR)/$(SRCDIR)/%.c Makefile
ifdef VERBOSE
	@echo "    (C)       $< --> $@"
endif
	@mkdir -p $(dir $@)
	@i386-elf-gcc -MD -c $< -o $@ -I $(BOOTINCDIR) -I $(BOOTARCHINCDIR) $(CCFLAGS) $(i386-CCFLAGS)

# C++ rule
$(BOOTDIR)/$(OBJDIR)/%.o : $(BOOTDIR)/$(SRCDIR)/%.cc Makefile
ifdef VERBOSE
	@echo "    (CC)      $< --> $@"
endif
	@mkdir -p $(dir $@)
	@i386-elf-g++ -MD -c $< -o $@ -I $(BOOTINCDIR) -I $(BOOTARCHINCDIR) $(CPPFLAGS) $(i386-CPPFLAGS)

clean-bootloader:
	@echo "    (MAKE)    Cleaning bootloader..."
	@rm -rf $(BOOTDIR)/$(OBJDIR)/*
	@rm -rf $(BOOTDIR)/$(BINDIR)/*
	
img-bootloader: $(BOOTIMG)

$(BOOTIMG): build-bootloader build-mbr build-imagemk
	@$(IMAGEMKBIN) -m $(MBRBIN) -b $(BOOTBIN) 506 507 -s 512 $(BOOTIMG)

debug-bootloader: $(BOOTIMG)
	@echo "    (MAKE)    Debugging bootloader..."
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(BOOTIMG) -S -s -daemonize && $(GDB) $(BOOTSYMBOLS) -x $(BOOTDEBUGSCRIPT)

run-bootloader: $(BOOTIMG)
	@echo "    (MAKE)    Running bootloader..."
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(BOOTIMG)

