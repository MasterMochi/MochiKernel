/******************************************************************************/
/* src/include/kernel/config.h                                                */
/*                                                                 2018/06/16 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
#ifndef _MK_CONFIG_H_
#define _MK_CONFIG_H_
/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* プロセスID定義 */
#define MK_CONFIG_PID_MAX           ( 1024 )            /** プロセスID最大値           */
#define MK_CONFIG_PID_MIN           (    0 )            /** プロセスID最小値(変更不可) */
#define MK_CONFIG_PID_NUM                               \
    ( MK_CONFIG_PID_MAX - MK_CONFIG_PID_MIN + 1 )       /** プロセスID数               */
#define MK_CONFIG_PID_NULL                              \
    ( MK_CONFIG_PID_MAX + 1 )                           /** プロセスID無効値           */

/* スレッドID定義 */
#define MK_CONFIG_TID_MAX           ( 4 )               /** スレッドID最大値           */
#define MK_CONFIG_TID_MIN           ( 0 )               /** スレッドID最小値(変更不可) */
#define MK_CONFIG_TID_NUM                               \
    ( MK_CONFIG_TID_MAX - MK_CONFIG_TID_MIN + 1 )       /** スレッドID数               */
#define MK_CONFIG_TID_NULL                              \
    ( MK_CONFIG_TID_MAX + 1 )                           /** スレッドID無効値           */

/* タスクID定義 */
#define MK_CONFIG_TASKID_MAKE( _PID, _TID )             \
    ( MK_CONFIG_PID_MAX * ( _TID ) + ( _PID ) )         /** タスクID作成マクロ */
#define MK_CONFIG_TASKID_MAX                            \
    ( MK_CONFIG_TASKID_MAKE( MK_CONFIG_PID_MAX,         \
                             MK_CONFIG_TID_MAX  ) )     /** タスクID最大値     */
#define MK_CONFIG_TASKID_MIN                            \
    ( MK_CONFIG_TASKID_MAKE( MK_CONFIG_PID_MIN,         \
                             MK_CONFIG_TID_MIN  ) )     /** タスクID最小値     */
#define MK_CONFIG_TASKID_NUM                            \
    ( MK_CONFIG_TASKID_MAX - MK_CONFIG_TASKID_MIN + 1 ) /** タスクID数         */
#define MK_CONFIG_TASKID_NULL                           \
    ( MK_CONFIG_TASKID_MAX + 1 )                        /** 無効タスクID       */

/* 仮想メモリマップ定義 */
#define MK_CONFIG_ADDR_KERNEL_START ( 0x00100000 )      /** カーネル領域先頭アドレス           */
#define MK_CONFIG_ADDR_KERNEL_STACK ( 0x3EFFE000 )      /** カーネル用スタック領域先頭アドレス */
#define MK_CONFIG_SIZE_KERNEL_STACK ( 0x00002000 )      /** カーネル用スタック領域サイズ       */
#define MK_CONFIG_ADDR_KERNEL_MAP1  ( 0x3F000000 )      /** メモリ制御用領域1先頭アドレス      */
#define MK_CONFIG_SIZE_KERNEL_MAP1  ( 0x00800000 )      /** メモリ制御用領域1サイズ            */
#define MK_CONFIG_ADDR_KERNEL_MAP2  ( 0x3F800000 )      /** メモリ制御用領域2先頭アドレス      */
#define MK_CONFIG_SIZE_KERNEL_MAP2  ( 0x00800000 )      /** メモリ制御用領域2サイズ            */
#define MK_CONFIG_SIZE_KERNEL_MAP   ( 0x01000000 )      /** メモリ制御用領域全サイズ           */
#define MK_CONFIG_ADDR_APL_START    ( 0x40000000 )      /** アプリ領域先頭アドレス             */
#define MK_CONFIG_ADDR_APL_STACK    ( 0xFFFFE000 )      /** アプリ用スタック領域先頭アドレス   */
#define MK_CONFIG_SIZE_APL_STACK    ( 0x00002000 )      /** アプリ用スタック領域サイズ         */

/* 割込み番号 */
#define MK_CONFIG_INTNO_MSGPASSING  ( 0x30 )            /** メッセージパッシング割込み番号   */
#define MK_CONFIG_INTNO_IOPORT      ( 0x31 )            /** I/Oポート制御割込み番号          */
#define MK_CONFIG_INTNO_INTERRUPT   ( 0x32 )            /** ハードウェア割込み制御割込み番号 */


/******************************************************************************/
#endif
