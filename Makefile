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
	$(Q) dd if=$(STAGE1) of=$@.tmp conv=notrunc iflag=count_bytes,skip_bytes oflag=seek_bytes count=11
	$(Q) dd if=$(STAGE1) of=$@.tmp conv=notrunc iflag=count_bytes,skip_bytes oflag=seek_bytes skip=90 seek=90
	$(Q) mcopy -i $@.tmp $(STAGE2) ::/STAGE2.BIN
	$(Q) $(SECTOR_MAPPER) 1 $@.tmp STAGE2.BIN $(STAGE2_MAP)
	$(Q) mcopy -i $@.tmp $(STAGE2_MAP) ::/STAGE2.MAP
	$(Q) $(SECTOR_MAPPER) 2 $@.tmp STAGE2.MAP
	$(Q) mcopy -i $@.tmp lboot.cfg ::/LBOOT.CFG
# Only update the target if the previous commands succeed
	$(Q) mv $@.tmp $@


$(BUILDDIR):
	$(Q) mkdir -p $@

$(SECTOR_MAPPER):
	$(Q) cd tools/sector_mapper; $(MAKE)

emu: $(FLOPPY)
	$(Q) qemu-system-i386 -fda $(FLOPPY) -serial stdio -machine pc -no-reboot

emu-dbg: $(FLOPPY)
	$(Q) qemu-system-i386 -fda $(FLOPPY) -serial stdio -machine pc -no-reboot -S -s

clean: stage1_clean stage2_clean
	$(Q) rm -f $(STAGE1) $(FLOPPY)

.PHONY: clean emu emu-dbg
