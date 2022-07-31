BUILDDIR = build

STAGE1 = stage1/stage1.bin
FLOPPY = boot.img

ifeq ($(VERBOSE), 1)
Q =
else
Q = @
endif

.DEFAULT_GOAL=$(FLOPPY)

MKDOSFS_FLAGS = -n "LBOOT" -F 12 

$(STAGE1): stage1/stage1.s
	@cd stage1; $(MAKE)

$(FLOPPY): $(STAGE1)
	$(Q) rm -f $@
	$(Q) mkdosfs $(MKDOSFS_FLAGS) -C $@ 1440
	$(Q) dd if=$(STAGE1) of=$(FLOPPY) conv=notrunc iflag=count_bytes,skip_bytes oflag=seek_bytes count=11
	$(Q) dd if=$(STAGE1) of=$(FLOPPY) conv=notrunc iflag=count_bytes,skip_bytes oflag=seek_bytes skip=30 seek=30

emu: $(FLOPPY)
	$(Q) qemu-system-i386 -fda $(FLOPPY) -serial stdio -machine pc -no-reboot

emu-dbg: $(FLOPPY)
	$(Q) qemu-system-i386 -fda $(FLOPPY) -serial stdio -machine pc -no-reboot -S -s

clean:
	$(Q) rm -f $(STAGE1) $(FLOPPY)

.PHONY: clean emu emu-dbg
