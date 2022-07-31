STAGE1     = stage1.bin

ifeq ($(VERBOSE), 1)
Q =
else
Q = @
endif

ASFLAGS = 
LDFLAGS = -T stage1.ld

OBJCOPY := objcopy

.DEFAULT_GOAL=$(STAGE1)

$(STAGE1): $(STAGE1).o
	@echo -e "\033[32m    \033[1mLD\033[21m    \033[34m$<\033[0m"
	$(Q) $(LD) $(LDFLAGS) -o $(STAGE1).elf $<
	$(Q) $(OBJCOPY) -O binary --only-section=.text $(STAGE1).elf $@

$(STAGE1).o: stage1.s
	@echo -e "\033[32m    \033[1mAS\033[21m    \033[34m$<\033[0m"
	$(Q) $(CC) $(ASFLAGS) -c -o $@ $<

$(FLOPPY): $(STAGE1)
	$(Q) rm -f $@
	$(Q) mkdosfs -C $@ 1440
	$(Q) dd if=$(STAGE1) of=$(FLOPPY) conv=notrunc


clean:
	$(Q) rm -f $(STAGE1) $(STAGE1_ELF)

.PHONY: clean
