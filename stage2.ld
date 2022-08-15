ENTRY(start)

SECTIONS {
    . = 0x7e00;

    __lboot_begin = .;

    .text : {
        __lboot_text_begin = .;
        *(.entrypoint) /* Entrypoint needs to be first, since we jump to the beginning of the binary */
        *(.text.*)
        __lboot_text_end = .;
    }

    .rodata : {
        __lboot_rodata_begin = .;
        *(.rodata.*)
        __lboot_rodata_end = .;
    }

    .data : {
        __lboot_data_begin = .;
        *(.data.*)
        __lboot_data_end = .;
    }

    .bss : {
        __lboot_bss_begin = .;
        *(COMMON)
        *(.bss.*)
        __lboot_bss_end = .;
    }

    __lboot_end = .;
}
