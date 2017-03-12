/******************************************************************************/
/* src/kernel/include/hardware/IA32/IA32.h                                    */
/*                                                                 2017/01/15 */
/* Copyright (C) 2016-2017 Mochi.                                             */
/******************************************************************************/
#ifndef IA32_H
#define IA32_H
/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* テーブルインジケータ定義 */
#define IA32_TI_GDT ( 0 )   /**< テーブルインジケータ：GDT */
#define IA32_TI_LDT ( 1 )   /**< テーブルインジケータ：LDT */

/* RPL定義 */
#define IA32_RPL_0  ( 0 )   /**< RPL0 */
#define IA32_RPL_1  ( 1 )   /**< RPL1 */
#define IA32_RPL_2  ( 2 )   /**< RPL1 */
#define IA32_RPL_3  ( 3 )   /**< RPL1 */

/** セグメントセレクタマクロ */
#define IA32_SEGMENT_SELECTOR( _INDEX, _TI, _RPL ) \
    ( ( uint16_t ) ( ( ( _INDEX ) << 3 ) | ( ( _TI ) << 2 ) | ( _RPL ) ) )

/* EFLAGSレジスタ */
#define IA32_EFLAGS_ID             ( 0x00200000 )
#define IA32_EFLAGS_VIP            ( 0x00100000 )
#define IA32_EFLAGS_VIF            ( 0x000C0000 )
#define IA32_EFLAGS_AC             ( 0x00040000 )
#define IA32_EFLAGS_VM             ( 0x00020000 )
#define IA32_EFLAGS_RF             ( 0x00010000 )
#define IA32_EFLAGS_NT             ( 0x00004000 )
#define IA32_EFLAGS_IOPL( _LEVEL ) ( 0x00003000 & ( ( _LEVEL ) << 12 ) )
#define IA32_EFLAGS_OF             ( 0x00000C00 )
#define IA32_EFLAGS_DF             ( 0x00000400 )
#define IA32_EFLAGS_IF             ( 0x00000200 )
#define IA32_EFLAGS_TF             ( 0x00000100 )
#define IA32_EFLAGS_SF             ( 0x000000C0 )
#define IA32_EFLAGS_ZF             ( 0x00000040 )
#define IA32_EFLAGS_AF             ( 0x00000010 )
#define IA32_EFLAGS_PF             ( 0x00000004 )
#define IA32_EFLAGS_RESERVED       ( 0x00000002 )
#define IA32_EFLAGS_CF             ( 0x00000001 )


/******************************************************************************/
#endif
