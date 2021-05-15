/******************************************************************************/
/*                                                                            */
/* src/kernel/include/Memmng.h                                                */
/*                                                                 2021/05/05 */
/* Copyright (C) 2016-2021 Mochi.                                             */
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
#include <hardware/IA32/IA32Descriptor.h>
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
#define MEMMNG_GDT_ENTRY_FULL (  0 )    /**< GDTエントリ空き無し   */
#define MEMMNG_GDT_ENTRY_MIN  (  1 )    /**< GDTエントリ番号最小値 */
#define MEMMNG_GDT_ENTRY_MAX  (  9 )    /**< GDTエントリ番号最大値 */
#define MEMMNG_GDT_ENTRY_NUM   \
    ( MEMMNG_GDT_ENTRY_MAX + 1 )        /**< GDTエントリ数         */

/* GDTインデックス */
#define MEMMNG_GDT_IDX_KERNEL_CODE ( 1 )    /**< カーネルコードGDTインデックス */
#define MEMMNG_GDT_IDX_KERNEL_DATA ( 2 )    /**< カーネルデータGDTインデックス */
#define MEMMNG_GDT_IDX_APL_CODE    ( 3 )    /**< アプリコードGDTインデックス   */
#define MEMMNG_GDT_IDX_APL_DATA    ( 4 )    /**< アプリデータGDTインデックス   */

/** カーネルコードセグメントセレクタ */
#define MEMMNG_SEGSEL_KERNEL_CODE \
    IA32_SEGMENT_SELECTOR( MEMMNG_GDT_IDX_KERNEL_CODE, IA32_TI_GDT, IA32_RPL_0 )
/** カーネルデータセグメントセレクタ */
#define MEMMNG_SEGSEL_KERNEL_DATA \
    IA32_SEGMENT_SELECTOR( MEMMNG_GDT_IDX_KERNEL_DATA, IA32_TI_GDT, IA32_RPL_0 )
/** アプリコードセグメントセレクタ */
#define MEMMNG_SEGSEL_APL_CODE    \
    IA32_SEGMENT_SELECTOR( MEMMNG_GDT_IDX_APL_CODE   , IA32_TI_GDT, IA32_RPL_3 )
/** アプリデータセグメントセレクタ */
#define MEMMNG_SEGSEL_APL_DATA    \
    IA32_SEGMENT_SELECTOR( MEMMNG_GDT_IDX_APL_DATA   , IA32_TI_GDT, IA32_RPL_3 )

/** ページディレクトリID */
#define MEMMNG_PAGE_DIR_ID_NULL ( 0 )   /**< 無効ページディレクトリID   */
#define MEMMNG_PAGE_DIR_ID_IDLE ( 0 )   /**< アイドルプロセス用PDID     */
#define MEMMNG_PAGE_DIR_ID_MIN  ( 1 )   /**< ページディレクトリID最小値 */

/** ページディレクトリID */
typedef uint32_t MemmngPageDirId_t;


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/*----------*/
/* Memmng.c */
/*----------*/
/* メモリ管理初期化 */
extern void MemmngInit( BiosE820Entry_t *pBiosE820,
                        size_t          biosE820Num,
                        MkMemMapEntry_t *pMemMap,
                        size_t          memMapNum    );

/*--------------*/
/* MemmngCtrl.c */
/*--------------*/
/* メモリコピー（仮想->物理） */
extern void MemmngCtrlCopyVirtToPhys( void   *pPAddr,
                                      void   *pVAddr,
                                      size_t size     );
/* メモリ設定 */
extern void MemmngCtrlSet( void    *pPAddr,
                           uint8_t value,
                           size_t  size     );

/*--------------*/
/* MemmngHeap.c */
/*--------------*/
/* カーネルヒープ領域割当 */
extern void *MemmngHeapAlloc( size_t size );
/* カーネルヒープ領域解放 */
extern void MemmngHeapFree( void *pAddr );

/*------------*/
/* MemmngIo.c */
/*------------*/
/* I/Oメモリ領域割当 */
extern void *MemmngIoAlloc( void   *pAddr,
                            size_t size    );
/* I/Oメモリ領域解放 */
extern CmnRet_t MemmngIoFree( void *pAddr );

/*-------------*/
/* MemmngMap.c */
/*-------------*/
extern CmnRet_t MemmngMapGetInfo( MkMemMapEntry_t *pInfo,
                                  uint32_t        type    );

/*--------------*/
/* MemmngPage.c */
/*--------------*/
/* ページディレクトリ割当 */
extern MemmngPageDirId_t MemmngPageAllocDir( MkPid_t pid );
/* ページディレクトリ解放 */
extern CmnRet_t MemmngPageFreeDir( MemmngPageDirId_t dirId );
/* ページディレクトリID取得 */
extern MemmngPageDirId_t MemmngPageGetDirId( void );
/* ページディレクトリベースレジスタ値取得 */
extern IA32PagingPDBR_t MemmngPageGetPdbr( MemmngPageDirId_t dirId );
/* ページマッピング設定 */
extern CmnRet_t MemmngPageSet( MemmngPageDirId_t dirId,
                               void              *pVirtAddr,
                               void              *pPhysAddr,
                               size_t            size,
                               uint32_t          attrGlobal,
                               uint32_t          attrUs,
                               uint32_t          attrRw      );
/* ページディレクトリ切替 */
extern void MemmngPageSwitchDir( MemmngPageDirId_t dirId );
/* ページマッピング解除 */
extern CmnRet_t MemmngPageUnset( MemmngPageDirId_t dirId,
                                 void              *pVirtAddr,
                                 size_t            size        );

/*--------------*/
/* MemmngPhys.c */
/*--------------*/
/* 物理メモリ領域割当 */
extern void *MemmngPhysAlloc( size_t size );
/* 物理メモリ領域解放 */
extern CmnRet_t MemmngPhysFree( void *pAddr );

/*--------------*/
/* MemmngSgmt.c */
/*--------------*/
/* GDTエントリ追加 */
extern uint16_t MemmngSgmtAdd( void    *pBase,
                               size_t  limit,
                               uint8_t limitG,
                               uint8_t sysFlg,
                               uint8_t type,
                               uint8_t level,
                               uint8_t opSize  );

/*--------------*/
/* MemmngVirt.c */
/*--------------*/
/* 仮想メモリ領域割当 */
extern void *MemmngVirtAlloc( MkPid_t pid,
                              size_t  size );
/* 仮想メモリ領域管理終了 */
extern CmnRet_t MemmngVirtEnd( MkPid_t pid );
/* 仮想メモリ領域解放 */
extern CmnRet_t MemmngVirtFree( MkPid_t pid,
                                void    *pAddr );
/* 仮想メモリ領域管理開始 */
extern CmnRet_t MemmngVirtStart( MkPid_t pid );


/******************************************************************************/
#endif
