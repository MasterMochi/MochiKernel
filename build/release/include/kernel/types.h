/******************************************************************************/
/*                                                                            */
/* kernel/types.h                                                             */
/*                                                                 2019/06/13 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef _MK_TYPES_H_
#define _MK_TYPES_H_
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
#define MK_PID_MIN  0                   /**< プロセスID最小値(変更禁止) */
#define MK_PID_NUM  ( MK_PID_MAX + 1 )  /**< プロセスID数               */
#define MK_PID_NULL ( MK_PID_MAX + 1 )  /**< プロセスID無効値           */

/** プロセスID */
typedef uint32_t MkPid_t;


/*----------------*/
/* スレッドID定義 */
/*----------------*/
#define MK_TID_MAX  MK_CONFIG_TID_MASK  /**< スレッドID最大値           */
#define MK_TID_MIN  0                   /**< スレッドID最小値(変更禁止) */
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


/******************************************************************************/
#endif
