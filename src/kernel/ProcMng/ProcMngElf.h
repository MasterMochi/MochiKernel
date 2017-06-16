/******************************************************************************/
/* src/kernel/ProcMng/ProcMngElf.h                                            */
/*                                                                 2017/06/09 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
#ifndef PROCMNG_ELF_H
#define PROCMNG_ELF_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stddef.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>

/* 内部モジュールヘッダ */
#include "ProcMngTask.h"


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/* ELFファイル読込 */
extern CmnRet_t ProcMngElfLoad( void             *pAddr,
                                size_t           size,
                                ProcMngTaskTbl_t *pTaskInfo );


/******************************************************************************/
#endif
