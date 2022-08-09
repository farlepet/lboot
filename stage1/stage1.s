.code16

/* Source for boot sector. Must fit within 512 bytes. */

stack_top = 0x1000     /* Top of temporary stack */

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

/* @note The above only accounts for minimal FAT12. FAT16/32 take up more space here. */
.skip (90 - (. - boot_sector_start))


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
    movb $0x80, %dl /* BIOS may have passed us a bogus drive */
.endif
  .store_dl:
    movb %dl, boot_drive

    /* Set video mode and clear screen */
    movw $0x0003, %ax /* 80x25 */
    int  $0x10

    /* Print boot message */
    movw $boot_message, %si
    call msg_print

    /* Save address to jump to before it gets modified. */
    movw (bootldr.stage2_addr), %bx
    movw %bx, (stage2_addr_bkp)

  .reset_drive:
    movb $0x00,      %ah /* Reset drive */
    movb boot_drive, %dl /* Boot drive */
    int $0x13

    /* Read stage 2 loader into memory */
    call read_sector_map

    /* Jmp into stage2 */
    movw (stage2_addr_bkp), %ax
    jmp  *%ax



.include "inc/read_sector.inc"
.include "inc/read_sector_map.inc"


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
    pusha
    movb $0x0E, %ah /* Write character */
    xorw %bx,   %bx /* Page, no color for text mode */
.loop:
    lodsb           /* Load character and increment %si */
    cmpb $0, %al    /* Exit if character is null */
    je   .end
    int  $0x10      /* Print character */
    jmp  .loop
.end:
    popa
    ret

boot_drive: .skip 1 /* Drive the BIOS tells us we booted from */

stage2_addr_bkp:     .skip 2

boot_message:
    .asciz "Stage1.\r\n"
disk_err_message:
    .asciz "Disk read error."
sector_read_success_message:
    .asciz "."

/* Place the following at the end of the boot sector */
.skip (506 - (. - boot_sector_start))

/* Bootloader-specific header data. Data here will eventually be populated by the build tool. */
bootldr.stage2_map_sector:   .word 0x0000 /* Sector containing stage 2 sector map, 0 indexed */
bootldr.stage2_addr:         .word 0x7e00 /* Address to which to load stage 2 loader */

/* Boot sector magic number */
.word 0xAA55
