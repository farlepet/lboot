BUILDDIR = build

FLOPPY = boot.img

ifeq ($(VERBOSE), 1)
Q =
else
Q = @
endif

.DEFAULT_GOAL=$(FLOPPY)

MKDOSFS_FLAGS = -n "LBOOT" -F 12 

STAGE2_MAP=build/stage2.map

OBJCOPY       := objcopy
SECTOR_MAPPER := tools/sector_mapper/sector_mapper

include stage1.mk
include stage2.mk

$(FLOPPY): $(STAGE1) $(STAGE2) $(SECTOR_MAPPER)
	$(Q) rm -f $@.tmp
	$(Q) mkdosfs $(MKDOSFS_FLAGS) -C $@.tmp 1440
	$(Q) tools/lboot_prepare.sh $@.tmp
	$(Q) mcopy -D oO -i $@.tmp lboot.cfg.example "::/LBOOT/LBOOT.CFG"
# Only update the target if the previous commands succeed
	$(Q) mv $@.tmp $@


$(BUILDDIR):
	$(Q) mkdir -p $@

$(SECTOR_MAPPER):
	$(Q) cd tools/sector_mapper; $(MAKE)

emu: $(FLOPPY)
	$(Q) qemu-system-i386 -fda $(FLOPPY) -serial stdio -machine pc -no-reboot

# Create socket for COM1
# NOTE: For certain applications, like XMODEM transfers, direct read/write of
# the socket is too fast, as qemu does not actually respect the 8250's set baud
# rate unless using a physical port. 
emu-sock: $(FLOPPY)
	$(Q) qemu-system-i386 -fda $(FLOPPY) -machine pc -no-reboot                         \
	                      -chardev socket,id=serial0,path=./com1.sock,server=on,debug=9 \
                          -serial chardev:serial0

# Emulate more realistic floppy disk speeds
emu-slow: $(FLOPPY)
	$(Q) qemu-system-i386 -drive file=$(FLOPPY),if=floppy,format=raw,bps=4000 \
		                  -serial stdio -machine pc -no-reboot

# Enable GDB server
emu-dbg: $(FLOPPY)
	$(Q) qemu-system-i386 -fda $(FLOPPY) -serial stdio -machine pc -no-reboot -S -s

emu-sock-dbg: $(FLOPPY)
	$(Q) qemu-system-i386 -fda $(FLOPPY) -machine pc -no-reboot -S -s           \
	                      -chardev socket,id=serial0,path=./com1.sock,server=on \
                          -serial chardev:serial0


clean: stage1_clean stage2_clean
	$(Q) rm -f $(STAGE1) $(FLOPPY)

.PHONY: clean emu emu-dbg
