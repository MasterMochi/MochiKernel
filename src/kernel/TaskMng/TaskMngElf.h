/******************************************************************************/
/* src/kernel/TaskMng/TaskMngElf.h                                            */
/*                                                                 2018/05/09 */
/* Copyright (C) 2017-2018 Mochi.                                             */
/******************************************************************************/
#ifndef TASKMNG_ELF_H
#define TASKMNG_ELF_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stddef.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/* ELFファイル読込 */
extern CmnRet_t TaskMngElfLoad( void     *pAddr,
                                size_t   size,
                                uint32_t pageDirId,
                                void     **ppEntryPoint );


/******************************************************************************/
#endif
