/******************************************************************************/
/* src/libraries/libMk/MkIoPort.c                                             */
/*                                                                 2018/06/18 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdint.h>

/* 外部モジュールヘッダ */
#include <kernel/ioport.h>

/* 内部モジュールヘッダ */


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       I/Oポート入力(1バイト単位)
 * @details     指定したポート番号から1バイトのデータを指定回数分入力する。
 *
 * @param[in]   portNo ポート番号
 * @param[out]  *pData データ格納先
 * @param[in]   count  読込み回数
 * @param[out]  *pErrNo エラー番号
 *                  - MK_IOPORT_ERR_NONE         エラー無し
 *                  - MK_IOPORT_ERR_UNAUTHORIZED 権限無し
 *
 * @return      処理結果を返す。
 * @retval      MK_IOPORT_RET_SUCCESS 成功
 * @retval      MK_IOPORT_RET_FAILURE 失敗
 */
/******************************************************************************/
int32_t MkIoPortInByte( uint16_t portNo,
                        void     *pData,
                        size_t   count,
                        uint32_t *pErrNo )
{
    volatile MkIoPortParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_IOPORT_FUNCID_IN_BYTE;
    param.errNo  = MK_IOPORT_ERR_NONE;
    param.ret    = MK_IOPORT_RET_FAILURE;
    param.portNo = portNo;
    param.pData  = pData;
    param.count  = count;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                 ),
                             "i" ( MK_CONFIG_INTNO_IOPORT )  );

    /* エラー番号設定要否判定 */
    if ( pErrNo != NULL ) {
        /* 必要 */

        /* エラー番号設定 */
        *pErrNo = param.errNo;
    }

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       I/Oポート入力(4バイト単位)
 * @details     指定したポート番号から4バイトのデータを指定回数分入力する。
 *
 * @param[in]   portNo ポート番号
 * @param[out]  *pData データ格納先
 * @param[in]   count  読込み回数
 * @param[out]  *pErrNo エラー番号
 *                  - MK_IOPORT_ERR_NONE         エラー無し
 *                  - MK_IOPORT_ERR_UNAUTHORIZED 権限無し
 *
 * @return      処理結果を返す。
 * @retval      MK_IOPORT_RET_SUCCESS 成功
 * @retval      MK_IOPORT_RET_FAILURE 失敗
 */
/******************************************************************************/
int32_t MkIoPortInDWord( uint16_t portNo,
                         void     *pData,
                         size_t   count,
                         uint32_t *pErrNo )
{
    volatile MkIoPortParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_IOPORT_FUNCID_IN_DWORD;
    param.errNo  = MK_IOPORT_ERR_NONE;
    param.ret    = MK_IOPORT_RET_FAILURE;
    param.portNo = portNo;
    param.pData  = pData;
    param.count  = count;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                 ),
                             "i" ( MK_CONFIG_INTNO_IOPORT )  );

    /* エラー番号設定要否判定 */
    if ( pErrNo != NULL ) {
        /* 必要 */

        /* エラー番号設定 */
        *pErrNo = param.errNo;
    }

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       I/Oポート入力(2バイト単位)
 * @details     指定したポート番号から2バイトのデータを指定回数分入力する。
 *
 * @param[in]   portNo ポート番号
 * @param[out]  *pData データ格納先
 * @param[in]   count  読込み回数
 * @param[out]  *pErrNo エラー番号
 *                  - MK_IOPORT_ERR_NONE         エラー無し
 *                  - MK_IOPORT_ERR_UNAUTHORIZED 権限無し
 *
 * @return      処理結果を返す。
 * @retval      MK_IOPORT_RET_SUCCESS 成功
 * @retval      MK_IOPORT_RET_FAILURE 失敗
 */
/******************************************************************************/
int32_t MkIoPortInWord( uint16_t portNo,
                        void     *pData,
                        size_t   count,
                        uint32_t *pErrNo )
{
    volatile MkIoPortParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_IOPORT_FUNCID_IN_WORD;
    param.errNo  = MK_IOPORT_ERR_NONE;
    param.ret    = MK_IOPORT_RET_FAILURE;
    param.portNo = portNo;
    param.pData  = pData;
    param.count  = count;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                 ),
                             "i" ( MK_CONFIG_INTNO_IOPORT )  );

    /* エラー番号設定要否判定 */
    if ( pErrNo != NULL ) {
        /* 必要 */

        /* エラー番号設定 */
        *pErrNo = param.errNo;
    }

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       I/Oポート出力(1バイト単位)
 * @details     指定したポート番号に1バイトのデータを指定回数分出力する。
 *
 * @param[in]   portNo ポート番号
 * @param[out]  *pData データ格納先
 * @param[in]   count  読込み回数
 * @param[out]  *pErrNo エラー番号
 *                  - MK_IOPORT_ERR_NONE         エラー無し
 *                  - MK_IOPORT_ERR_UNAUTHORIZED 権限無し
 *
 * @return      処理結果を返す。
 * @retval      MK_IOPORT_RET_SUCCESS 成功
 * @retval      MK_IOPORT_RET_FAILURE 失敗
 */
/******************************************************************************/
int32_t MkIoPortOutByte( uint16_t portNo,
                         void     *pData,
                         size_t   count,
                         uint32_t *pErrNo )
{
    volatile MkIoPortParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_IOPORT_FUNCID_OUT_BYTE;
    param.errNo  = MK_IOPORT_ERR_NONE;
    param.ret    = MK_IOPORT_RET_FAILURE;
    param.portNo = portNo;
    param.pData  = pData;
    param.count  = count;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                 ),
                             "i" ( MK_CONFIG_INTNO_IOPORT )  );

    /* エラー番号設定要否判定 */
    if ( pErrNo != NULL ) {
        /* 必要 */

        /* エラー番号設定 */
        *pErrNo = param.errNo;
    }

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       I/Oポート出力(4バイト単位)
 * @details     指定したポート番号に4バイトのデータを指定回数分出力する。
 *
 * @param[in]   portNo ポート番号
 * @param[out]  *pData データ格納先
 * @param[in]   count  読込み回数
 * @param[out]  *pErrNo エラー番号
 *                  - MK_IOPORT_ERR_NONE         エラー無し
 *                  - MK_IOPORT_ERR_UNAUTHORIZED 権限無し
 *
 * @return      処理結果を返す。
 * @retval      MK_IOPORT_RET_SUCCESS 成功
 * @retval      MK_IOPORT_RET_FAILURE 失敗
 */
/******************************************************************************/
int32_t MkIoPortOutDWord( uint16_t portNo,
                          void     *pData,
                          size_t   count,
                          uint32_t *pErrNo )
{
    volatile MkIoPortParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_IOPORT_FUNCID_OUT_DWORD;
    param.errNo  = MK_IOPORT_ERR_NONE;
    param.ret    = MK_IOPORT_RET_FAILURE;
    param.portNo = portNo;
    param.pData  = pData;
    param.count  = count;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                 ),
                             "i" ( MK_CONFIG_INTNO_IOPORT )  );

    /* エラー番号設定要否判定 */
    if ( pErrNo != NULL ) {
        /* 必要 */

        /* エラー番号設定 */
        *pErrNo = param.errNo;
    }

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       I/Oポート出力(2バイト単位)
 * @details     指定したポート番号に2バイトのデータを指定回数分出力する。
 *
 * @param[in]   portNo ポート番号
 * @param[out]  *pData データ格納先
 * @param[in]   count  読込み回数
 * @param[out]  *pErrNo エラー番号
 *                  - MK_IOPORT_ERR_NONE         エラー無し
 *                  - MK_IOPORT_ERR_UNAUTHORIZED 権限無し
 *
 * @return      処理結果を返す。
 * @retval      MK_IOPORT_RET_SUCCESS 成功
 * @retval      MK_IOPORT_RET_FAILURE 失敗
 */
/******************************************************************************/
int32_t MkIoPortOutWord( uint16_t portNo,
                         void     *pData,
                         size_t   count,
                         uint32_t *pErrNo )
{
    volatile MkIoPortParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_IOPORT_FUNCID_OUT_WORD;
    param.errNo  = MK_IOPORT_ERR_NONE;
    param.ret    = MK_IOPORT_RET_FAILURE;
    param.portNo = portNo;
    param.pData  = pData;
    param.count  = count;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                 ),
                             "i" ( MK_CONFIG_INTNO_IOPORT )  );

    /* エラー番号設定要否判定 */
    if ( pErrNo != NULL ) {
        /* 必要 */

        /* エラー番号設定 */
        *pErrNo = param.errNo;
    }

    return param.ret;
}


/******************************************************************************/
