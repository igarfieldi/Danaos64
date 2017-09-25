BOOTNAME		:= bootloader

BOOTSYMBOLS		:= $(BOOTDIR)/$(BINDIR)/$(BOOTNAME)-symbols.sym
BOOTBIN			:= $(BOOTDIR)/$(BINDIR)/$(BOOTNAME).bin
BOOTELF			:= $(BOOTDIR)/$(BINDIR)/$(BOOTNAME).elf
BOOTIMG			:= $(BOOTDIR)/$(BINDIR)/$(BOOTNAME).img
BOOTSRCDIR		:= $(BOOTDIR)/$(SRCDIR)
BOOTSRC			:= $(shell $(FIND) $(BOOTSRCDIR) -name "*.s")
BOOTSRC			+= $(shell $(FIND) $(BOOTSRCDIR) -name "*.cc")
# Remove all files from the arch directory
BOOTSRC			:= $(patsubst $(BOOTSRCDIR)/arch/%,,$(BOOTSRC))
# Add the sources from the relevant arch directory again
BOOTSRC			+= $(shell $(FIND) $(BOOTSRCDIR)/arch/$(ISA) -name "*.s")
BOOTSRC			+= $(shell $(FIND) $(BOOTSRCDIR)/arch/$(ISA) -name "*.cc")
BOOTOBJ			:= $(patsubst $(BOOTDIR)/$(SRCDIR)/%,$(BOOTDIR)/$(OBJDIR)/%,$(BOOTSRC))
BOOTOBJ			:= $(subst .s,.o,$(BOOTOBJ))
BOOTOBJ			:= $(subst .cc,.o,$(BOOTOBJ))
BOOTINCDIR		:= $(BOOTSRCDIR)
BOOTARCHINCDIR	:= $(BOOTSRCDIR)/arch/$(ISA)

BOOTASMFLAGS	:= -ggdb
BOOTCPPFLAGS	:= -ffreestanding -O0 -std=c++17 -fno-exceptions -fno-rtti -ggdb -Wall -Wextra -m32

BOOTLDSCRIPT	:= $(BOOTDIR)/$(CFGDIR)/linker/ld.script
BOOTDEBUGSCRIPT	:= $(BOOTDIR)/$(CFGDIR)/debug/gdb-$(ISA).script

BOOTDEP			:= $(patsubst %.o,%.d,$(BOOTOBJ))

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
$(BOOTDIR)/$(OBJDIR)/%.o : $(BOOTDIR)/$(SRCDIR)/%.s
ifdef VERBOSE
	@echo "    (ASM)     $< --> $@"
endif
	@mkdir -p $(dir $@)
	@$(ASM) -MD $(patsubst %.o,%.d,$@) $< -o $@ -I $(BOOTINCDIR) -I $(BOOTARCHINCDIR) $(BOOTASMFLAGS)

# C++ rule
$(BOOTDIR)/$(OBJDIR)/%.o : $(BOOTDIR)/$(SRCDIR)/%.cc Makefile
ifdef VERBOSE
	@echo "    (CC)      $< --> $@"
endif
	@mkdir -p $(dir $@)
	@$(CPP) -MD -c $< -o $@ -I $(BOOTINCDIR) -I $(BOOTARCHINCDIR) $(BOOTCPPFLAGS)

clean-bootloader:
	@echo "    (MAKE)    Cleaning bootloader..."
	@rm -rf $(BOOTDIR)/$(OBJDIR)/*
	@rm -rf $(BOOTDIR)/$(BINDIR)/*
	
img-bootloader: $(BOOTIMG)

$(BOOTIMG): build-bootloader build-imagemk
	@$(IMAGEMK) -b $(BOOTBIN) 502 503 -s 512 $(BOOTIMG)

debug-bootloader: $(BOOTIMG)
	@echo "    (MAKE)    Debugging bootloader..."
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(BOOTIMG) -S -s -daemonize && $(GDB) $(BOOTSYMBOLS) -x $(BOOTDEBUGSCRIPT)

run-bootloader: $(BOOTIMG)
	@echo "    (MAKE)    Running bootloader..."
	@$(QEMU) $(QEMUFLAGS) -drive format=raw,file=$(BOOTIMG)

