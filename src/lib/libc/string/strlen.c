/******************************************************************************/
/*                                                                            */
/* src/lib/libc/string/strlen.c                                               */
/*                                                                 2019/04/02 */
/* Copyright (C) 2016-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
#include <stddef.h>


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       strlen
 * @details     文字列の文字数を返す。
 *
 * @param[in]   *s 文字列
 *
 * @return      文字数
 *
 * @note        null文字はカウントされない。
 */
/******************************************************************************/
size_t strlen( const char *s )
{
    size_t ret;

    /* 初期化 */
    ret = 0;

    /* null文字まで繰り返し */
    while ( *s != '\0' ) {
        /* 文字数加算 */
        ret++;

        /* アドレス更新 */
        s++;
    }

    return ret;
}


/******************************************************************************/