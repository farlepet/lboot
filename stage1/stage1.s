.code16

/* Source for boot sector. Must fit within 512 bytes. */

stack_top = 0x1000 /* Top of temporary stack */

IS_FLOPPY = 1

/* Need to reserve some bytes for the FAT12 data */
boot_sector_start:
    jmp start
    nop

/* FAT header, bytes 3-29 */
fat.name:                .ascii "lboot   " /* 0x03 */
/* BIOS parameter block */
fat.bytes_per_sector:    .skip 2 /* 0x0b */
fat.sectors_per_cluster: .skip 1 /* 0x0d */
fat.reserved_sectors:    .skip 2 /* 0x0e */
fat.fat_copies:          .skip 1 /* 0x10 */
fat.root_dir_entries:    .skip 2 /* 0x11 */
fat.total_sectors:       .skip 2 /* 0x13 */
fat.media_desc_type:     .skip 1 /* 0x15 */
fat.sectors_per_fat:     .skip 2 /* 0x16 */
fat.sectors_per_track:   .skip 2 /* 0x18 */
fat.head_count:          .skip 2 /* 0x1a */
fat.hidden_sectors:      .skip 2 /* 0x1c */

/* @note The above only accounts for FAT12. FAT16/32 take up more space here. */

/* Bootloader-specific header data. Data here will eventually be populated by the build tool. */
bootldr.stage2_addr:         .word 0x7e00 /* Address to which to load stage 2 loader */
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
.if !IS_FLOPPY
    cmp  $0, %dl
    jne  .store_dl
    movb $0x80, %dl
.endif
  .store_dl:
    movb %dl, boot_drive

    /* Set video mode and clear screen */
    movw $0x0003, %ax /* 80x25 */
    int  $0x10

    /* Print boot message */
    movw $boot_message, %si
    call msg_print

  .reset_drive:
    movb $0x00,      %ah /* Reset drive */
    movb boot_drive, %dl /* Boot drive */
    int $0x13

read_stage2:
    /* @todo This assumes the sectors are contiguous. This might not always be the
     * case, either due to bad blocks, or due to fragmentation. */
    movw (bootldr.stage2_addr),         %bx
    movw (bootldr.stage2_sector_start), %ax

    call read_sector

    /* @todo actually read and execute stage2 */
    jmp .



/* Read single sector
 *
 * Parameters:
 *   %ax: Block address
 *   %bx: Destination
 */
read_sector:
.if IS_FLOPPY
  .block_to_chs:
    /* @note This is likely not a very efficient method of performing this calculation */
    pushw %bx                     /* Save destination address */
    pushw %dx                     /* Save drive number */
    movw  (fat.total_sectors), %bx
    divl  %ebx                    /* EAX = Head number; EDX = Sector offset within head */
    movb  %al, %cl                /* CL  = Head number (temporary) */
    xchgw %dx, %ax                /* AX  = Sector within head; DX will be trashed */
    movw  (fat.sectors_per_track), %bx
    divl  %ebx                    /* EAX = Track number; EDX = Sector offset within track */
    movb  %al, %ch                /* CH  = Track number */
    xchgb %cl, %dh                /* CL  = Sector number; DH = Head number */
    incb  %cl                     /* Adjust CL to be 1-indexed */
    popw  %bx                     /* Restore drive number */
    movb  %bl, %dl                /* DL  = Drive number */
    popw  %bx                     /* Restore destination address */

    movw  $03, %si                /* Three attempts max */

  .attempt_read:
    dec %si
    jo  disk_error /* No attempts remaining */

    /* INT 13h, AH=02h 
     *
     * Parameters (for floppy):
     *   AL    = Sector count
     *   CH    = Low 8 bits of cylinder/track number
     *   CL    = Sector number
     *   DH    = Head number
     *   DL    = Drive number
     *   ES:BX = Data buffer
     */
    mov $0x0201, %ax
    int $0x13

    jc .attempt_read /* Retry on error */
.else
    pushl $0   /* LBA */
    pushl %eax /* ^   */
    pushl %ebx /* Buffer */
    pushw $1   /* Blocks to read */
    pushw $16  /* Packet size */
    movw  %sp, %si

    movb $3, %cl /* Setup attempts counter */

    /* @note This is currently not functional */
.read_retry:
    dec %cl        /* Only attempt 3 times */
    jo  disk_error

    movb $0x42,      %ah /* Extended read */
    movb boot_drive, %dl /* Boot drive */
    int  $0x13

    jc  .read_retry /* Carry flag is set on error */

    lea 16(%si), %sp
.endif
    ret




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

boot_message:
    .asciz "Stage1. "
disk_err_message:
    .asciz "Disk read error."

/* Boot marker */
.skip (510 - (. - boot_sector_start))
.word 0xAA55
