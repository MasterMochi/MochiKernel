/******************************************************************************/
/*                                                                            */
/* src/kernel/kernel.lds                                                      */
/*                                                                 2019/08/13 */
/* Copyright (C) 2016-2019 Mochi.                                             */
/*                                                                            */
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
    __end_point = .;
}


/******************************************************************************/
