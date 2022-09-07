.extern interrupt_wrapper

.macro isr_wrapper num err
    .global isr_wrapper_\num
    .type   isr_wrapper_\num, @function
    isr_wrapper_\num:
    .if !\err
        push $0 # Errcode
    .endif
        pusha
        push $\num
        call interrupt_wrapper
        addl $4, %esp # Interrupt number
        popa
    .if !\err
        addl $4, %esp # Errcode
    .endif
        iret
    .size isr_wrapper_\num, (. - isr_wrapper_\num)
.endm

# Exceptions
isr_wrapper 0,  0
isr_wrapper 1,  0
isr_wrapper 2,  0
isr_wrapper 3,  0
isr_wrapper 4,  0
isr_wrapper 5,  0
isr_wrapper 6,  0
isr_wrapper 7,  0
isr_wrapper 8,  1
isr_wrapper 9,  0
isr_wrapper 10, 1
isr_wrapper 11, 1
isr_wrapper 12, 1
isr_wrapper 13, 1
isr_wrapper 14, 1
isr_wrapper 15, 0
isr_wrapper 16, 0
isr_wrapper 17, 1
isr_wrapper 18, 0
isr_wrapper 19, 0
isr_wrapper 20, 0
isr_wrapper 21, 0
isr_wrapper 22, 0
isr_wrapper 23, 0
isr_wrapper 24, 0
isr_wrapper 25, 0
isr_wrapper 26, 0
isr_wrapper 27, 0
isr_wrapper 28, 0
isr_wrapper 29, 0
isr_wrapper 30, 1
isr_wrapper 31, 0

# IRQs
isr_wrapper 32, 0
isr_wrapper 33, 0
isr_wrapper 34, 0
isr_wrapper 35, 0
isr_wrapper 36, 0
isr_wrapper 37, 0
isr_wrapper 38, 0
isr_wrapper 39, 0
isr_wrapper 40, 0
isr_wrapper 41, 0
isr_wrapper 42, 0
isr_wrapper 43, 0
isr_wrapper 44, 0
isr_wrapper 45, 0
isr_wrapper 46, 0
isr_wrapper 47, 0

