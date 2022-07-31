.code16

/* Source for boot sector. Must fit within 512 bytes. */

stack_top       = 0x2000 /* Top of temporary stack */

/* Need to reserve some bytes for the FAT12 data */
boot_sector_start:
    jmp start
    nop

/* FAT header, bytes 3-29 */
fat.name:                .ascii "lboot   "
/* BIOS parameter block */
fat.bytes_per_sector:    .skip 2
fat.sectors_per_cluster: .skip 1
fat.reserved_sectors:    .skip 2
fat.fat_copies:          .skip 1
fat.root_dir_entries:    .skip 2
fat.total_sectors:       .skip 2
fat.media_desc_type:     .skip 1
fat.sectors_per_fat:     .skip 2
fat.sectors_per_track:   .skip 2
fat.head_count:          .skip 2
fat.hidden_sectors:      .skip 2 /* Perhaps where we can store the stage2 data */

/* @note The above only accounts for FAT12. FAT16/32 take up more space here. */

/* Bootloader-specific header data */
bootldr.stage2_sector_addr:  .word 0x7e00 /* Address to which to load stage 2 loader */
bootldr.stage2_sector_start: .word 0x0000 /* First sector of the stage 2 loader */
bootldr.stage2_sector_count: .word 0x0001 /* Number of sectors occupied by the stage 2 loader */


.global start
start:
    /* Enforce that we are using 0x0000:0xXXXX addressing */
    ljmp $0, $.seg0

.seg0:
    cli
    /* Setup segment registers */
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    /* Setup stack */
    movw $stack_top, %sp
    movw %sp,        %bp
    sti

    /* Save boot drive */
    movb %dl, boot_drive

    /* Set video mode and clear screen */
    movw $0x0003, %ax /* 80x25 */
    int  $0x10

    /* Print boot message */
    movw $boot_message, %si
    call msg_print

reset_drive:
    movb $0x00,      %ah /* Reset drive */
    movb boot_drive, %dl /* Boot drive */
    int $0x13

read_stage2:
    /* @todo This assumes the sectors are contiguous. This might not always be the
     * case, either due to bad blocks, or due to fragmentation. */
    /* @note This is not an effecient use of space, could have the build tool
     * populate these directly in the DAP */
    movw bootldr.stage2_sector_addr,  %ax
    movw %ax,                         dap.destination
    movw bootldr.stage2_sector_start, %ax
    movw %ax,                         dap.block_number
    movw bootldr.stage2_sector_count, %ax
    movw %ax,                         dap.blocks

    /* @note This is currently not functional */

    movb $0x42,      %ah /* Extended read */
    movb boot_drive, %dl /* Boot drive */
    movw $dap,       %si /* Pointer to DAP */
    int $0x13

    jc disk_error /* Carry flag is set on error */

    /* @todo Read stage2 */
    jmp .

disk_error:
    movw $disk_err_message, %si
    call msg_print
    jmp .


/* Print message.
 *
 * Parameters:
 *   %si: Pointer to string to print.
 */
msg_print:
    movb $0x0E, %ah /* Write character */
    xorw %bx,   %bx /* Page, no color for text mode */
.loop:
    lodsb           /* Load character and increment %si */
    cmpb $0, %al    /* Exit if character is null */
    je   .end
    int  $0x10      /* Print character */
    jmp  .loop
.end:
    ret

boot_drive: .skip 1 /* Drive the BIOS tells us we booted from */

/* Disk Address Packet */
dap:
dap.size:         .byte 0x10
dap.reserved:     .byte 0x00
dap.blocks:       .word 0x0000                         /* To be filled in later */
dap.destination:  .word 0x0000, 0x0000                 /* To be filled in later */
dap.block_number: .word 0x0000, 0x0000, 0x0000, 0x0000 /* To be filled in later */

boot_message:
    .asciz "Stage1. "
disk_err_message:
    .asciz "Disk read error."

/* Boot marker */
.skip (510 - (. - boot_sector_start))
.word 0xAA55
