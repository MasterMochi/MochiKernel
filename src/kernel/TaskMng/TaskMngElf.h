/******************************************************************************/
/*                                                                            */
/* src/kernel/TaskMng/TaskMngElf.h                                            */
/*                                                                 2019/07/23 */
/* Copyright (C) 2017-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef TASKMNG_ELF_H
#define TASKMNG_ELF_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
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
                                void     **ppEntryPoint,
                                void     **ppEndPoint    );


/******************************************************************************/
#endif
