.code16

/* Entrypoint for stage 2 */

.section .entrypoint

.extern cstart
.global start
.type   start, @function
start:
    /* Print boot message */
    movw $boot_message, %si
    call msg_print


    /* Switching to protected mode, following recommendations laid out in
     * Intel's Software Developer Manual, Vol. 3A, section 9.9.1 - Switching to
     * Protected Mode. */

    /* 1. Disable interrupts, and NMI */
    cli
    /* @todo Disable NMI */

    /* 2. Setup GDT */
    lgdt (gdtr)

    /* 3. Enable PE in CR0 */
    movl %cr0, %eax
    orl  $1,   %eax
    movl %eax, %cr0


    /* 4. Far jump */
    ljmpl $0x08, $1f

.code32
1:
    /* 5-7. N/A */
    /* 8. Set up task state segment */
    movw $0x28, %ax
    ltr %ax

    /* 9. Reload segment registers DS, SS, ES, FS, GS */
    movw $0x10, %ax
    movw %ax,   %ds
    movw %ax,   %ss
    movw %ax,   %es
    movw %ax,   %fs
    movw %ax,   %gs

    /* 10. Setup IDT */
    lidt (idtr)

    /* 11. Re-enable interrupts and NMI */
    /* @todo Properly setup IDT, then we can re-enable interrupts */
    /* @todo Re-enable NMI */

    /* Enable A20 line @todo Attempt multiple methods, perhaps in C */
    inb   $0x92, %al
    testb $0x02, %al
    jnz   1f
    orb   $0x02, %al
    outb  %al, $0x92
1:

    /* Call into C code */
    call cstart

    jmp .
.size start, (. - start)


/* Print message.
 *
 * Parameters:
 *   %si: Pointer to string to print.
 */
.type msg_print, @function
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
.size msg_print, (. - msg_print)


boot_message:
    .asciz "\r\nStage2.\r\n"

gdtr:
    .word ((gdt_end - gdt) - 1)  /* Limit */
    .long gdt                    /* Base */

idtr:
    .word ((idt_end - idt) - 1) /* Limit */
    .long idt                   /* Base */

.align 8
/* @note Flat memory model with no protection */
gdt:
    /* 0x00: Null descriptor */
    .quad 0x00000000
    /* 0x08: 32-bit Code segment */
    .long 0x0000FFFF
    .long 0x00CF9A00
    /* 0x10: 32-bit Data segment */
    .long 0x0000FFFF
    .long 0x00CF9200
    /* 0x18: 16-bit Code segment */
    .long 0x0000FFFF
    .long 0x000F9A00
    /* 0x20: 16-bit Data segment */
    .long 0x0000FFFF
    .long 0x000F9200
    /* 0x28: Task segment */
    .word ((tss_end - tss) - 1)
    .word tss /* @note Since we are in the lower 16-bits of memory, we can just use the address here */
    .long 0x00408900
gdt_end:

idt:
    /* @todo */
    .quad 0
idt_end:

/* @note Within the bootloader, we will never be in any ring other than 0 */
tss:
    .long 0x00000000 /* Prev task link */
    /* @note Since we are never leaving ring 0, I don't _think_ we need to set ESP0 */
    .long 0x00000000 /* ESP0 */
    .long 0x00000010 /* SS0 */
    .long 0x00000000 /* ESP1 */
    .long 0x00000010 /* SS1 */
    .long 0x00000000 /* ESP2 */
    .long 0x00000010 /* SS2 */
    .long 0x00000000 /* CR3 */
    .long 0x00000000 /* EIP */
    .long 0x00000000 /* EFLAGS */
    .long 0x00000000 /* EAX */
    .long 0x00000000 /* ECX */
    .long 0x00000000 /* EDX */
    .long 0x00000000 /* EBX */
    .long 0x00000000 /* ESP */
    .long 0x00000000 /* EBP */
    .long 0x00000000 /* ESI */
    .long 0x00000000 /* EDI */
    .long 0x00000010 /* ES */
    .long 0x0000000C /* CS */
    .long 0x00000010 /* SS */
    .long 0x00000010 /* DS */
    .long 0x00000010 /* FS */
    .long 0x00000010 /* GS */
    .long 0x00000000 /* LDT selector */
    .word 0x0000 /* Debug trap enable */
    .word ((tss_end - tss) - 1) /* IO Map Base Address */
tss_end:

