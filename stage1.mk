# Stage 1 bootloader

S1_BUILDDIR = $(BUILDDIR)/stage1

STAGE1  = $(S1_BUILDDIR)/stage1.bin

S1_ASFLAGS = -I stage1
S1_LDFLAGS = -T stage1/stage1.ld

$(STAGE1): $(STAGE1).o
	@echo -e "\033[32m    \033[1mLD\033[21m    \033[34m$<\033[0m"
	$(Q) $(LD) $(S1_LDFLAGS) -o $(STAGE1).elf $<
	$(Q) $(OBJCOPY) -O binary --only-section=.text $(STAGE1).elf $@

$(STAGE1).o: stage1/stage1.s
	@echo -e "\033[32m    \033[1mAS\033[21m    \033[34m$<\033[0m"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(CC) $(S1_ASFLAGS) -c -o $@ $<

stage1_clean:
	$(Q) rm -f $(STAGE1) $(STAGE1).o $(STAGE1).elf

.PHONY: stage1_clean

