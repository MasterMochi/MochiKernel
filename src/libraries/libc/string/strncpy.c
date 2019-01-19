/******************************************************************************/
/* src/libraries/libc/string/strncpy.c                                        */
/*                                                                 2019/01/13 */
/* Copyright (C) 2019 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stddef.h>


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       strncpy
 * @details     文字列をコピー文字数分またはnull文字までコピーする。
 *
 * @param[in]   *s1 コピー先文字列
 * @param[in]   *s2 コピー元文字列
 * @param[in]   n   コピー文字数
 *
 * @return      コピー先文字列
 *
 * @note        文字数にはnull文字も含まれる。文字数内にnull文字がない場合は、
 *              コピー先文字列にはnull文字は含まれない。
 */
/******************************************************************************/
char *strncpy( char       *s1,
               const char *s2,
               size_t     n    )
{
    char *pRet;

    /* 初期化 */
    pRet = s1;

    /* null文字まで繰り返し */
    while ( ( *s2 != '\0' ) && ( n > 1 ) ) {
        /* コピー */
        *s1 = *s2;

        /* アドレス・文字数更新 */
        s1++;
        s2++;
        n--;
    }

    /* 最終文字コピー */
    *s1 = *s2;

    return pRet;
}


/******************************************************************************/
