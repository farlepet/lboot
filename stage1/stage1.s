.code16

/* Source for boot sector. Must fit within 512 bytes. */

/* 0x0500 - 0x7bff is usable */
stack_top = 0x2000

.global start
start:
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

    /* Set video mode and clear screen */
    movw $0x0003, %ax /* 80x25 */
    int  $0x10

    /* Print boot message */
    movw $boot_message, %si
    call msg_print

    /* @todo Actually boot */
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



boot_message:
    .asciz "Stage1"

/* MBR marker */
.skip (510 - (. - start))
.word 0xAA55
