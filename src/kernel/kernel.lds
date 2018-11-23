/******************************************************************************/
/* src/kernel/kernel.lds                                                      */
/*                                                                 2018/11/23 */
/* Copyright (C) 2016-2018 Mochi.                                             */
/******************************************************************************/
OUTPUT_FORMAT( elf32-i386 )
ENTRY( InitCtrlInit )

PHDRS {
    load PT_LOAD;
}

MEMORY {
    kernel (wxai): ORIGIN = 0x00100000,
                   LENGTH = 0x03F00000 - 0x00100000
}

SECTIONS {
    .text   : { *( .text   ) } > kernel : load
    .rodata : { *( .rodata ) } > kernel : load
    .data   : { *( .data   ) } > kernel : load
    .bss    : { *( .bss    ) } > kernel : load
}


/******************************************************************************/
