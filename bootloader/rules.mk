BOOTNAME		:= bootloader

BOOTSYMBOLS		:= $(BOOTDIR)/$(BINDIR)/$(BOOTNAME)-symbols.sym
BOOTBIN			:= $(BOOTDIR)/$(BINDIR)/$(BOOTNAME).bin
BOOTELF			:= $(BOOTDIR)/$(BINDIR)/$(BOOTNAME).elf
BOOTSRCDIR		:= $(BOOTDIR)/$(SRCDIR)
BOOTSRC			:= $(shell $(FIND) $(BOOTSRCDIR) -name "*.s")
BOOTOBJ			:= $(notdir $(BOOTSRC))
BOOTOBJ			:= $(addprefix $(BOOTDIR)/$(OBJDIR)/,$(BOOTOBJ))
BOOTOBJ			:= $(subst .s,.o,$(BOOTOBJ))
BOOTINCDIR		:= $(BOOTSRCDIR)
BOOTARCHINCDIR	:= $(BOOTSRCDIR)/arch/$(ISA)

BOOTLDSCRIPT	:= $(BOOTDIR)/$(CFGDIR)/linker/ld.script
BOOTDEBUGSCRIPT	:= $(BOOTDIR)/$(CFGDIR)/debug/gdb-$(ISA).script

BOOTDEP			:= $(patsubst %.o,%.d,$(BOOTOBJ))
VPATH			+= $(dir $(BOOTSRC))

dir-bootloader:
	@mkdir -p $(BOOTDIR)/$(BINDIR)
	@mkdir -p $(BOOTDIR)/$(OBJDIR)

build-bootloader: dir-bootloader $(BOOTBIN) $(BOOTSYMBOLS)
	@echo "    (MAKE)    Building bootloader..."

$(BOOTSYMBOLS): $(BOOTELF)
	@echo "    (OBJCOPY) Creating symbol file..."
	@$(OBJCOPY) --only-keep-debug $(BOOTELF) $(BOOTSYMBOLS)

$(BOOTBIN): $(BOOTELF)
	@echo "    (OBJCOPY) Stripping debug info..."
	@$(OBJCOPY) -O binary --strip-all $(BOOTELF) $(BOOTBIN)

$(BOOTELF): $(BOOTOBJ)
ifdef VERBOSE
	@echo "    (LD)      Linking..."
endif
	@$(LD) -T $(BOOTLDSCRIPT) -o $(BOOTELF) $(LDFLAGS) $^

# Include the dependency rules (if present; if not, we have to build the obj file anyway)
-include $(BOOTDEP)

# Assembly rule
$(BOOTDIR)/$(OBJDIR)/%.o : %.s
ifdef VERBOSE
	@echo "    (ASM)     $< --> $@"
endif
	@$(ASM) -MD $(patsubst %.o,%.d,$@) $< -o $@ -I $(BOOTINCDIR) -I $(BOOTARCHINCDIR) $(ASMFLAGS)

clean-bootloader:
	@echo "    (MAKE)    Cleaning bootloader..."
	@rm -rf $(BOOTDIR)/$(OBJDIR)/*
	@rm -rf $(BOOTDIR)/$(BINDIR)/*

debug-bootloader: build-bootloader
	@echo "    (MAKE)    Debugging bootloader..."
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(BOOTBIN) -S -s -daemonize && $(GDB) $(BOOTSYMBOLS) -x $(BOOTDEBUGSCRIPT)

run-bootloader: build-bootloader
	@echo "    (MAKE)    Running bootloader..."
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(BOOTBIN)
