/******************************************************************************/
/* src/include/string.h                                                       */
/*                                                                 2018/11/24 */
/* Copyright (C) 2016-2017 Mochi                                              */
/******************************************************************************/
#ifndef _STRING_H_
#define _STRING_H_
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stddef.h>


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
extern void *memcpy( void       *s1,
                     const void *s2,
                     size_t     n    );

extern void *memset( void   *s,
                     int    c,
                     size_t n   );

extern char *strcpy( char       *s1,
                     const char *s2  );

extern size_t strlen( const char *s );


/******************************************************************************/
#endif
