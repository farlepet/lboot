BUILDDIR = build

STAGE1 = stage1/stage1.bin
FLOPPY = boot.img

ifeq ($(VERBOSE), 1)
Q =
else
Q = @
endif

.DEFAULT_GOAL=$(FLOPPY)

$(STAGE1): stage1/stage1.s
	@cd stage1; $(MAKE)

$(FLOPPY): $(STAGE1)
	$(Q) rm -f $@
	$(Q) mkdosfs -C $@ 1440
	$(Q) dd if=$(STAGE1) of=$(FLOPPY) conv=notrunc

emu: $(FLOPPY)
	$(Q) qemu-system-i386 -fda $(FLOPPY) -serial stdio -machine pc -no-reboot

emu-dbg: $(FLOPPY)
	$(Q) qemu-system-i386 -fda $(FLOPPY) -serial stdio -machine pc -no-reboot -S -s

clean:
	$(Q) rm -f $(STAGE1) $(FLOPPY)

.PHONY: clean emu
