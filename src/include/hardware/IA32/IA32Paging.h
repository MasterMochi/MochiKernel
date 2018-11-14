/******************************************************************************/
/* src/kernel/include/hardware/IA32/IA32Paging.h                              */
/*                                                                 2018/09/22 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
#ifndef IA32_PAGING_H
#define IA32_PAGING_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
#include <stdint.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** PDBR設定マクロ */
#define IA32_PAGING_SET_PDBR( _PDBR, _PBASE, _PCD, _PWT )   \
    {                                                       \
        *( ( uint32_t * ) &( _PDBR ) ) =                    \
            ( ( uint32_t ) ( _PBASE ) ) & 0xFFFFF000;       \
        ( _PDBR ).attr_pcd             = ( _PCD );          \
        ( _PDBR ).attr_pwt             = ( _PWT );          \
    }

/** ページディレクトリエントリインデックス取得マクロ */
#define IA32_PAGING_GET_PDE_IDX( _PADDR )               \
    ( ( ( ( uint32_t ) ( _PADDR ) ) >> 22 ) & 0x3FF )
/** ページテーブルエントリインデックス取得マクロ */
#define IA32_PAGING_GET_PTE_IDX( _PADDR )               \
    ( ( ( ( uint32_t ) ( _PADDR ) ) >> 12 ) & 0x3FF )

/** ページテーブル・ページベースアドレス設定マクロ */
#define IA32_PAGING_SET_BASE( _PENTRY, _PBASE )                 \
    ( *( ( uint32_t * ) ( _PENTRY ) ) =                         \
        (  ( ( uint32_t   ) ( _PBASE  ) ) & 0xFFFFF000 ) |      \
        ( *( ( uint32_t * ) ( _PENTRY ) ) & 0x00000FFF )   )
/** ページテーブル・ページベースアドレス取得マクロ */
#define IA32_PAGING_GET_BASE( _PENTRY )                             \
    ( ( void * ) ( *( ( uint32_t * ) ( _PENTRY ) ) & 0xFFFFF000 ) )

/* ページテーブル・ページ存在フラグ */
#define IA32_PAGING_P_NO        ( 0 )       /** ページテーブル・ページ無し */
#define IA32_PAGING_P_YES       ( 1 )       /** ページテーブル・ページ有り */

/* 読込/書込フラグ */
#define IA32_PAGING_RW_R        ( 0 )       /** 読込専用ページ    */
#define IA32_PAGING_RW_RW       ( 1 )       /** 読込/書込可ページ */

/* ユーザ/スーパバイザフラグ */
#define IA32_PAGING_US_SV       ( 0 )       /** スーパバイザ特権レベル */
#define IA32_PAGING_US_USER     ( 1 )       /** ユーザ特権レベル       */

/* ページレベルライトスルーフラグ */
#define IA32_PAGING_PWT_WB      ( 0 )       /** ライトバックキャッシング */
#define IA32_PAGING_PWT_WT      ( 1 )       /** ライトスルーキャッシング */

/* ページレベルキャッシュディスエーブルフラグ */
#define IA32_PAGING_PCD_ENABLE  ( 0 )       /** キャッシング有効 */
#define IA32_PAGING_PCD_DISABLE ( 1 )       /** キャッシング無効 */

/* アクセス済みフラグ */
#define IA32_PAGING_A_NO        ( 0 )       /** 未アクセス   */
#define IA32_PAGING_A_YES       ( 1 )       /** アクセス済み */

/* ダーティフラグ */
#define IA32_PAGING_D_NO        ( 0 )       /** 未書込み   */
#define IA32_PAGING_D_YES       ( 1 )       /** 書込み済み */

/* ページサイズフラグ */
#define IA32_PAGING_PS_4K       ( 0 )       /** 4KiBページサイズ           */
#define IA32_PAGING_PS_4M2M     ( 1 )       /** 4MiBまたは2MiBページサイズ */

/* グローバルフラグ */
#define IA32_PAGING_G_NO        ( 0 )       /** 非グローバルページ */
#define IA32_PAGING_G_YES       ( 1 )       /** グローバルページ   */

/* エントリ数 */
#define IA32_PAGING_PDE_NUM     ( 1024 )    /** ページディレクトリエントリ数 */
#define IA32_PAGING_PTE_NUM     ( 1024 )    /** ページテーブルエントリ数     */

/** ページサイズ */
#define IA32_PAGING_PAGE_SIZE   ( 4096 )

/** PDBR */
typedef struct {
    uint32_t reserved2:3;   /**< 予約済み                                   */
    uint32_t attr_pwt :1;   /**< ページレベルライトスルーフラグ             */
    uint32_t attr_pcd :1;   /**< ページレベルキャッシュディスエーブルフラグ */
    uint32_t reserved1:7;   /**< 予約済み                                   */
    uint32_t base     :20;  /**< ページディレクトリベースアドレス           */
} IA32PagingPDBR_t;

/** ページディレクトリエントリ */
typedef struct {
    uint32_t attr_p  :1;    /**< 存在フラグ                                 */
    uint32_t attr_rw :1;    /**< 読込/書込フラグ                            */
    uint32_t attr_us :1;    /**< ユーザ/スーパバイザフラグ                  */
    uint32_t attr_pwt:1;    /**< ページレベルライトスルーフラグ             */
    uint32_t attr_pcd:1;    /**< ページレベルキャッシュディスエーブルフラグ */
    uint32_t attr_a  :1;    /**< アクセス済みフラグ                         */
    uint32_t reserved:1;    /**< 予約済                                     */
    uint32_t attr_ps :1;    /**< ページサイズフラグ                         */
    uint32_t attr_g  :1;    /**< グローバルページ                           */
    uint32_t attr_avl:3;    /**< 未使用                                     */
    uint32_t base    :20;   /**< ページテーブルベースアドレス               */
} IA32PagingPDE_t;

/** ページディレクトリ */
typedef struct {
    IA32PagingPDE_t entry[ IA32_PAGING_PDE_NUM ];   /**< エントリ */
} IA32PagingDir_t;
/* [memo] 4096byteアライメントで配置する必要がある。 */

/** ページテーブルエントリ */
typedef struct {
    uint32_t attr_p  :1;    /**< 存在フラグ                                 */
    uint32_t attr_rw :1;    /**< 読込/書込フラグ                            */
    uint32_t attr_us :1;    /**< ユーザ/スーパバイザフラグ                  */
    uint32_t attr_pwt:1;    /**< ページレベルライトスルーフラグ             */
    uint32_t attr_pcd:1;    /**< ページレベルキャッシュディスエーブルフラグ */
    uint32_t attr_a  :1;    /**< アクセス済みフラグ                         */
    uint32_t attr_d  :1;    /**< ダーティフラグ                             */
    uint32_t attr_pat:1;    /**< ページ属性テーブルインデックスフラグ       */
    uint32_t attr_g  :1;    /**< グローバルフラグ                           */
    uint32_t attr_avl:3;    /**< 未使用                                     */
    uint32_t base    :20;   /**< ページベースアドレス                       */
} IA32PagingPTE_t;

/** ページテーブル */
typedef struct {
    IA32PagingPTE_t entry[ IA32_PAGING_PTE_NUM ];   /**< エントリ */
} IA32PagingTbl_t;
/* [memo] 4096byteアライメントで配置する必要がある。 */


/******************************************************************************/
#endif
