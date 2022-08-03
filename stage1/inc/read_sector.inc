/* Read single sector
 *
 * Parameters:
 *   %ax: Block address
 *   %bx: Destination
 */
read_sector:
    pusha
.if IS_FLOPPY
    /* Head  = Sector / TotalSectors
     * Track = (Sector % TotalSectors) / SectorsPerTrack
     * Track = (Sector % TotalSectors) % SectorsPerTrack
     */
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
    movb  %cl, %dh                /* DH = Head number */
    movb  %dl, %cl                /* CL  = Sector number */
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

    jc .attempt_read /* Retry on error @todo reset controller */
.else
    /* @note UNTESTED */
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
    popa
    ret
