MBRNAME			:= mbr

MBRSYMBOLS		:= $(MBRDIR)/$(BINDIR)/$(MBRNAME)-symbols.sym
MBRBIN			:= $(MBRDIR)/$(BINDIR)/$(MBRNAME).bin
MBRELF			:= $(MBRDIR)/$(BINDIR)/$(MBRNAME).elf
MBRSRC			:= $(shell $(FIND) $(MBRDIR) -name "*.s")
MBROBJ			:= $(notdir $(MBRSRC))
MBROBJ			:= $(addprefix $(MBRDIR)/$(OBJDIR)/,$(MBROBJ))
MBROBJ			:= $(subst .s,.o,$(MBROBJ))
MBRINCDIR		:= $(MBRDIR)/$(SRCDIR)

MBRLDSCRIPT		:= $(MBRDIR)/$(CFGDIR)/linker/ld.script
MBRDEBUGSCRIPT	:= $(MBRDIR)/$(CFGDIR)/debug/gdb-$(ISA).script

MBRDEP			:= $(patsubst %.o,%.d,$(MBROBJ))
VPATH			+= $(dir $(MBRSRC))

build-mbr: $(MBRBIN) $(MBRSYMBOLS)
	@echo "    (MAKE)    Building MBR..."

$(MBRSYMBOLS): $(MBRELF)
ifdef VERBOSE
	@echo "    (OBJCOPY) Creating symbol file..."
endif
	@$(OBJCOPY) --only-keep-debug $(MBRELF) $(MBRSYMBOLS)

$(MBRBIN): $(MBRELF)
ifdef VERBOSE
	@echo "    (OBJCOPY) Stripping debug info..."
endif
	@$(OBJCOPY) -O binary --strip-all $(MBRELF) $(MBRBIN)

$(MBRELF): $(MBROBJ)
ifdef VERBOSE
	@echo "    (LD)      Linking..."
endif
	@$(LD) -T $(MBRLDSCRIPT) -o $(MBRELF) $(LDFLAGS) $^

# Include the dependency rules (if present; if not, we have to build the obj file anyway)
-include $(MBRDEP)

# Assembly rule
$(MBRDIR)/$(OBJDIR)/%.o : %.s
ifdef VERBOSE
	@echo "    (ASM)     $< --> $@"
endif
	@$(ASM) -MD $(patsubst %.o,%.d,$@) $< -o $@ -I $(MBRINCDIR) $(ASMFLAGS)

clean-mbr:
	@echo "    (MAKE)    Cleaning MBR..."
	@rm -rf $(MBRDIR)/$(OBJDIR)/*
	@rm -rf $(MBRDIR)/$(BINDIR)/*

debug-mbr: build-mbr
	@echo "    (MAKE)    Debugging MBR..."
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(MBRBIN) -S -s -daemonize && $(GDB) $(MBRSYMBOLS) -x $(MBRDEBUGSCRIPT)

run-mbr: build-mbr
	@echo "    (MAKE)    Running MBR..."
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(MBRBIN)
