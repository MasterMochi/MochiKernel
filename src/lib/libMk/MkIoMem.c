/******************************************************************************/
/*                                                                            */
/* src/lib/libMk/MkIoMem.c                                                    */
/*                                                                 2019/04/02 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdint.h>

/* 外部モジュールヘッダ */
#include <kernel/iomem.h>

/* 内部モジュールヘッダ */


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       I/Oメモリ割当
 * @details     指定したI/Oメモリアドレスを割り当てる。
 *
 * @param[in]   *pAddr I/Oメモリ領域先頭アドレス
 * @param[in]   size   I/Oメモリ領域サイズ
 * @param[out]  *pErrNo エラー番号
 *                  - MK_IOMEM_ERR_NONE         エラー無し
 *                  - MK_IOMEM_ERR_UNAUTHORIZED 権限無し
 *                  - MK_IOMEM_ERR_IO_ALLOC     I/Oメモリ領域割当失敗
 *                  - MK_IOMEM_ERR_VIRT_ALLOC   仮想メモリ領域割当失敗
 *                  - MK_IOMEM_ERR_PAGE_SET     ページ設定失敗
 *
 * @return      割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功(割当先メモリアドレス)
 */
/******************************************************************************/
void *MkIoMemAlloc( void     *pAddr,
                    size_t   size,
                    uint32_t *pErrNo )
{
    volatile MkIoMemParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_IOMEM_FUNCID_ALLOC;
    param.errNo  = MK_IOMEM_ERR_NONE;
    param.pRet   = NULL;
    param.pAddr  = pAddr;
    param.size   = size;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                ),
                             "i" ( MK_CONFIG_INTNO_IOMEM )  );

    /* エラー番号設定要否判定 */
    if ( pErrNo != NULL ) {
        /* 必要 */

        /* エラー番号設定 */
        *pErrNo = param.errNo;
    }

    return param.pRet;
}


/******************************************************************************/
