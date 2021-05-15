/******************************************************************************/
/*                                                                            */
/* kernel/types.h                                                             */
/*                                                                 2021/04/27 */
/* Copyright (C) 2018-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef __KERNEL_TYPES_H_
#define __KERNEL_TYPES_H_
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdint.h>

/* カーネルヘッダ */
#include "config.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/*----------------*/
/* プロセスID定義 */
/*----------------*/
#define MK_PID_MAX  MK_CONFIG_PID_MASK  /**< プロセスID最大値           */
#define MK_PID_MIN  ( 0 )               /**< プロセスID最小値(変更禁止) */
#define MK_PID_NUM  ( MK_PID_MAX + 1 )  /**< プロセスID数               */
#define MK_PID_NULL ( MK_PID_MAX + 1 )  /**< プロセスID無効値           */
#define MK_PID_IDLE ( 0 )               /**< アイドルプロセスID         */

/** プロセスID */
typedef uint32_t MkPid_t;


/*----------------*/
/* スレッドID定義 */
/*----------------*/
#define MK_TID_MAX  MK_CONFIG_TID_MASK  /**< スレッドID最大値           */
#define MK_TID_MIN  ( 0 )               /**< スレッドID最小値(変更禁止) */
#define MK_TID_NUM  ( MK_TID_MAX + 1 )  /**< スレッドID数               */
#define MK_TID_NULL ( MK_TID_MAX + 1 )  /**< スレッドID無効値           */

/** スレッドID */
typedef uint32_t MkTid_t;


/*--------------*/
/* タスクID定義 */
/*--------------*/
/** タスクID作成マクロ */
#define MK_TASKID_MAKE( _PID, _TID ) ( ( _TID ) * MK_PID_NUM + ( _PID ) )
/** タスクID->プロセスID変換マクロ */
#define MK_TASKID_TO_PID( _TASKID )  ( ( _TASKID ) & MK_CONFIG_PID_MASK )
/** タスクID->スレッドID変換マクロ */
#define MK_TASKID_TO_TID( _TASKID )  ( ( _TASKID ) / MK_PID_NUM )

/** タスクID最大値 */
#define MK_TASKID_MAX  ( MK_TASKID_MAKE( MK_PID_MAX, MK_TID_MAX ) )
/** タスクID最小値 */
#define MK_TASKID_MIN  ( MK_TASKID_MAKE( MK_PID_MIN, MK_TID_MIN ) )
/** タスクID数 */
#define MK_TASKID_NUM  ( MK_TASKID_MAX - MK_TASKID_MIN + 1 )
/** 無効タスクID */
#define MK_TASKID_NULL ( MK_TASKID_MAX + 1 )

/** タスクID   */
typedef uint32_t MkTaskId_t;


/*------------*/
/* 戻り値定義 */
/*------------*/
#define MK_RET_SUCCESS (  0 )   /**< 成功 */
#define MK_RET_FAILURE ( -1 )   /**< 失敗 */

/** 戻り値型 */
typedef int32_t MkRet_t;


/*------------*/
/* エラー定義 */
/*------------*/
#define MK_ERR_NONE          ( 0x00000000 ) /**< エラー無し             */
#define MK_ERR_PARAM         ( 0x00000001 ) /**< パラメータ不正         */
#define MK_ERR_UNAUTHORIZED  ( 0x00000002 ) /**< 権限無し               */
#define MK_ERR_ALREADY_START ( 0x00000003 ) /**< 開始済み               */
#define MK_ERR_IO_ALLOC      ( 0x00000004 ) /**< I/Oメモリ領域割当失敗  */
#define MK_ERR_VIRT_ALLOC    ( 0x00000005 ) /**< 仮想メモリ領域割当失敗 */
#define MK_ERR_PAGE_SET      ( 0x00000006 ) /**< ページ設定失敗         */
#define MK_ERR_NO_EXIST      ( 0x00000007 ) /**< 存在しない             */
#define MK_ERR_NO_MEMORY     ( 0x00000008 ) /**< メモリ不足             */
#define MK_ERR_SIZE_OVER     ( 0x00000009 ) /**< サイズ超過             */
#define MK_ERR_NO_REGISTERED ( 0x0000000A ) /**< 登録無し               */
#define MK_ERR_REGISTERED    ( 0x0000000B ) /**< 登録済み               */
#define MK_ERR_NO_RESOURCE   ( 0x0000000C ) /**< リソース不足           */
#define MK_ERR_TIMEOUT       ( 0x0000000D ) /**< タイムアウト           */

/** エラー型 */
typedef uint32_t MkErr_t;


/******************************************************************************/
#endif
