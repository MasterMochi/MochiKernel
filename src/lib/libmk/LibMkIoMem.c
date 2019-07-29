/******************************************************************************/
/*                                                                            */
/* src/lib/libmk/LibMkIoMem.c                                                 */
/*                                                                 2019/07/27 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdint.h>

/* ライブラリヘッダ */
#include <MLib/MLib.h>

/* カーネルヘッダ */
#include <kernel/iomem.h>


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       I/Oメモリ割当
 * @details     指定したI/Oメモリアドレスを割り当てる。
 *
 * @param[in]   *pIoAddr     I/Oメモリ領域先頭アドレス
 * @param[in]   size         I/Oメモリ領域サイズ
 * @param[out]  **ppVirtAddr 割当先メモリアドレス
 * @param[out]  *pErr        エラー内容
 *                  - MK_ERR_NONE         エラー無し
 *                  - MK_ERR_UNAUTHORIZED 権限無し
 *                  - MK_ERR_IO_ALLOC     I/Oメモリ領域割当失敗
 *                  - MK_ERR_VIRT_ALLOC   仮想メモリ領域割当失敗
 *                  - MK_ERR_PAGE_SET     ページ設定失敗
 *
 * @return      割当結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkIoMemAlloc( void    *pIoAddr,
                         size_t  size,
                         void    **ppVirtAddr,
                         MkErr_t *pErr         )
{
    volatile MkIoMemParam_t param;

    /* 引数チェック */
    if ( ppVirtAddr == NULL ) {
        /* 不正 */

        /* エラー内容設定 */
        MLIB_SET_IFNOT_NULL( pErr, MK_ERR_PARAM );

        return MK_RET_FAILURE;
    }

    /* パラメータ設定 */
    param.funcId    = MK_IOMEM_FUNCID_ALLOC;
    param.ret       = MK_RET_FAILURE;
    param.err       = MK_ERR_NONE;
    param.pIoAddr   = pIoAddr;
    param.size      = size;
    param.pVirtAddr = NULL;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param         ),
                             "i" ( MK_IOMEM_INTNO )
                           : "esi"                   );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    /* 割当先メモリアドレス設定 */
    *ppVirtAddr = param.pVirtAddr;

    return param.ret;
}


/******************************************************************************/
