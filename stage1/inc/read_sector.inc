/* Read single sector
 *
 * Parameters:
 *   %ax: Block address
 *   %bx: Destination
 */
read_sector:
    pusha
.if IS_FLOPPY
    /* tmp      =  LBA / SectorsPerTrack
     * Sector   = (LBA % SectorsPerTrack) + 1
     * Cylinder =  tmp / NumberOfHeads
     * Head     =  tmp % NumberOfHeads */
  .block_to_chs:
    pushw %bx                     /* Save destination address */
    pushw %dx                     /* Save drive number */

    xorw %dx, %dx
    movw (fat.sectors_per_track), %bx
    divw %bx                      /* AX = tmp; DX = Sector - 1 */
    movb %dl, %cl                 /* CL = Sector - 1 */
    incb %cl                      /* CL = Sector */

    xorw %dx, %dx
    movw (fat.head_count), %bx
    divw %bx                      /* AX = Cylinder; DX = Head */
    movb %al, %ch                 /* CH = Cylinder */
    movb %dl, %dh                 /* DH = Head */

    popw %bx
    movb %bl, %dl                 /* DL = Drive Number */
    popw %bx                      /* BX = Destination Address */

    movw  $3, %si                 /* Three attempts max */

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
    movw $sector_read_success_message, %si
    call msg_print
    popa
    ret
