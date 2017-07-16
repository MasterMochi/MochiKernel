/******************************************************************************/
/* src/tools/makeimg/makeimg.c                                                */
/*                                                                 2017/07/16 */
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

/* 読込バッファサイズ */
#define BUFFER_SIZE ( 512 )


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
/* ファイル追加 */
static void addFile( int  imgFd,
                     char *pPath );

/* オプションチェック */
static void checkOptions( int32_t argNum,
                          char    *pArg[] );

/* ファイル名取得 */
static void getFileName( char     *pFileName,
                         char     *pFilePath,
                         uint32_t length      );

/* USAGE出力 */
static void printUsage( int status );


/******************************************************************************/
/* グローバル変数定義                                                         */
/******************************************************************************/
/* オプション */
char    *gpImgPath;     /* 出力先イメージファイルパス */
uint8_t gFileType;      /* 入力ファイルタイプ         */


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       makeimg
 * @details     カーネルイメージを作成する。
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
    int imgFd;  /* イメージファイルディスクリプタ */
    int flags;  /* オープンフラグ                 */
    
    /* オプションチェック */
    checkOptions( argNum, pArg );
    
    /* ファイルタイプチェック */
    if ( gFileType == MOCHIKERNEL_PROCESS_TYPE_KERNEL ) {
        /* カーネル */
        
        /* オープンフラグ設定 */
        flags = O_RDWR | O_CREAT | O_TRUNC;
        
    } else {
        /* カーネル以外 */
        
        /* オープンフラグ設定 */
        flags = O_RDWR;
    }
    
    /* イメージオープン */
    imgFd = open( gpImgPath,
                  flags,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH );
    
    /* オープン結果判定 */
    if ( imgFd == -1 ) {
        /* 失敗 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't open %s. errno=%d.\n",
               __LINE__,
               gpImgPath,
               errno );
    }
    
    /* ファイル書込み */
    for ( ; optind < argNum; optind++ ) {
        addFile( imgFd, pArg[ optind ] );
    }
    
    /* 仮想ディスククローズ */
    close( imgFd );
    
    /* 正常終了 */
    return EXIT_SUCCESS;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ファイル追加
 * @details     イメージファイルにファイルを追加する
 * 
 * @param[in]   imgFd  イメージファイルディスクリプタ
 * @param[in]   *pPath ファイルパス
 */
/******************************************************************************/
static void addFile( int  imgFd,
                     char *pPath )
{
    int                 fd;                     /* ファイルディスクリプタ */
    off_t               offset;                 /* ファイルオフセット     */
    off_t               headerOffset;           /* ヘッダオフセット       */
    ssize_t             readSize;               /* 読込みサイズ           */
    ssize_t             writeSize;              /* 書込みサイズ           */
    uint8_t             buffer[ BUFFER_SIZE ];  /* バッファ               */
    uint32_t            size;                   /* ファイルサイズ         */
    MochiKernelImgHdr_t header;                 /* ファイルヘッダ         */
    
    /* 初期化 */
    size = 0;
    memset( &header, 0, sizeof ( MochiKernelImgHdr_t ) );
    
    /* イメージファイルシーク */
    offset = lseek( imgFd, 512, SEEK_END );
    
    /* シーク結果判定 */
    if ( ( ( ( int32_t ) offset       ) <  0 ) &&
         ( ( ( int32_t ) offset % 512 ) != 0 )    ) {
        /* 失敗または異常 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't seek at the image. ret=%d, errno%d.\n",
               __LINE__,
               ( int32_t ) offset,
               errno );
    }
    
    /* ヘッダオフセット設定 */
    headerOffset = offset - ( off_t ) sizeof ( MochiKernelImgHdr_t );
    
    /* ファイルオープン */
    fd = open( pPath, O_RDONLY );
    
    /* オープン結果判定 */
    if ( fd == -1 ) {
        /* 失敗 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't open %s. errno=%d.\n",
               __LINE__,
               pPath,
               errno );
    }
    
    /* バッファサイズ毎に繰り返し */
    do {
        /* バッファ初期化 */
        memset( buffer, 0, BUFFER_SIZE );
        
        /* ファイル読込 */
        readSize = read( fd, buffer, BUFFER_SIZE );
        
        /* 読込結果判定 */
        if ( readSize == -1 ) {
            /* 失敗 */
            
            /* アボート */
            ABORT( "ERROR(%04u): Can't read from %s. errno=%d.\n",
                   __LINE__,
                   pPath,
                   errno );
            
        } else if ( readSize == 0 ) {
            /* EOF */
            
            break;
        }
        
        /* ファイルサイズ更新 */
        size += ( uint32_t ) readSize;
        
        /* 書込み */
        writeSize = write( imgFd, buffer, MLIB_BASIC_ALIGN( readSize, 512 ) );
        
        /* 書込み結果判定 */
        if ( writeSize != MLIB_BASIC_ALIGN( readSize, 512 ) ) {
            /* 失敗 */
            
            /* アボート */
            ABORT( "ERROR(%04u): Can't write %s. ret=%d, errno=%d.\n",
                   __LINE__,
                   pPath,
                   ( int32_t ) writeSize,
                   errno );
        }
        
    } while ( readSize == BUFFER_SIZE );
    
    /* ファイルヘッダ設定 */
    getFileName( header.fileName, pPath, sizeof ( header.fileName ) - 1 );
    header.fileSize = size;
    header.fileType = gFileType;
    
    /* イメージファイルシーク */
    offset = lseek( imgFd, headerOffset, SEEK_SET );
    
    /* シーク結果判定 */
    if ( offset != headerOffset ) {
        /* 失敗または異常 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't seek at the image. ret=%d, errno%d.\n",
               __LINE__,
               ( int32_t ) offset,
               errno );
    }
    
    /* ファイルヘッダ書込み */
    writeSize = write( imgFd, &header, sizeof ( MochiKernelImgHdr_t ) );
    
    /* 書込み結果判定 */
    if ( writeSize != sizeof ( MochiKernelImgHdr_t ) ) {
        /* 失敗 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't write %s. ret=%d, errno=%d.\n",
               __LINE__,
               pPath,
               ( int32_t ) writeSize,
               errno );
    }
    
    /* ファイルクローズ */
    close( fd );
    
    return;
}


/******************************************************************************/
/**
 * @brief       オプションチェック
 * @details     オプションが許容可能な値かチェックする。
 * 
 * @param[in]   argNum  引数の数
 * @param[in]   *pArg[] 引数
 */
/******************************************************************************/
static void checkOptions( int32_t argNum,
                          char    *pArg[] )
{
    char opt;   /* オプション文字 */
    
    /* 初期化 */
    gpImgPath = NULL;
    gFileType = MOCHIKERNEL_PROCESS_TYPE_NONE;
    
    /* オプションが無くなるまで繰り返し */
    while ( true ) {
        /* オプション取得 */
        opt = getopt( argNum, pArg, "o:KDSUh" );
        
        /* 取得結果判定 */
        if ( opt == -1 ) {
            /* オプション無し */
            break;
            
        } else if ( opt == 'o' ) {
            /* 出力先ファイルパス */
            gpImgPath = optarg;
            
        } else if ( opt == 'K' ) {
            /* カーネルファイル入力 */
            gFileType = MOCHIKERNEL_PROCESS_TYPE_KERNEL;
            
        } else if ( opt == 'D' ) {
            /* ドライバファイル入力 */
            gFileType = MOCHIKERNEL_PROCESS_TYPE_DRIVER;
            
        } else if ( opt == 'S' ) {
            /* サーバファイル入力 */
            gFileType = MOCHIKERNEL_PROCESS_TYPE_SERVER;
            
        } else if ( opt == 'U' ) {
            /* ユーザファイル入力 */
            gFileType = MOCHIKERNEL_PROCESS_TYPE_USER;
            
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
    
    /* 出力先ファイルパス設定チェック */
    if ( gpImgPath == NULL ) {
        /* 未設定 */
        
        /* アボート */
        ABORT( "ERROR(%04u): No '-o' option!\n", __LINE__ );
    }
    
    /* 入力ファイルタイプ設定チェック */
    if ( gFileType == MOCHIKERNEL_PROCESS_TYPE_NONE ) {
        /* 未設定 */
        
        /* アボート */
        ABORT( "ERROR(%04u): No set file type! ( '-K', '-D', '-S' or '-U' )\n",
               __LINE__ );
    }
    
    return;
}


/******************************************************************************/
/**
 * @brief       ファイル名取得
 * @details     ファイルパスからファイル名を取得する。
 * 
 * @param[out]  *pFileName ファイル名
 * @param[in]   *pFilePath ファイルパス
 * @param[in]   length     ファイル名長
 */
/******************************************************************************/
static void getFileName( char     *pFileName,
                         char     *pFilePath,
                         uint32_t length      )
{
    char *pStr1;
    char *pStr2;
    
    /* 初期化 */
    pStr1 = pFilePath;
    pStr2 = NULL;
    
    /* 最終分離符検索 */
    do {
        /* 分離符位置バックアップ */
        pStr2 = pStr1;
        pStr2++;
        
        /* 分離符検索 */
        pStr1 = strstr( pStr2, "/" );
        
    } while ( pStr1 != NULL );
    
    /* 検索結果判定 */
    if ( pStr2 == NULL ) {
        /* 失敗 */
        
        /* アボート */
        ABORT( "ERROR(%04u): Can't search!", __LINE__ );
    }
    
    /* ファイル名格納 */
    strncpy( pFileName, pStr2, length );
    
    return;
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
    fprintf( stderr, "USAGE: makedisk -o [FILE] { -K | -D | -S | -U } [FILE]...\n"              );
    fprintf( stderr, "\n"                                                                       );
    fprintf( stderr, "Option:\n"                                                                );
    fprintf( stderr, "  -o FILE specify the output FILE that is a kernel image.\n"              );
    fprintf( stderr, "  -K FILE specify the input FILE type that is a kernel binary.\n"         );
    fprintf( stderr, "  -D FILE specify the input FILE type that is a driver process binary.\n" );
    fprintf( stderr, "  -S FILE specify the input FILE type that is a server process binary.\n" );
    fprintf( stderr, "  -U FILE specify the input FILE type that is a user process binary.\n"   );
    fprintf( stderr, "  -h      print help.\n"                                                  );
    
    /* 終了 */
    exit( status );
}


/******************************************************************************/
