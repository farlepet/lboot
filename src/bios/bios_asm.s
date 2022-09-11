.code32

.extern bios_idt

/* int bios_call(bios_call_t *) */
.global bios_call_asm
.type   bios_call_asm, @function
bios_call_asm:
    pushal

    /* Save BIOS call parameter pointer */
    movl 36(%esp), %eax
    movl %eax,     (_bios_call_ptr)

    /* Save current IDTR and GDTR */
    sidt (_saved_idtr)
    sgdt (_saved_gdtr)


    /* 
     * Enter real mode
     */
    /* 1. Disable interrupts */
    /* Interrupt disable is done in bios_call() */
    /* Disable NMI */
    inb  $0x70, %al
    orb  $0x80, %al
    outb %al,  $0x80
    inb  $0x71, %al

    /* 2. Disable paging (N/A) */
    /* 3. Transfer control to segment with limit of 0xFFFF */
    ljmp $0x18, $1f
.code16
1:

    /* 4. Set data segments to one with limit of 0xFFFF */
    movw $0x20, %ax
    movw %ax,   %ds
    movw %ax,   %ss
    movw %ax,   %es
    movw %ax,   %fs
    movw %ax,   %gs

    /* 5. Load real mode IDT */
    lidt (_realmode_idtr)

    /* 6. Clear PE flag in CR0 */
    movl %cr0,        %eax
    andl $0xFFFFFFFE, %eax
    movl %eax,        %cr0

    /* 7. Far jump into real-mode code */
    ljmp $0x00, $1f

1:
    /* 8. Setup data segment registers */
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %ss
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    /* 9. Re-enable interrupts */
    sti
    /* Enable NMI */
    inb  $0x70, %al
    andb $0x7F, %al
    outb %al,  $0x80
    inb  $0x71, %al


    /*
     * Call BIOS function
     */
    
    movw (_bios_call_ptr), %di
    /* Load interrupt number */
    movb (%di), %al
    movb %al,   (_int_id)

    /* Load parameters */
    movl  4(%di), %eax
    movl  8(%di), %ebx
    movl 12(%di), %ecx
    movl 16(%di), %edx
    movl 20(%di), %esi
    movl 24(%di), %edi
    /* @note Not loading EFLAGS, not sure any interrupts use any of those bits as input */

    /* @note INT only accepts immediates, so we need to modify the code */
    .byte 0xCD /* INT */
_int_id:
    .byte 0x00 /* imm8 */

    /* Save results */
    pushw %bp
    movw  (_bios_call_ptr), %bp
    movl  %eax,  4(%bp)
    movl  %ebx,  8(%bp)
    movl  %ecx, 12(%bp)
    movl  %edx, 16(%bp)
    movl  %esi, 20(%bp)
    movl  %edi, 24(%bp)
    /* Save EFLAGS */
    pushfl
    popl   %eax
    movl   %eax, 28(%bp)

    popw %bp


    /*
     * Enter protected mode
     */

    /* 1. Disable interrupts */
    cli

    /* 2. Setup GDT */
    lgdt (_saved_gdtr)

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
    /* @todo This is currently causing a GPF - Not sure it's strictly required
     * though since we set it up earlier. */
    /*movw $0x28, %ax
    ltr %ax*/

    /* 9. Reload segment registers DS, SS, ES, FS, GS */
    movw $0x10, %ax
    movw %ax,   %ds
    movw %ax,   %ss
    movw %ax,   %es
    movw %ax,   %fs
    movw %ax,   %gs

    /* 10. Setup IDT */
    lidt (_saved_idtr)

    /* 11. Re-enable interrupts */
    /* Interrupt enable is done in bios_call() */
    /* Enable NMI */
    inb  $0x70, %al
    andb $0x7F, %al
    outb %al,  $0x80
    inb  $0x71, %al

    popal
    ret
.size bios_call_asm, (. - bios_call_asm)

_bios_call_ptr:
    .skip 4

_saved_idtr:
    .skip 6

_saved_gdtr:
    .skip 6

_realmode_idtr:
    .word 0x03FF
    .long 0x00000000

