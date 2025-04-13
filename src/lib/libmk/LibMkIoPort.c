/******************************************************************************/
/*                                                                            */
/* src/lib/libmk/LibMkIoPort.c                                                */
/*                                                                 2025/04/06 */
/* Copyright (C) 2018-2025 Mochi.                                             */
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
#include <kernel/ioport.h>


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief           I/Oポートバルク入出力
 * @details         I/Oポート入出力をまとめて行う。
 *
 * @param[in,out]   *pData バルクデータ
 * @param[out]      *pErr  エラー内容
 *                      - MK_ERR_NONE         エラー無し
 *                      - MK_ERR_UNAUTHORIZED 権限無し
 *                      - MK_ERR_PARAM        パラメータ不正
 *
 * @return          処理結果を返す。
 * @retval          MK_RET_SUCCESS 成功
 * @retval          MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkIoPortBulk( MkIoPortBulk_t *pData,
                         MkErr_t        *pErr   )
{
    volatile MkIoPortParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_IOPORT_FUNCID_BULK;
    param.ret    = MK_RET_FAILURE;
    param.err    = MK_ERR_NONE;
    param.pData  = pData;

    /* カーネルコール */
    __asm__ __volatile__ ( "int %1"
                           :
                           : "S" ( &param          ),   /* input: esi */
                             "i" ( MK_IOPORT_INTNO )
                           : "memory"                 );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       I/Oポート入力(1byte)
 * @details     指定したポート番号から1byteのデータを指定回数分入力する。
 *
 * @param[in]   portNo ポート番号
 * @param[out]  *pData データ格納先
 * @param[in]   count  読込み回数
 * @param[out]  *pErr  エラー内容
 *                  - MK_ERR_NONE         エラー無し
 *                  - MK_ERR_UNAUTHORIZED 権限無し
 *
 * @return      処理結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkIoPortInByte( uint16_t portNo,
                           void     *pData,
                           size_t   count,
                           MkErr_t  *pErr   )
{
    volatile MkIoPortParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_IOPORT_FUNCID_IN_BYTE;
    param.ret    = MK_RET_FAILURE;
    param.err    = MK_ERR_NONE;
    param.portNo = portNo;
    param.pData  = pData;
    param.count  = count;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param          ),
                             "i" ( MK_IOPORT_INTNO )
                           : "esi"                    );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       I/Oポート入力(4byte)
 * @details     指定したポート番号から4byteのデータを指定回数分入力する。
 *
 * @param[in]   portNo ポート番号
 * @param[out]  *pData データ格納先
 * @param[in]   count  読込み回数
 * @param[out]  *pErr  エラー内容
 *                  - MK_ERR_NONE         エラー無し
 *                  - MK_ERR_UNAUTHORIZED 権限無し
 *
 * @return      処理結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkIoPortInDWord( uint16_t portNo,
                            void     *pData,
                            size_t   count,
                            MkErr_t  *pErr   )
{
    volatile MkIoPortParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_IOPORT_FUNCID_IN_DWORD;
    param.ret    = MK_RET_FAILURE;
    param.err    = MK_ERR_NONE;
    param.portNo = portNo;
    param.pData  = pData;
    param.count  = count;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param          ),
                             "i" ( MK_IOPORT_INTNO )
                           : "esi"                    );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       I/Oポート入力(2byte)
 * @details     指定したポート番号から2byteのデータを指定回数分入力する。
 *
 * @param[in]   portNo ポート番号
 * @param[out]  *pData データ格納先
 * @param[in]   count  読込み回数
 * @param[out]  *pErr  エラー内容
 *                  - MK_ERR_NONE         エラー無し
 *                  - MK_ERR_UNAUTHORIZED 権限無し
 *
 * @return      処理結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkIoPortInWord( uint16_t portNo,
                           void     *pData,
                           size_t   count,
                           MkErr_t  *pErr   )
{
    volatile MkIoPortParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_IOPORT_FUNCID_IN_WORD;
    param.ret    = MK_RET_FAILURE;
    param.err    = MK_ERR_NONE;
    param.portNo = portNo;
    param.pData  = pData;
    param.count  = count;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param          ),
                             "i" ( MK_IOPORT_INTNO )
                           : "esi"                    );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       I/Oポート出力(1byte)
 * @details     指定したポート番号に1byteのデータを指定回数分出力する。
 *
 * @param[in]   portNo ポート番号
 * @param[out]  *pData データ格納先
 * @param[in]   count  読込み回数
 * @param[out]  *pErr  エラー内容
 *                  - MK_ERR_NONE         エラー無し
 *                  - MK_ERR_UNAUTHORIZED 権限無し
 *
 * @return      処理結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkIoPortOutByte( uint16_t portNo,
                            void     *pData,
                            size_t   count,
                            MkErr_t  *pErr   )
{
    volatile MkIoPortParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_IOPORT_FUNCID_OUT_BYTE;
    param.ret    = MK_RET_FAILURE;
    param.err    = MK_ERR_NONE;
    param.portNo = portNo;
    param.pData  = pData;
    param.count  = count;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param          ),
                             "i" ( MK_IOPORT_INTNO )
                           : "esi"                    );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       I/Oポート出力(4byte)
 * @details     指定したポート番号に4byteのデータを指定回数分出力する。
 *
 * @param[in]   portNo ポート番号
 * @param[out]  *pData データ格納先
 * @param[in]   count  読込み回数
 * @param[out]  *pErr  エラー内容
 *                  - MK_ERR_NONE         エラー無し
 *                  - MK_ERR_UNAUTHORIZED 権限無し
 *
 * @return      処理結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkIoPortOutDWord( uint16_t portNo,
                             void     *pData,
                             size_t   count,
                             MkErr_t  *pErr   )
{
    volatile MkIoPortParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_IOPORT_FUNCID_OUT_DWORD;
    param.ret    = MK_RET_FAILURE;
    param.err    = MK_ERR_NONE;
    param.portNo = portNo;
    param.pData  = pData;
    param.count  = count;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param          ),
                             "i" ( MK_IOPORT_INTNO )
                           : "esi"                    );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       I/Oポート出力(2byte)
 * @details     指定したポート番号に2byteのデータを指定回数分出力する。
 *
 * @param[in]   portNo ポート番号
 * @param[out]  *pData データ格納先
 * @param[in]   count  読込み回数
 * @param[out]  *pErr  エラー内容
 *                  - MK_ERR_NONE         エラー無し
 *                  - MK_ERR_UNAUTHORIZED 権限無し
 *
 * @return      処理結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkIoPortOutWord( uint16_t portNo,
                            void     *pData,
                            size_t   count,
                            MkErr_t  *pErr   )
{
    volatile MkIoPortParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_IOPORT_FUNCID_OUT_WORD;
    param.ret    = MK_RET_FAILURE;
    param.err    = MK_ERR_NONE;
    param.portNo = portNo;
    param.pData  = pData;
    param.count  = count;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param          ),
                             "i" ( MK_IOPORT_INTNO )
                           : "esi"                    );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    return param.ret;
}


/******************************************************************************/
