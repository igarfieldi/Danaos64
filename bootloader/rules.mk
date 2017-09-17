BOOTNAME		:= bootloader
BOOTSYMBOLS		:= $(BOOTNAME)-symbols.sym

BOOTBIN			:= $(BOOTDIR)/$(BINDIR)/$(BOOTNAME).bin
BOOTELF			:= $(BOOTDIR)/$(BINDIR)/$(BOOTNAME).elf
BOOTSRC			:= $(shell $(FIND) $(BOOTDIR) -name "*.s")
BOOTOBJ			:= $(notdir $(BOOTSRC))
BOOTOBJ			:= $(addprefix $(BOOTDIR)/$(OBJDIR)/,$(BOOTOBJ))
BOOTOBJ			:= $(subst .s,.o,$(BOOTOBJ))
BOOTINCDIR		:= $(BOOTDIR)/$(SRCDIR)

BOOTLDSCRIPT	:= $(BOOTDIR)/$(CFGDIR)/linker/ld.script
BOOTDEBUGSCRIPT	:= $(BOOTDIR)/$(CFGDIR)/debug/gdb.script

BOOTDEP			:= $(patsubst %.o,%.d,$(BOOTOBJ))
VPATH			+= $(dir $(BOOTSRC))

build-bootloader: $(BOOTBIN) $(BOOTSYMBOLS)

$(BOOTSYMBOLS): $(BOOTELF)
	@$(OBJCOPY) --only-keep-debug $(BOOTELF) $(BOOTDIR)/$(BINDIR)/$(BOOTSYMBOLS)

$(BOOTBIN): $(BOOTELF)
	@$(OBJCOPY) -O binary --strip-all $(BOOTELF) $(BOOTBIN)

$(BOOTELF): $(BOOTOBJ)
	@$(LD) -T $(BOOTLDSCRIPT) -o $(BOOTELF) $(BOOTOBJ)

# Include the dependency rules (if present; if not, we have to build the obj file anyway)
-include $(BOOTDEP)

# Assembly rule
$(BOOTDIR)/$(OBJDIR)/%.o : %.s
ifdef VERBOSE
	@echo "    (ASM)  $< --> $@"
endif
	@$(ASM) -MD $(patsubst %.o,%.d,$@) $< -o $@ -I $(BOOTINCDIR) $(ASMFLAGS)

clean-bootloader:
	@rm -rf $(BOOTDIR)/$(OBJDIR)/*
	@rm -rf $(BOOTDIR)/$(BINDIR)/*

debug-bootloader: build-bootloader
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(BOOTBIN) -S -s -daemonize && $(GDB) $(BOOTDIR)/$(BINDIR)/$(BOOTSYMBOLS) -x $(BOOTDEBUGSCRIPT)

run-bootloader: build-bootloader
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(BOOTBIN)
