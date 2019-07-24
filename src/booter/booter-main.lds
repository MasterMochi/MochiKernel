/******************************************************************************/
/*                                                                            */
/* src/booter/booter-main.lds                                                 */
/*                                                                 2019/07/23 */
/* Copyright (C) 2016-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
OUTPUT_FORMAT( binary )
ENTRY( InitCtrlInit16 )

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
