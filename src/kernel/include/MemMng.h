/******************************************************************************/
/* src/kernel/include/MemMng.h                                                */
/*                                                                 2017/05/24 */
/* Copyright (C) 2016-2017 Mochi.                                             */
/******************************************************************************/
#ifndef MEMMNG_H
#define MEMMNG_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stddef.h>
#include <stdint.h>
#include <kernel/MochiKernel.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <ProcMng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* GDT定義 */
#define MEMMNG_GDT_ENTRY_FULL     (  0 )        /**< GDTエントリ空き無し   */
#define MEMMNG_GDT_ENTRY_MIN      (  1 )        /**< GDTエントリ番号最小値 */
#define MEMMNG_GDT_ENTRY_MAX      (  9 )        /**< GDTエントリ番号最大値 */
#define MEMMNG_GDT_ENTRY_NUM   \
    ( MEMMNG_GDT_ENTRY_MAX + 1 )                /**< GDTエントリ数         */

/* セグメントセレクタ定義 */
#define MEMMNG_SEGSEL_KERNEL_CODE ( 1 * 8     ) /**< カーネルコードセグメント */
#define MEMMNG_SEGSEL_KERNEL_DATA ( 2 * 8     ) /**< カーネルデータセグメント */
#define MEMMNG_SEGSEL_DRIVER_CODE ( 3 * 8 + 1 ) /**< ドライバコードセグメント */
#define MEMMNG_SEGSEL_DRIVER_DATA ( 4 * 8 + 1 ) /**< ドライバデータセグメント */
#define MEMMNG_SEGSEL_SERVER_CODE ( 5 * 8 + 2 ) /**< サーバコードセグメント   */
#define MEMMNG_SEGSEL_SERVER_DATA ( 6 * 8 + 2 ) /**< サーバデータセグメント   */
#define MEMMNG_SEGSEL_USER_CODE   ( 7 * 8 + 3 ) /**< ユーザコードセグメント   */
#define MEMMNG_SEGSEL_USER_DATA   ( 8 * 8 + 3 ) /**< ユーザデータセグメント   */

/** ページディレクトリID */
#define MEMMNG_PAGE_DIR_ID_IDLE   ( 0 )         /**< アイドルプロセス用PDID */
#define MEMMNG_PAGE_DIR_ID_MIN    ( 1 )         /**< PDID最小値             */

/* ページディレクトリ定義 */
#define MEMMNG_PAGE_DIR_NUM       PROCMNG_TASK_ID_NUM   /** PD管理数   */
#define MEMMNG_PAGE_DIR_FULL      MEMMNG_PAGE_DIR_NUM   /** PD空き無し */

/* ページテーブル定義 */
#define MEMMNG_PAGE_TBL_NUM       ( 4096 )              /** PT管理数   */
#define MEMMNG_PAGE_TBL_FULL      MEMMNG_PAGE_TBL_NUM   /** PT空き無し */

/** メモリ領域情報 */
typedef struct {
    void   *pAddr;      /**< 先頭アドレス */
    size_t size;        /**< メモリサイズ */
} MemMngAreaInfo_t;


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/*--------------*/
/* MemMngArea.c */
/*--------------*/
/* メモリ領域割当 */
extern void *MemMngAreaAlloc( size_t size );

/* メモリ領域解放 */
extern CmnRet_t MemMngAreaFree( void *pAddr );

/* メモリ領域情報取得 */
extern CmnRet_t MemMngAreaGetInfo( MemMngAreaInfo_t *pInfo,
                                   uint32_t         type    );


/*-------------*/
/* MemMngGdt.c */
/*-------------*/
/* GDTエントリ追加 */
extern uint16_t MemMngGdtAdd( void    *pBase,
                              size_t  limit,
                              uint8_t limitG,
                              uint8_t sysFlg,
                              uint8_t type,
                              uint8_t level,
                              uint8_t opSize  );


/*--------------*/
/* MemMngInit.c */
/*--------------*/
/* メモリ管理初期化 */
extern void MemMngInit( MochiKernelMemoryMap_t *pMap,
                        size_t                 mapSize );


/*--------------*/
/* MemMngPage.c */
/*--------------*/
/* ページディレクトリ割当 */
extern uint32_t MemMngPageAllocDir( void );

/* ページディレクトリ解放 */
extern CmnRet_t MemMngPageFreeDir( uint32_t id );

/* ページディレクトリID取得 */
extern uint32_t MemMngPageGetDirId( void );

/* ページディレクトリ切替 */
extern void MemMngPageSwitchDir( uint32_t id );

/* ページマッピング設定 */
extern CmnRet_t MemMngPageSet( uint32_t dirId,
                               void     *pVAddr,
                               void     *pPAddr,
                               size_t   size,
                               uint32_t attrGlobal,
                               uint32_t attrUs,
                               uint32_t attrRw      );

/* ページマッピング解除 */
extern void MemMngPageUnset( uint32_t dirId,
                             void     *pVAddr,
                             size_t   size     );


/*--------------*/
/* MemMngCtrl.c */
/*--------------*/
/* メモリコピー（仮想->物理） */
extern void MemMngCtrlCopyVirtToPhys( void   *pPAddr,
                                      void   *pVAddr,
                                      size_t size     );

/* メモリ設定 */
extern void MemMngCtrlSet( void    *pPAddr,
                           uint8_t value,
                           size_t  size     );


/******************************************************************************/
#endif
