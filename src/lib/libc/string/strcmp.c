/******************************************************************************/
/*                                                                            */
/* src/lib/libc/string/strcmp.c                                               */
/*                                                                 2019/08/28 */
/* Copyright (C) 2019 Mochi.                                                  */
/*                                                                            */
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
 * @brief       strcmp
 * @details     文字列s1と文字列s2をnull文字まで1文字づつ比較した結果を返す。
 *
 * @param[in]   *s1 文字列
 * @param[in]   *s2 文字列
 *
 * @return      最後に比較した文字の差(s1-s2)を返す。
 * @retval      0     同じ文字列
 * @retval      0以外 異なる文字列
 */
/******************************************************************************/
int strcmp( const char *s1,
            const char *s2  )
{
    /* 1文字毎に繰り返す */
    while ( *s1 == *s2 ) {
        /* null文字判定 */
        if ( *s1 == '\0' ) {
            /* null文字 */
            break;
        }

        s1++;
        s2++;
    }

    return ( int ) ( *s1 - *s2 );
}


/******************************************************************************/
