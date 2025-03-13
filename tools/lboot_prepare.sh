#!/bin/sh
# LBoot disk/image preparation script
# Usage:
#   lboot_prepare.sh <floppy disk device or image file>
#
# Presently must be used within the root of the repository. Assumes required
# tools are already built.

if [[ $# -ne 1 ]]; then
    echo "USAGE:"
    echo "  lboot_prepare.sh <floppy disk device or image file>"
    exit 2
fi

# Auto-fail
set -e

FLOPPY=$1

LBOOT_DIR=.
STAGE1=$LBOOT_DIR/build/stage1/stage1.bin
STAGE2=$LBOOT_DIR/build/stage2/stage2.bin
STAGE2_MAP=$LBOOT_DIR/build/stage2.map

SECTOR_MAPPER=$LBOOT_DIR/tools/sector_mapper/sector_mapper

if   [[ ! -e $FLOPPY ]]; then
    echo "Floppy/image '$FLOPPY' does not exist!"
    exit 2
elif [[ ! -f $STAGE1 ]]; then
    echo "Stage 1 binary '$STAGE1' does not exist!"
    exit 1
elif [[ ! -f $STAGE2 ]]; then
    echo "Stage 2 binary '$STAGE2' does not exist!"
    exit 1
elif [[ ! -f $SECTOR_MAPPER ]]; then
    echo "sector_mapper tool '$SECTOR_MAPPER' does not exist!"
    exit 1
fi

# Stage 1 / bootsector
echo "Writing bootsector"
dd if=$STAGE1 of=$FLOPPY conv=notrunc iflag=count_bytes,skip_bytes oflag=seek_bytes count=11
dd if=$STAGE1 of=$FLOPPY conv=notrunc iflag=count_bytes,skip_bytes oflag=seek_bytes skip=90 seek=90

# Stage 2 + Map
echo "Creating LBOOT directory"
set +e # mmd returns error if the directory exists
mmd   -D oO -i $FLOPPY             ::/LBOOT
set -e

echo "Copying stage 2 bootloader"
mcopy -D oO -i $FLOPPY $STAGE2     ::/LBOOT/STAGE2.BIN
echo "Generating stage 2 sector map"
$SECTOR_MAPPER 1 $FLOPPY "LBOOT/STAGE2.BIN" $STAGE2_MAP
echo "Copying stage 2 sector map"
mcopy -D oO -i $FLOPPY $STAGE2_MAP ::/LBOOT/STAGE2.MAP
echo "Setting pointer to stage 2 sector map"
$SECTOR_MAPPER 2 $FLOPPY "LBOOT/STAGE2.MAP"

