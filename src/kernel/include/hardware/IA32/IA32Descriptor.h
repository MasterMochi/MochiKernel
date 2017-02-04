/******************************************************************************/
/* src/kernel/include/hardware/IA32/IA32Descriptors.h                         */
/*                                                                 2016/12/16 */
/* Copyright (C) 2016 Mochi.                                                  */
/******************************************************************************/
#ifndef IA32_DESCRIPTOR_H
#define IA32_DESCRIPTOR_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
#include <stdint.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* ディスクリプタ存在フラグ */
#define IA32_DESCRIPTOR_P_NO             ( 0 )  /**< ディスクリプタ無し */
#define IA32_DESCRIPTOR_P_YES            ( 1 )  /**< ディスクリプタ有り */

/* ディスクリプタ特権レベル */
#define IA32_DESCRIPTOR_DPL_0            ( 0 )  /**< レベル0 */
#define IA32_DESCRIPTOR_DPL_1            ( 1 )  /**< レベル1 */
#define IA32_DESCRIPTOR_DPL_2            ( 2 )  /**< レベル2 */
#define IA32_DESCRIPTOR_DPL_3            ( 3 )  /**< レベル3 */

/* システムフラグ */
#define IA32_DESCRIPTOR_S_SYSTEM         ( 0 )  /**< システムディスクリプタ      */
#define IA32_DESCRIPTOR_S_CODEDATA       ( 1 )  /**< コード/データディスクリプタ */

/* システムディスクリプタタイプ */
#define IA32_DESCRIPTOR_TYPE_TSS16       ( 1 )  /**< 16ビットTSS            */
#define IA32_DESCRIPTOR_TYPE_LDT         ( 2 )  /**< LDT                    */
#define IA32_DESCRIPTOR_TYPE_TSS16_BUSY  ( 3 )  /**< 16ビットTSS(ビジー)    */
#define IA32_DESCRIPTOR_TYPE_GATE16_CALL ( 4 )  /**< 16ビットコールゲート   */
#define IA32_DESCRIPTOR_TYPE_GATE_TASK   ( 5 )  /**< タスクゲート           */
#define IA32_DESCRIPTOR_TYPE_GATE16_INT  ( 6 )  /**< 16ビット割込みゲート   */
#define IA32_DESCRIPTOR_TYPE_GATE16_TRAP ( 7 )  /**< 16ビットトラップゲート */
#define IA32_DESCRIPTOR_TYPE_TSS32       ( 9 )  /**< 32ビットTSS            */
#define IA32_DESCRIPTOR_TYPE_TSS32_BUSY  ( 11 ) /**< 32ビットTSS(ビジー)    */
#define IA32_DESCRIPTOR_TYPE_GATE32_CALL ( 12 ) /**< 32ビットコールゲート   */
#define IA32_DESCRIPTOR_TYPE_GATE32_INT  ( 14 ) /**< 32ビット割込みゲート   */
#define IA32_DESCRIPTOR_TYPE_GATE32_TRAP ( 15 ) /**< 32ビットトラップゲート */

/* コード/データディスクリプタタイプ */
#define IA32_DESCRIPTOR_TYPE_DATA_R      ( 0 )  /**< データ 読取専用                                 */
#define IA32_DESCRIPTOR_TYPE_DATA_RA     ( 1 )  /**< データ 読取専用, アクセス済                     */
#define IA32_DESCRIPTOR_TYPE_DATA_RW     ( 2 )  /**< データ 読書                                     */
#define IA32_DESCRIPTOR_TYPE_DATA_RWA    ( 3 )  /**< データ 読書, アクセス済                         */
#define IA32_DESCRIPTOR_TYPE_DATA_ER     ( 4 )  /**< データ エクスパンドダウン, 読取専用             */
#define IA32_DESCRIPTOR_TYPE_DATA_ERA    ( 5 )  /**< データ エクスパンドダウン, 読取専用, アクセス済 */
#define IA32_DESCRIPTOR_TYPE_DATA_ERW    ( 6 )  /**< データ エクスパンドダウン, 読書                 */
#define IA32_DESCRIPTOR_TYPE_DATA_ERWA   ( 7 )  /**< データ エクスパンドダウン, 読書, アクセス済     */
#define IA32_DESCRIPTOR_TYPE_CODE_X      ( 8 )  /**< コード 実行専用                                 */
#define IA32_DESCRIPTOR_TYPE_CODE_XA     ( 9 )  /**< コード 実行専用, アクセス済                     */
#define IA32_DESCRIPTOR_TYPE_CODE_XR     ( 10 ) /**< コード 実行読取                                 */
#define IA32_DESCRIPTOR_TYPE_CODE_XRA    ( 11 ) /**< コード 実行読取, アクセス済                     */
#define IA32_DESCRIPTOR_TYPE_CODE_CX     ( 12 ) /**< コード コンフォーミング, 実行専用               */
#define IA32_DESCRIPTOR_TYPE_CODE_CXA    ( 13 ) /**< コード コンフォーミング, 実行専用, アクセス済   */
#define IA32_DESCRIPTOR_TYPE_CODE_CXR    ( 14 ) /**< コード コンフォーミング, 実行読取               */
#define IA32_DESCRIPTOR_TYPE_CODE_CXRA   ( 15 ) /**< コード コンフォーミング, 実行読取, アクセス済   */

/* リミット粒度 */
#define IA32_DESCRIPTOR_G_BYTE           ( 0 )  /**< 1byte単位     */
#define IA32_DESCRIPTOR_G_4K             ( 1 )  /**< 4,095byte単位 */

/* デフォルトオペレーションサイズ */
#define IA32_DESCRIPTOR_DB_UNUSED        ( 0 )  /**< 未使用 */
#define IA32_DESCRIPTOR_DB_16            ( 0 )  /**< 16bit  */
#define IA32_DESCRIPTOR_DB_32            ( 1 )  /**< 32bit  */

/* システム利用可能フラグ */
#define IA32_DESCRIPTOR_AVL_OFF          ( 0 )  /**< 0 */
#define IA32_DESCRIPTOR_AVL_ON           ( 1 )  /**< 1 */

/** セグメントリミット（0-15）マクロ */
#define IA32_DESCRIPTOR_LIMIT_LOW( _LIMIT )   \
    ( ( uint16_t ) ( ( ( uint32_t ) ( _LIMIT   ) & 0x0000FFFF )       ) )
/** セグメントリミット（16-23）マクロ */
#define IA32_DESCRIPTOR_LIMIT_HIGH( _LIMIT )  \
    ( ( uint8_t  ) ( ( ( uint32_t ) ( _LIMIT   ) & 0x00FF0000 ) >> 16 ) )
/** セグメントベースアドレス（0-15）マクロ */
#define IA32_DESCRIPTOR_BASE_LOW( _PBASE )    \
    ( ( uint16_t ) ( ( ( uint32_t ) ( _PBASE   ) & 0x0000FFFF )       ) )
/** セグメントベースアドレス（16-23）マクロ */
#define IA32_DESCRIPTOR_BASE_MIDDLE( _PBASE ) \
    ( ( uint8_t  ) ( ( ( uint32_t ) ( _PBASE   ) & 0x00FF0000 ) >> 16 ) )
/** セグメントベースアドレス（24-31）マクロ */
#define IA32_DESCRIPTOR_BASE_HIGH( _PBASE )   \
    ( ( uint8_t  ) ( ( ( uint32_t ) ( _PBASE   ) & 0xFF000000 ) >> 24 ) )
/** オフセット（0-15）マクロ */
#define IA32_DESCRIPTOR_OFFSET_LOW( _POFFSET ) \
    ( ( uint16_t ) ( ( ( uint32_t ) ( _POFFSET ) & 0x0000FFFF )       ) )
/** オフセット（16-31）マクロ */
#define IA32_DESCRIPTOR_OFFSET_HIGH( _POFFSET ) \
    ( ( uint16_t ) ( ( ( uint32_t ) ( _POFFSET ) & 0xFFFF0000 ) >> 16 ) )

/* セグメントディスクリプタ */
typedef struct {
    uint16_t limit_low;         /* セグメントリミット(0-15)        */
    uint16_t base_low;          /* セグメントベースアドレス(0-15)  */
    uint8_t  base_middle;       /* セグメントベースアドレス(16-23) */
    uint8_t  attr_type: 4;      /* セグメントタイプ                */
    uint8_t  attr_s   : 1;      /* システムフラグ                  */
    uint8_t  attr_dpl : 2;      /* ディスクリプタ特権レベル        */
    uint8_t  attr_p   : 1;      /* ディスクリプタ存在フラグ        */
    uint8_t  limit_high   : 4;  /* セグメントリミット(16-19)       */
    uint8_t  attr_avl     : 1;  /* 未使用                          */
    uint8_t  attr_reserved: 1;  /* 予約(0)                         */
    uint8_t  attr_opSize  : 1;  /* デフォルトオペレーションサイズ  */
    uint8_t  attr_g       : 1;  /* リミット粒度                    */
    uint8_t  base_high;         /* セグメントベースアドレス(24-31) */
} __attribute__( ( aligned( 8 ) ) ) IA32DescriptorSeg_t;

/* TSSディスクリプタ */
typedef struct {
    uint16_t limit_low;         /* セグメントリミット(0-15)        */
    uint16_t base_low;          /* セグメントベースアドレス(0-15)  */
    uint8_t  base_middle;       /* セグメントベースアドレス(16-23) */
    uint8_t  attr_type: 4;      /* セグメントタイプ                */
    uint8_t  attr_s   : 1;      /* システムフラグ                  */
    uint8_t  attr_dpl : 2;      /* ディスクリプタ特権レベル        */
    uint8_t  attr_p   : 1;      /* ディスクリプタ存在フラグ        */
    uint8_t  limit_high   : 4;  /* セグメントリミット(16-19)       */
    uint8_t  attr_avl     : 1;  /* 未使用                          */
    uint8_t  attr_reserved: 2;  /* 予約(0)                         */
    uint8_t  attr_g       : 1;  /* リミット粒度                    */
    uint8_t  base_high;         /* セグメントベースアドレス(24-31) */
} __attribute__( ( aligned( 8 ) ) ) IA32DescriptorTSS_t;

/* ゲートディスクリプタ */
typedef struct {
    uint16_t offset_low;        /* オフセット(0-15)         */
    uint16_t selector;          /* セグメントセレクタ       */
    uint8_t  reserved: 5;       /* 予約(0)                  */
    uint8_t  count   : 3;       /* 引数カウント             */
    uint8_t  attr_type: 4;      /* ディスクリプタタイプ     */
    uint8_t  attr_s   : 1;      /* システムフラグ(0)        */
    uint8_t  attr_dpl : 2;      /* ディスクリプタ特権レベル */
    uint8_t  attr_p   : 1;      /* ディスクリプタ存在フラグ */
    uint16_t offset_high;       /* オフセット(16-31)        */
} __attribute__( ( aligned( 8 ) ) ) IA32DescriptorGate_t;

/* ディスクリプタ */
typedef union {
    IA32DescriptorSeg_t  seg;   /* セグメントディスクリプタ */
    IA32DescriptorTSS_t  tss;   /* TSSディスクリプタ        */
    IA32DescriptorGate_t gate;  /* ゲートディスクリプタ     */
} IA32Descriptor_t;

/* 疑似ディスクリプタ */
typedef struct {
    uint16_t         limit;     /* リミット       */
    IA32Descriptor_t *pBase;    /* ベースアドレス */
} __attribute__( ( packed ) ) IA32DescriptorPseudo_t;


/******************************************************************************/
#endif
