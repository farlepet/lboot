STAGE2  = $(BUILDDIR)/stage2/stage2.bin

S2_ASFLAGS = -I inc
S2_LDFLAGS = -T stage2.ld

$(STAGE2): $(STAGE2).o
	@echo -e "\033[32m    \033[1mLD\033[21m    \033[34m$<\033[0m"
	$(Q) $(LD) $(S2_LDFLAGS) -o $(STAGE2).elf $<
	$(Q) $(OBJCOPY) -O binary --only-section=.text $(STAGE2).elf $@

$(STAGE2).o: src/startup/startup.s $(BUILDDIR)/stage2
	@echo -e "\033[32m    \033[1mAS\033[21m    \033[34m$<\033[0m"
	$(Q) $(CC) $(S2_ASFLAGS) -c -o $@ $<

$(BUILDDIR)/stage2: $(BUILDDIR)
	$(Q) mkdir -p $@

stage2_clean:
	$(Q) rm -f $(STAGE2) $(STAGE2).o $(STAGE2).elf

.PHONY: stage2_clean
