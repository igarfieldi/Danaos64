MBRNAME			:= mbr
MBRSYMBOLS		:= $(MBRNAME)-symbols.sym

MBRBIN			:= $(MBRDIR)/$(BINDIR)/$(MBRNAME).bin
MBRELF			:= $(MBRDIR)/$(BINDIR)/$(MBRNAME).elf
MBRSRC			:= $(shell $(FIND) $(MBRDIR) -name "*.s")
MBROBJ			:= $(notdir $(MBRSRC))
MBROBJ			:= $(addprefix $(MBRDIR)/$(OBJDIR)/,$(MBROBJ))
MBROBJ			:= $(subst .s,.o,$(MBROBJ))
MBRINCDIR		:= $(MBRDIR)/$(SRCDIR)

MBRLDSCRIPT		:= $(MBRDIR)/$(CFGDIR)/linker/ld.script
MBRDEBUGSCRIPT	:= $(MBRDIR)/$(CFGDIR)/debug/gdb.script

MBRDEP			:= $(patsubst %.o,%.d,$(MBROBJ))
VPATH			+= $(dir $(MBRSRC))

build-mbr: $(MBRBIN) $(MBRSYMBOLS)

$(MBRSYMBOLS): $(MBRELF)
	@$(OBJCOPY) --only-keep-debug $(MBRELF) $(MBRDIR)/$(BINDIR)/$(MBRSYMBOLS)

$(MBRBIN): $(MBRELF)
	@$(OBJCOPY) -O binary --strip-all $(MBRELF) $(MBRBIN)

$(MBRELF): $(MBROBJ)
	@$(LD) -T $(MBRLDSCRIPT) -o $(MBRELF) $(MBROBJ)

# Include the dependency rules (if present; if not, we have to build the obj file anyway)
-include $(MBRDEP)

# Assembly rule
$(MBRDIR)/$(OBJDIR)/%.o : %.s
ifdef VERBOSE
	@echo "    (ASM)  $< --> $@"
endif
	@$(ASM) -MD $(patsubst %.o,%.d,$@) $< -o $@ -I $(MBRINCDIR) $(ASMFLAGS)

clean-mbr:
	@rm -rf $(MBRDIR)/$(OBJDIR)/*
	@rm -rf $(MBRDIR)/$(BINDIR)/*

debug-mbr: build-mbr
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(MBRBIN) -S -s -daemonize && $(GDB) $(MBRDIR)/$(BINDIR)/$(MBRSYMBOLS) -x $(MBRDEBUGSCRIPT)

run-mbr: build-mbr
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(MBRBIN)
