/******************************************************************************/
/* src/include/hardware/ATA/ATA.h                                             */
/*                                                                 2018/11/24 */
/* Copyright (C) 2017-2018 Mochi.                                             */
/******************************************************************************/
#ifndef ATA_H
#define ATA_H
/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* PITポート定義 */
#define ATA_PORT_DATA       ( 0x01F0 )  /**< Data レジスタ             */
#define ATA_PORT_ERROR      ( 0x01F1 )  /**< Error レジスタ            */
#define ATA_PORT_FEATURES   ( 0x01F1 )  /**< Features レジスタ         */
#define ATA_PORT_SECTOR_CNT ( 0x01F2 )  /**< Sector Count レジスタ     */
#define ATA_PORT_SECTOR_NUM ( 0x01F3 )  /**< Sector Number レジスタ    */
#define ATA_PORT_CYL_LOW    ( 0x01F4 )  /**< Cylinder Low レジスタ     */
#define ATA_PORT_CYL_HIGH   ( 0x01F5 )  /**< Cylinder High レジスタ    */
#define ATA_PORT_DEV_HEAD   ( 0x01F6 )  /**< Device/Head レジスタ      */
#define ATA_PORT_STATUS     ( 0x01F7 )  /**< Status レジスタ           */
#define ATA_PORT_COMMAND    ( 0x01F7 )  /**< Command レジスタ          */
#define ATA_PORT_ALT_STATUS ( 0x03F6 )  /**< Alternate Status レジスタ */
#define ATA_PORT_DEV_CTRL   ( 0x03F6 )  /**< Device Control レジスタ   */

/* Status / Alternate Status レジスタビット定義 */
#define ATA_STATUS_BSY      ( 0x80 )    /**< ビジー               */
#define ATA_STATUS_DRDY     ( 0x40 )    /**< デバイスレディ       */
#define ATA_STATUS_DF       ( 0x20 )    /**< デバイス障害         */
#define ATA_STATUS_DSC      ( 0x10 )    /**< デバイスシーク完了   */
#define ATA_STATUS_DRQ      ( 0x08 )    /**< データ転送要求       */
#define ATA_STATUS_CORR     ( 0x04 )    /**< 修正可能データエラー */
#define ATA_STATUS_IDX      ( 0x02 )    /**< Index（ベンダ特有）  */
#define ATA_STATUS_ERR      ( 0x01 )    /**< エラー発生           */

/* Error レジスタビット定義 */
#define ATA_ERROR_UNC       ( 0x40 )    /**< 修正不可能データエラー */
#define ATA_ERROR_MC        ( 0x20 )    /**< メディア交換           */
#define ATA_ERROR_IDNF      ( 0x10 )    /**< セクタID不検知         */
#define ATA_ERROR_MCR       ( 0x08 )    /**< メディア交換要求       */
#define ATA_ERROR_ABRT      ( 0x04 )    /**< コマンドアボート       */
#define ATA_ERROR_TK0NF     ( 0x02 )    /**< Track0不検知           */
#define ATA_ERROR_AMNF      ( 0x01 )    /**< アドレスマーク不検知   */

/* Device/Head レジスタビット定義 */
#define ATA_DEV_HEAD_LBA    ( 0x40 )    /**< LBA/CHSモード    */
#define ATA_DEV_HEAD_DEV    ( 0x10 )    /**< デバイスアドレス */
#define ATA_DEV_HEAD_HS3    ( 0x08 )    /**< ヘッド番号       */
#define ATA_DEV_HEAD_HS2    ( 0x04 )    /**< ヘッド番号       */
#define ATA_DEV_HEAD_HS1    ( 0x02 )    /**< ヘッド番号       */
#define ATA_DEV_HEAD_HS0    ( 0x01 )    /**< ヘッド番号       */
#define ATA_DEV_HEAD_LBA4   ( 0x0F )    /**< LBA[27:24]       */

/* Device Control レジスタビット定義 */
#define ATA_DEV_CTRL_SRST   ( 0x04 )    /**< ソフトウェアリセット */
#define ATA_DEV_CTRL_NIEN   ( 0x02 )    /**< デバイス割込み許可   */

/* Sector Count レジスタ定義 */
#define ATA_SECTOR_CNT_MAX  ( 256 )     /**< セクタカウント最大値 */

/* Data レジスタ定義 */
#define ATA_TRANSFER_COUNT  ( 256 )     /**< データ転送回数 */

/* コマンド */
#define ATA_CMD_READ_SECTOR ( 0x20 )    /**< READ SECTOR(S)コマンド */

/** LBA->SectorNumberレジスタ変換マクロ */
#define ATA_LBA_TO_SECTOR_NUM( _LBA ) ( (   _LBA         ) & 0xFF )
/** LBA->CylinderLowレジスタ変換マクロ */
#define ATA_LBA_TO_CYL_LOW( _LBA )    ( ( ( _LBA ) >>  8 ) & 0xFF )
/** LBA->CylinderHighレジスタ変換マクロ */
#define ATA_LBA_TO_CYL_HIGH( _LBA )   ( ( ( _LBA ) >> 16 ) & 0xFF )
/** LBA->Device/Headレジスタ変換マクロ */
#define ATA_LBA_TO_DEV_HEAD( _LBA )   ( ( ( _LBA ) >> 24 ) & 0x0F )


/******************************************************************************/
#endif
