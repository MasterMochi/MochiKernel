/******************************************************************************/
/*                                                                            */
/* src/kernel/kernel.lds                                                      */
/*                                                                 2021/11/27 */
/* Copyright (C) 2016-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
OUTPUT_FORMAT( elf32-i386 )
ENTRY( InitctrlInit )

PHDRS {
    load PT_LOAD;
}

MEMORY {
    KERNEL  (wxai): ORIGIN = 0x00100000,
                    LENGTH = 0x03F00000 - 0x00100000
    STACK   (wa)  : ORIGIN = 0x03F00000,
                    LENGTH = 0x04000000 - 0x03F00000
    PROCIMG (wa)  : ORIGIN = 0x04000000,
                    LENGTH = 0x05000000 - 0x04000000
}

SECTIONS {
    .text   : { *( .text   ) } > KERNEL : load
    .rodata : { *( .rodata ) } > KERNEL : load
    .data   : { *( .data   ) } > KERNEL : load
    .bss    : { *( .bss    ) } > KERNEL : load

    __end_point = ORIGIN( PROCIMG ) + LENGTH( PROCIMG );
}


/******************************************************************************/
