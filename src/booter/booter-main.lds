/******************************************************************************/
/*                                                                            */
/* src/booter/booter-main.lds                                                 */
/*                                                                 2021/11/27 */
/* Copyright (C) 2016-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
OUTPUT_FORMAT( binary )
ENTRY( InitctrlInit16 )

SECTIONS {
    gBiosE820EntryNum = 0x0500;
    gBiosE820         = 0x0504;
    gMemMapEntryNum   = 0x0CD4;
    gMemMap           = 0x0CD8;

    . = 0x7C00;
    .text   : { * ( .text   ) }
    .rodata : { * ( .rodata ) }
    .data   : { * ( .data   ) }
    .bss    : { * ( .bss    ) }
}


/******************************************************************************/
