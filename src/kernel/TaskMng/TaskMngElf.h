/******************************************************************************/
/*                                                                            */
/* src/kernel/TaskMng/TaskMngElf.h                                            */
/*                                                                 2021/05/05 */
/* Copyright (C) 2017-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef TASKMNG_ELF_H
#define TASKMNG_ELF_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stddef.h>
#include <stdint.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Memmng.h>


/******************************************************************************/
/* モジュール内グローバル関数宣言                                             */
/******************************************************************************/
/* ELFファイル読込 */
extern CmnRet_t ElfLoad( void              *pAddr,
                         size_t            size,
                         MemmngPageDirId_t dirId,
                         void              **ppEntryPoint,
                         void              **ppEndPoint    );


/******************************************************************************/
#endif
