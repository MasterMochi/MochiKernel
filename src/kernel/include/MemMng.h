/******************************************************************************/
/*                                                                            */
/* src/kernel/include/MemMng.h                                                */
/*                                                                 2019/06/12 */
/* Copyright (C) 2016-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef MEMMNG_H
#define MEMMNG_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stddef.h>
#include <stdint.h>

/* 共通ヘッダ */
#include <firmware/bios/e820.h>
#include <hardware/IA32/IA32Paging.h>
#include <kernel/config.h>
#include <kernel/kernel.h>
#include <kernel/types.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <TaskMng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* GDT定義 */
#define MEMMNG_GDT_ENTRY_FULL     (  0 )        /**< GDTエントリ空き無し   */
#define MEMMNG_GDT_ENTRY_MIN      (  1 )        /**< GDTエントリ番号最小値 */
#define MEMMNG_GDT_ENTRY_MAX      (  9 )        /**< GDTエントリ番号最大値 */
#define MEMMNG_GDT_ENTRY_NUM        \
    ( MEMMNG_GDT_ENTRY_MAX + 1 )                /**< GDTエントリ数         */

/* セグメントセレクタ定義 */
#define MEMMNG_SEGSEL_KERNEL_CODE ( 1 * 8     ) /**< カーネルコードセグメント */
#define MEMMNG_SEGSEL_KERNEL_DATA ( 2 * 8     ) /**< カーネルデータセグメント */
#define MEMMNG_SEGSEL_APL_CODE    ( 3 * 8 + 3 ) /**< アプリコードセグメント   */
#define MEMMNG_SEGSEL_APL_DATA    ( 4 * 8 + 3 ) /**< アプリデータセグメント   */

/** ページディレクトリID */
#define MEMMNG_PAGE_DIR_ID_IDLE   ( 0 )         /**< アイドルプロセス用PDID */
#define MEMMNG_PAGE_DIR_ID_MIN    ( 1 )         /**< PDID最小値             */

/* ページディレクトリ定義 */
#define MEMMNG_PAGE_DIR_NUM       MK_PID_NUM            /**< PD管理数   */
#define MEMMNG_PAGE_DIR_FULL      MEMMNG_PAGE_DIR_NUM   /**< PD空き無し */

/* ページテーブル定義 */
#define MEMMNG_PAGE_TBL_NUM       ( 4096 )              /**< PT管理数   */
#define MEMMNG_PAGE_TBL_FULL      MEMMNG_PAGE_TBL_NUM   /**< PT空き無し */


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
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
extern void MemMngInit( BiosE820Entry_t *pBiosE820,
                        size_t          biosE820Num,
                        MkMemMapEntry_t *pMemMap,
                        size_t          memMapNum    );


/*------------*/
/* MemMngIo.c */
/*------------*/
/* I/Oメモリ領域割当 */
extern void *MemMngIoAlloc( void   *pAddr,
                            size_t size    );

/* I/Oメモリ領域解放 */
extern CmnRet_t MemMngIoFree( void *pAddr );


/*-------------*/
/* MemMngMap.c */
/*-------------*/
extern CmnRet_t MemMngMapGetInfo( MkMemMapEntry_t *pInfo,
                                  uint32_t        type    );

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
extern IA32PagingPDBR_t MemMngPageSwitchDir( uint32_t pageDirId );

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
/* MemMngPhys.c */
/*--------------*/
/* 物理メモリ領域割当 */
extern void *MemMngPhysAlloc( size_t size );

/* 物理メモリ領域解放 */
extern CmnRet_t MemMngPhysFree( void *pAddr );


/*--------------*/
/* MemMngVirt.c */
/*--------------*/
/* 仮想メモリ領域割当 */
extern void *MemMngVirtAlloc( MkPid_t pid,
                              size_t  size );

/* 指定仮想メモリ領域割当 */
extern void *MemMngVirtAllocSpecified( MkPid_t pid,
                                       void    *pAddr,
                                       size_t  size    );

/* 仮想メモリ領域管理終了 */
extern CmnRet_t MemMngVirtEnd( MkPid_t pid );

/* 仮想メモリ領域解放 */
extern CmnRet_t MemMngVirtFree( MkPid_t pid,
                                void    *pAddr );

/* 仮想メモリ領域管理開始 */
extern CmnRet_t MemMngVirtStart( MkPid_t pid );


/******************************************************************************/
#endif
