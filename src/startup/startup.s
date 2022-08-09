.code16

/* Entrypoint for stage 2 */

.global start
start:
    /* Print boot message */
    movw $boot_message, %si
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
boot_message:
    .asciz "Stage2.\r\n"
