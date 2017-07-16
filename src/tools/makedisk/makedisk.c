/******************************************************************************/
/* src/tools/makedisk/makedisk.c                                              */
/*                                                                 2017/07/11 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* パーティションテーブル定義 */
#define PT_STATUS_BOOT          ( 0x80 )    /** ブート可能 */
#define PT_TYPE_MOCHI_BOOTER    ( 0x30 )    /** MochiBooter用パーティション */
#define PT_TYPE_MOCHI_KERNEL    ( 0x31 )    /** MochiKernel用パーティション */

/* 引数定義 */
#define OPTION_CYLINDER_DEFAULT ( 10 )      /** 引数シリンダ数デフォルト値 */
#define OPTION_HEAD_DEFAULT     ( 16 )      /** 引数ヘッド数デフォルト値   */
#define OPTION_SECTOR_DEFAULT   ( 63 )      /** 引数セクタ数デフォルト値   */

/* シリンダ定義 */
#define CYLINDER_MIN            (    0 )    /** シリンダ最小値 */
#define CYLINDER_MAX            ( 1023 )    /** シリンダ最大値 */

/* ヘッド定義 */
#define HEAD_MIN                (   0 )     /** セクタ最小値 */
#define HEAD_MAX                ( 254 )     /** セクタ最大値 */

/* セクタ定義 */
#define SECTOR_MIN              (  1 )      /** セクタ最小値 */
#define SECTOR_MAX              ( 63 )      /** セクタ最大値 */

/** バッファサイズ */
#define BUFFER_SIZE             ( 512 )     /** 読込みバッファサイズ */

/** アボートマクロ */
#define ABORT( ... )                    \
    {                                   \
        /* エラー出力 */                \
        fprintf( stderr, __VA_ARGS__ ); \
        fprintf( stderr, "\n" );        \
                                        \
        /* USAGE出力 */                 \
        printUsage( EXIT_FAILURE );     \
    }

/** CS設定マクロ */
#define SET_CYL_SEC( _CYLINDER, _SECTOR )   \
    ( ( ( _CYLINDER & 0x0300 ) << 6 ) |     \
      ( ( _SECTOR   & 0x003F ) << 8 ) |     \
      (   _CYLINDER & 0x00FF        )   )

/** CYLINDER取得マクロ */
#define GET_CYLINDER( _CYLSEC )     \
    ( ( ( _CYLSEC >> 6 ) & 0x0300 ) | ( _CYLSEC & 0x00FF ) )

/** SECTOR取得マクロ */
#define GET_SECTOR( _CYLSEC ) ( ( _CYLSEC >> 8 ) & 0x3F )

/** CHSアドレス */
typedef struct {
    uint16_t cylSec;            /**< シリンダ&セクタ */
    uint8_t  head;              /**< ヘッド          */
}  __attribute__( ( packed ) ) chs_t;

/** パーティションテーブル */
typedef struct {
    uint8_t  status;            /**< ステータス           */
    chs_t    chsFirstAddr;      /**< CHS先頭アドレス      */
    uint8_t  type;              /**< パーティションタイプ */
    chs_t    chsLastAddr;       /**< CHS最後尾アドレス    */
    uint32_t lbaFirstAddr;      /**< LBA先頭アドレス      */
    uint32_t lbaSize;           /**< LBAサイズ            */
} __attribute__( ( packed ) ) pt_t;

/** MBR */
typedef struct {
    uint8_t code[ 446 ];        /**< ブートストラップコード */
    pt_t    partitionTbl[ 4 ];  /**< パーティションテーブル */
    uint8_t signature[ 2 ];     /**< ブートシグネチャ       */
} __attribute__( ( packed ) ) mbr_t;


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
/* オプションチェック */
static void checkOptions( int32_t argNum,
                          char    *pArg[],
                          char    **ppDiskPath,
                          char    **ppIplPath,
                          char    **ppBootPath,
                          char    **ppKernelPath );
/* CHSアドレス取得 */
static chs_t getChs( uint32_t lba );

/* USAGE出力 */
static void printUsage( int status );

/* IPLバイナリ書込み */
static void writeIpl( int  diskFd,
                      char *pIplPath );

/* ブートローダバイナリ書込み */
static chs_t writeBoot( int   diskFd,
                        char  *pBootPath,
                        chs_t chsFirstAddr );

/* カーネルバイナリ書込み */
static chs_t writeKernel( int   diskFd,
                          char  *pKernelPath,
                          chs_t chsFirstAddr  );

/* パーティションエントリ書込み */
static void writePartitionEntry( int      diskFd,
                                 uint32_t no,
                                 pt_t     *pPe    );


/******************************************************************************/
/* グローバル変数定義                                                         */
/******************************************************************************/
uint32_t cylinder;  /* 仮想ディスクのシリンダ数 */
uint32_t head;      /* 仮想ディスクのヘッド数   */
uint32_t sector;    /* 仮想ディスクのセクタ数   */


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       makedisk
 * @details     仮想マシン用ディスクイメージを作成する。
 * 
 * param[in]    argNum  引数の数
 * param[in]    *pArg[] 引数
 * 
 * retval       EXIT_SUCCESS 正常終了
 * retval       EXIT_FAILURE 異常終了
 */
/******************************************************************************/
int main( int  argNum,
          char *pArg[] )
{
    int      diskFd;        /* 仮想ディスクファイルディスクリプタ */
    char     *pDiskPath;    /* 仮想ディスクパス                   */
    char     *pIplPath;     /* IPLバイナリパス                    */
    char     *pBootPath;    /* ブートローダバイナリパス           */
    char     *pKernelPath;  /* カーネルバイナリパス               */
    char     end;           /* 仮想ディスク最終バイト             */
    chs_t    chs;           /* CHSアドレス                        */
    off_t    offset;        /* オフセット                         */
    int32_t  size;          /* 書込みサイズ                       */
    
    /* 初期化 */
    end = 0;
    memset( &chs, 0, sizeof ( chs_t ) );
    
    /* オプションチェック */
    checkOptions( argNum,
                  pArg,
                  &pDiskPath,
                  &pIplPath,
                  &pBootPath,
                  &pKernelPath );
    
    /* 仮想ディスクオープン */
    diskFd = open( pDiskPath,
                   O_RDWR | O_CREAT | O_TRUNC,
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH );
    
    /* オープン結果判定 */
    if ( diskFd == -1 ) {
        /* 失敗 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't open %s. errno=%d.\n",
               __LINE__,
               pDiskPath,
               errno );
    }
    
    /* IPLバイナリ書込み */
    writeIpl( diskFd, pIplPath );
    
    /* ブートローダバイナリ書込み */
    chs.cylSec   = SET_CYL_SEC( 0, 1 );
    chs.head     = 1;
    chs = writeBoot( diskFd, pBootPath, chs );
    
    /* カーネルバイナリ書込み */
    chs.cylSec = SET_CYL_SEC( GET_CYLINDER( chs.cylSec ) + 1, 1 );
    chs.head   = 0;
    chs = writeKernel( diskFd, pKernelPath, chs );
    
    /* 仮想ディスクファイルサイズチェック */
    if ( GET_CYLINDER( chs.cylSec ) < cylinder ) {
        /* 指定サイズ未達 */
        
        /* 仮想ディスクシーク */
        offset = lseek( diskFd, cylinder * head * sector * 512 - 1, SEEK_SET );
        
        /* シーク結果判定 */
        if ( ( int32_t ) offset != ( cylinder * head * sector * 512 - 1 ) ) {
            /* 失敗 */
            
            /* アボート */
            ABORT( "ERROR(%04u): Can't seek at the disk image. ret=%d, errno=%d.\n",
                   __LINE__,
                   ( int32_t ) offset,
                   errno );
        }
        
        /* 書込み */
        size = ( int32_t ) write( diskFd, &end, 1 );
        
        /* 書込み結果判定 */
        if ( size != 1 ) {
            /* 失敗 */
            
            /* アボート */
            ABORT( "ERROR(%04u): Can't write %s. ret=%d, errno=%d.\n",
                   __LINE__,
                   pDiskPath,
                   size,
                   errno );
        }
        
    } else {
        /* 指定サイズ過達 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Overwrite.CHS=%u/%u/%u > %u/%u/%u.\n",
               __LINE__,
               GET_CYLINDER( chs.cylSec ),
               chs.head,
               GET_SECTOR( chs.cylSec ),
               cylinder,
               head,
               sector );
    }
    
    /* 仮想ディスククローズ */
    close( diskFd );
    
    /* 正常終了 */
    return EXIT_SUCCESS;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       オプションチェック
 * @details     オプションが許容可能な値かチェックする。
 * 
 * @param[in]   argNum        引数の数
 * @param[in]   *pArg[]       引数
 * @param[out]  *ppDiskPath   仮想ディスクのファイルパス
 * @param[out]  *ppIplPath    IPLバイナリのファイルパス
 * @param[out]  *ppBootPath   ブートローダバイナリのファイルパス
 * @param[out]  *ppKernelPath カーネルバイナリのファイルパス
 */
/******************************************************************************/
static void checkOptions( int32_t argNum,
                          char    *pArg[],
                          char    **ppDiskPath,
                          char    **ppIplPath,
                          char    **ppBootPath,
                          char    **ppKernelPath )
{
    char opt;   /* オプション文字 */
    
    /* 初期化 */
    *ppDiskPath   = NULL;                   /* 仮想ディスクパス         */
    *ppIplPath    = NULL;                   /* IPLバイナリパス          */
    *ppBootPath   = NULL;                   /* ブートローダバイナリパス */
    *ppKernelPath = NULL;                   /* カーネルバイナルパス     */
    cylinder      = OPTION_CYLINDER_DEFAULT;/* デフォルトシリンダ       */
    head          = OPTION_HEAD_DEFAULT;    /* デフォルトヘッド         */
    sector        = OPTION_SECTOR_DEFAULT;  /* デフォルトセクタ         */
    
    /* オプションが無くなるまで繰り返し */
    while ( true ) {
        /* オプション取得 */
        opt = getopt( argNum, pArg, "o:i:b:k:C:H:S:h" );
        
        /* 取得結果判定 */
        if ( opt == -1 ) {
            /* オプション無し */
            break;
            
        } else if ( opt == 'o' ) {
            /* 仮想ディスクパス */
            *ppDiskPath = optarg;
            
        } else if ( opt == 'i' ) {
            /* IPLバイナリパス */
            *ppIplPath = optarg;
            
        } else if ( opt == 'b' ) {
            /* ブートローダバイナリパス */
            *ppBootPath = optarg;
            
        } else if ( opt == 'k' ) {
            /* カーネルバイナリパス */
            *ppKernelPath = optarg;
            
        } else if ( opt == 'C' ) {
            /* シリンダ */
            cylinder = ( uint32_t ) atoi( optarg );
            
        } else if ( opt == 'H' ) {
            /* ヘッダ */
            head = ( uint32_t ) atoi( optarg );
            
        } else if ( opt == 'S' ) {
            /* セクタ */
            sector = ( uint32_t ) atoi( optarg );
            
        } else if ( opt == 'h' ) {
            /* ヘルプ */
            
            /* USAGE出力して終了 */
            printUsage( EXIT_SUCCESS );
            
        } else {
            /* 他 */
            
            /* アボート */
            ABORT( "ERROR(%04u): Unknown option!\n", __LINE__ );
        }
    }
    
    /* 仮想ディスクパス設定チェック */
    if ( *ppDiskPath == NULL ) {
        /* 未設定 */
        
        /* アボート */
        ABORT( "ERROR(%04u): No '-o' option!\n", __LINE__ );
    }
    
    /* IPLバイナリパス設定チェック */
    if ( *ppIplPath == NULL ) {
        /* 未設定 */
        
        /* アボート */
        ABORT( "ERROR(%04u): No '-i' option!\n", __LINE__ );
    }
    
    /* ブートローダバイナリパス設定チェック */
    if ( *ppBootPath == NULL ) {
        /* 未設定 */
        
        /* アボート */
        ABORT( "ERROR(%04u): No '-b' option!\n", __LINE__ );
    }
    
    /* カーネルバイナリパス設定チェック */
    if ( *ppKernelPath == NULL ) {
        /* 未設定 */
        
        /* アボート */
        ABORT( "ERROR(%04u): No '-k' option!\n", __LINE__ );
    }
    
    /* シリンダ値チェック */
    if ( !( ( CYLINDER_MIN <= cylinder     ) &&
            ( cylinder     <= CYLINDER_MAX )    ) ) {
        /* 範囲外 */
        
        /* アボート */
        ABORT( "ERROR(%04u): cylinder(%u) is out of range(%u-%u).\n",
               __LINE__,
               cylinder,
               CYLINDER_MIN,
               CYLINDER_MAX );
    }
    
    /* ヘッド値チェック */
    if ( !( ( HEAD_MIN <= head     ) &&
            ( head     <= HEAD_MAX )    ) ) {
        /* 範囲外 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Head(%u) is out of range(%u-%u).\n",
               __LINE__,
               head,
               HEAD_MIN,
               HEAD_MAX );
    }
    
    /* セクタ値チェック */
    if ( !( ( SECTOR_MIN <= sector     ) &&
            ( sector     <= SECTOR_MAX )    ) ) {
        /* 範囲外 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Sector(%u) is out of range(%u-%u).\n",
               __LINE__,
               sector,
               CYLINDER_MIN,
               CYLINDER_MAX );
    }
    
    return;
}


/******************************************************************************/
/**
 * @brief       CHSアドレス取得
 * @details     LBAアドレスをCHSアドレスに変換して取得する。
 * 
 * @param[in]   lba LBAアドレス
 * 
 * @return      CHSアドレス
 */
/******************************************************************************/
static chs_t getChs( uint32_t lba )
{
    chs_t chs;  /* CHSアドレス */
    
    /* 初期化 */
    memset( &chs, 0, sizeof ( chs_t ) );
    
    /* CHSアドレス設定 */
    chs.cylSec   = SET_CYL_SEC( ( lba / sector ) / head,
                                lba % sector + 1 );
    chs.head     = ( lba / sector ) % head;
    
    return chs;
}


/******************************************************************************/
/**
 * @brief       USAGE出力
 * @details     USAGEを出力しプログラムを終了する。
 * 
 * @param[in]   status 終了ステータス
 */
/******************************************************************************/
static void printUsage( int status )
{
    /* USAGE出力 */
    fprintf( stderr, "USAGE: makedisk -o [FILE] -b [FILE] -k [FILE] [OPTION]...\n"                                 );
    fprintf( stderr, "\n"                                                                                          );
    fprintf( stderr, "Option:\n"                                                                                   );
    fprintf( stderr, "  -o FILE   specify the output FILE that is a disk image.\n"                                 );
    fprintf( stderr, "  -i FILE   specify the input FILE that is a IPL binary.\n"                                  );
    fprintf( stderr, "  -b FILE   specify the input FILE that is a boot loader binary.\n"                          );
    fprintf( stderr, "  -k FILE   specify the input FILE that is a kernel binary.\n"                               );
    fprintf( stderr, "  -C NUMBER specify the NUMBER of cylinders.(default:%u)\n",         OPTION_CYLINDER_DEFAULT );
    fprintf( stderr, "  -H NUMBER specify the NUMBER of heads.(default:%u)\n",             OPTION_HEAD_DEFAULT     );
    fprintf( stderr, "  -S NUMBER specify the NUMBER of sectors per track.(default:%u)\n", OPTION_SECTOR_DEFAULT   );
    fprintf( stderr, "  -h        print help.\n"                                                                   );
    
    /* 終了 */
    exit( status );
}


/******************************************************************************/
/**
 * @brief       IPLバイナリ書込み
 * @details     IPLバイナリを仮想ディスクに書き込む。
 * 
 * @param[in]   diskFd    仮想ディスクファイルディスクリプタ
 * @param[in]   *pIplPath IPLバイナリのファイルパス
 */
/******************************************************************************/
static void writeIpl( int  diskFd,
                      char *pIplPath )
{
    int     iplFd;  /* IPLバイナリファイルディスクリプタ */
    mbr_t   mbr;    /* マスタブートレコード              */
    int32_t size;   /* サイズ                            */
    
    /* IPLバイナリファイルオープン */
    iplFd = open( pIplPath, O_RDONLY );
    
    /* オープン結果判定 */
    if ( iplFd == -1 ) {
        /* 失敗 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't open %s. errno=%d.\n",
               __LINE__,
               pIplPath,
               errno );
    }
    
    /* IPLバイナリ読込み */
    size = ( int32_t ) read( iplFd, &mbr, sizeof ( mbr_t ) );
    
    /* 読込み結果判定 */
    if ( size != ( sizeof ( mbr_t ) ) ) {
        /* 読込み失敗 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't read from %s. ret=%d, errno=%d.\n",
               __LINE__,
               pIplPath,
               size,
               errno );
    }
    
    /* IPLバイナリを仮想ディスクに書込み */
    size = ( int32_t ) write( diskFd, &mbr, sizeof ( mbr_t ) );
    
    /* 書込み結果判定 */
    if ( size != ( sizeof ( mbr_t ) ) ) {
        /* 書込み */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't write %s. ret=%d, errno=%d.\n",
               __LINE__,
               pIplPath,
               size,
               errno );
    }
    
    /* ファイルクローズ */
    close( iplFd );
    
    return;
}


/******************************************************************************/
/**
 * @brief       ブートローダバイナリ書込み
 * @details     ブートローダバイナリを仮想ディスクのパーティション1番に書込む。
 * 
 * @param[in]   diskFd       仮想ディスクファイルディスクリプタ
 * @param[in]   *pBootPath   ブートローダバイナリのファイルパス
 * @param[in]   chsFirstAddr 書込み先先頭CHSアドレス
 * 
 * @return      書込み先最後尾CHSアドレス
 */
/******************************************************************************/
static chs_t writeBoot( int   diskFd,
                        char  *pBootPath,
                        chs_t chsFirstAddr )
{
    int      bootFd;                /* ブートローダファイルディスクリプタ */
    pt_t     pe;                    /* パーティションエントリ             */
    char     buffer[ BUFFER_SIZE ]; /* 読込みバッファ                     */
    off_t    offset;                /* ファイルオフセット                 */
    int32_t  readSize;              /* 読込サイズ                         */
    int32_t  writeSize;             /* 書込みサイズ                       */
    uint32_t fileSize;              /* ファイルサイズ                     */
    uint32_t lbaSize;               /* ファイルサイズ（セクタ数）         */
    uint32_t lbaFirstAddr;          /* 書込み先先頭LBAアドレス            */
    
    /* 初期化 */
    fileSize = 0;
    lbaSize  = 0;
    lbaFirstAddr = GET_CYLINDER( chsFirstAddr.cylSec ) * head * sector +
                   chsFirstAddr.head * sector + 
                   GET_SECTOR( chsFirstAddr.cylSec ) - 1;
    memset( &pe, 0, sizeof ( pt_t ) );
    
    /* 仮想ディスクシーク */
    offset = lseek( diskFd, lbaFirstAddr * 512, SEEK_SET );
    
    /* シーク結果判定 */
    if ( ( int32_t ) offset != ( lbaFirstAddr * 512 ) ) {
        /* 失敗 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't seek at the disk image. ret=%d, errno=%d.\n",
               __LINE__,
               ( int32_t ) offset,
               errno );
    }
    
    /* ブートローダバイナリファイルオープン */
    bootFd = open( pBootPath, O_RDONLY );
    
    /* オープン結果判定 */
    if ( bootFd == -1 ) {
        /* 失敗 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't open %s. errno=%d.\n",
               __LINE__,
               pBootPath,
               errno );
    }
    
    /* 読み書きバッファサイズ毎に繰り返し */
    do {
        /* バッファ初期化 */
        memset( buffer, 0, BUFFER_SIZE );
        
        /* ブートローダバイナリ読込み */
        readSize = read( bootFd, buffer, BUFFER_SIZE );
        
        /* 読込み結果判定 */
        if ( readSize == -1 ) {
            /* 失敗 */
            
            /* アボート */
            ABORT( "ERROR(%04u): Can't read from %s. errno=%d.\n",
                   __LINE__,
                   pBootPath,
                   errno );
            
        } else if ( readSize == 0 ) {
            /* EOF */
            
            break;
        }
        
        /* 書込み */
        writeSize = write( diskFd, buffer, readSize );
        
        /* 書込み結果判定 */
        if ( writeSize != readSize ) {
            /* 失敗 */
            
            /* アボート */
            ABORT( "ERROR(%04u): Can't write %s. errno=%d.\n",
                   __LINE__,
                   pBootPath,
                   errno );
        }
        
        /* ファイルサイズ更新 */
        fileSize += writeSize;
        lbaSize++;
    } while ( writeSize == BUFFER_SIZE );
    
    /* パーティションテーブル設定 */
    pe.status       = PT_STATUS_BOOT;
    pe.chsFirstAddr = chsFirstAddr;
    pe.type         = PT_TYPE_MOCHI_BOOTER;
    pe.chsLastAddr  = getChs( lbaFirstAddr + lbaSize - 1 );
    pe.lbaFirstAddr = lbaFirstAddr;
    pe.lbaSize      = lbaSize;
    writePartitionEntry( diskFd, 1, &pe );
    
    /* ファイルクローズ */
    close( bootFd );
    
    return pe.chsLastAddr;
}


/******************************************************************************/
/**
 * @brief       カーネルバイナリ書込み
 * @details     カーネルバイナリを仮想ディスクのパーティション2番に書込む。
 * 
 * @param[in]   diskFd       仮想ディスクファイルディスクリプタ
 * @param[in]   *pKernelPath カーネルバイナリのファイルパス
 * @param[in]   chsFirstAddr 書込み先先頭CHSアドレス
 * 
 * @return      書込み先最後尾CHSアドレス
 */
/******************************************************************************/
static chs_t writeKernel( int   diskFd,
                          char  *pKernelPath,
                          chs_t chsFirstAddr  )
{
    int      kernelFd;              /* カーネルファイルディスクリプタ */
    pt_t     pe;                    /* パーティションエントリ         */
    char     buffer[ BUFFER_SIZE ]; /* 読込みバッファ                 */
    off_t    offset;                /* ファイルオフセット             */
    int32_t  readSize;              /* 読込サイズ                     */
    int32_t  writeSize;             /* 書込みサイズ                   */
    uint32_t fileSize;              /* ファイルサイズ                 */
    uint32_t lbaSize;               /* ファイルサイズ（セクタ数）     */
    uint32_t lbaFirstAddr;          /* 書込み先先頭LBAアドレス        */
    
    /* 初期化 */
    fileSize     = 0;
    lbaSize      = 0;
    lbaFirstAddr = GET_CYLINDER( chsFirstAddr.cylSec ) * head * sector +
                   chsFirstAddr.head * sector + 
                   GET_SECTOR( chsFirstAddr.cylSec ) - 1;
    memset( &pe, 0, sizeof ( pt_t ) );
    
    /* 仮想ディスクシーク */
    offset = lseek( diskFd, lbaFirstAddr * 512, SEEK_SET );
    
    /* シーク結果判定 */
    if ( ( int32_t ) offset != ( lbaFirstAddr * 512 ) ) {
        /* 失敗 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't seek at the disk image. ret=%d, errno=%d.\n",
               __LINE__,
               ( int32_t ) offset,
               errno );
    }
    
    /* カーネルバイナリファイルオープン */
    kernelFd = open( pKernelPath, O_RDONLY );
    
    /* オープン結果判定 */
    if ( kernelFd == -1 ) {
        /* 失敗 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't open %s. errno=%d.\n",
               __LINE__,
               pKernelPath,
               errno );
    }
    
    /* 読み書きバッファサイズ毎に繰り返し */
    do {
        /* バッファ初期化 */
        memset( buffer, 0, BUFFER_SIZE );
        
        /* ブートローダバイナリ読込み */
        readSize = read( kernelFd, buffer, BUFFER_SIZE );
        
        /* 読込み結果判定 */
        if ( readSize == -1 ) {
            /* 失敗 */
            
            /* アボート */
            ABORT( "ERROR(%04u): Can't read from %s. errno=%d.\n",
                   __LINE__,
                   pKernelPath,
                   errno );
            
        } else if ( readSize == 0 ) {
            /* EOF */
            
            break;
        }
        
        /* 書込み */
        writeSize = write( diskFd, buffer, readSize );
        
        /* 書込み結果判定 */
        if ( writeSize != readSize ) {
            /* 失敗 */
            
            /* アボート */
            ABORT( "ERROR(%04u): Can't write %s. errno=%d.\n",
                   __LINE__,
                   pKernelPath,
                   errno );
        }
        
        /* ファイルサイズ更新 */
        fileSize += writeSize;
        lbaSize++;
    } while ( writeSize == BUFFER_SIZE );
    
    /* パーティションテーブル設定 */
    pe.status       = 0;
    pe.chsFirstAddr = chsFirstAddr;
    pe.type         = PT_TYPE_MOCHI_KERNEL;
    pe.chsLastAddr  = getChs( lbaFirstAddr + lbaSize - 1 );
    pe.lbaFirstAddr = lbaFirstAddr;
    pe.lbaSize      = lbaSize;
    writePartitionEntry( diskFd, 2, &pe );
    
    /* ファイルクローズ */
    close( kernelFd );
    
    return pe.chsLastAddr;
}


/******************************************************************************/
/**
 * @brief       パーティションエントリ書込み
 * @details     パーティションテーブルのエントリを仮想ディスクに書き込む。
 * 
 * @param[in]   diskFd 仮想ディスクファイルディスクリプタ
 * @param[in]   no     パーティション番号
 * @param[in]   *pPe   パーティションエントリ
 */
/******************************************************************************/
static void writePartitionEntry( int      diskFd,
                                 uint32_t no,
                                 pt_t     *pPe    )
{
    int     iplFd;  /* IPLバイナリファイルディスクリプタ */
    mbr_t   mbr;    /* マスタブートレコード              */
    off_t   offset; /* ファイルオフセット                */
    int32_t size;   /* サイズ                            */
    
    /* 仮想ディスクシーク */
    offset = lseek( diskFd, 0, SEEK_SET );
    
    /* シーク結果判定 */
    if ( ( int32_t ) offset != 0 ) {
        /* 失敗 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't seek at the disk image. ret=%d, errno=%d.\n",
               __LINE__,
               ( int32_t ) offset,
               errno );
    }
    
    /* MBR読込み */
    size = ( int32_t ) read( diskFd, &mbr, sizeof ( mbr_t ) );
    
    /* 読込み結果判定 */
    if ( size != ( sizeof ( mbr_t ) ) ) {
        /* 読込み失敗 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't read from MBR. ret=%d, errno=%d.\n",
               __LINE__,
               size,
               errno );
    }
    
    /* パーティションエントリ設定 */
    mbr.partitionTbl[ no - 1 ] = *pPe;
    
    /* 仮想ディスクシーク */
    offset = lseek( diskFd, 0, SEEK_SET );
    
    /* シーク結果判定 */
    if ( ( int32_t ) offset != 0 ) {
        /* 失敗 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't seek at the disk image. ret=%d, errno=%d.\n",
               __LINE__,
               ( int32_t ) offset,
               errno );
    }
    
    /* MBR書込み */
    size = ( int32_t ) write( diskFd, &mbr, sizeof ( mbr_t ) );
    
    /* 書込み結果判定 */
    if ( size != ( sizeof ( mbr_t ) ) ) {
        /* 書込み */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't write MBR. ret=%d, errno=%d.\n",
               __LINE__,
               size,
               errno );
    }
    
    return;
}


/******************************************************************************/
