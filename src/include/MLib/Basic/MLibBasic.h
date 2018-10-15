/******************************************************************************/
/* src/include/MLib/Basic/MLibBasic.h                                         */
/*                                                                 2018/10/05 */
/* Copyright (C) 2017-2018 Mochi.                                             */
/******************************************************************************/
#ifndef _MLIB_BASIC_H_
#define _MLIB_BASIC_H_
/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** アライメント計算マクロ */
#define MLIB_BASIC_ALIGN( _VALUE, _ALIGNMENT )                            \
    ( ( ( _VALUE ) + ( ( _ALIGNMENT ) - 1 ) ) & ~( ( _ALIGNMENT ) - 1 ) )

/** フラグ判定マクロ */
#define MLIB_BASIC_HAVE_FLAG( _VALUE, _FLAG )   \
    ( ( ( _VALUE ) & ( _FLAG ) ) == ( _FLAG ) )

/** 最大値取得マクロ */
#define MLIB_BASIC_MAX( _VALUE1, _VALUE2 )                    \
    ( ( _VALUE1 ) > ( _VALUE2 ) ? ( _VALUE1 ) : ( _VALUE2 ) )

/** 最小値取得マクロ */
#define MLIB_BASIC_MIN( _VALUE1, _VALUE2 )                    \
    ( ( _VALUE1 ) < ( _VALUE2 ) ? ( _VALUE1 ) : ( _VALUE2 ) )

/** 配列エントリ数マクロ */
#define MLIB_BASIC_NUMOF( _ARRAY ) \
    ( sizeof ( _ARRAY ) / sizeof ( _ARRAY[ 0 ] ) )


/******************************************************************************/
#endif
