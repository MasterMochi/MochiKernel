/******************************************************************************/
/* src/include/MLib/MLib.h                                                    */
/*                                                                 2017/02/05 */
/* Copyright (C) 2017 Mochi                                                   */
/******************************************************************************/
#ifndef _MLIB_H_
#define _MLIB_H_
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
#include <stdint.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* 関数戻り値定義 */
#define MLIB_SUCCESS (  0 )     /** 正常終了 */
#define MLIB_FAILURE ( -1 )     /** 異常終了 */

/** 関数戻り値型 */
typedef int32_t MLibRet_t;


/******************************************************************************/
#endif