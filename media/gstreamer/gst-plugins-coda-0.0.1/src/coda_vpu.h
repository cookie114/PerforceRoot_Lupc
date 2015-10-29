/* GStreamer
 * Copyright (C)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef CODA_VPU_H
#define CODA_VPU_H

#include "hal_arch.h"
#include "coda_reg.h"

G_BEGIN_DECLS

#define USE_SECOND_AXI

typedef guint PhysicalAddress;

#define FWVERSION			120

#define MAX_FRAME_BASE			0x2000000                   // 32 MB
#define INST_FRAME_BUF_SIZE		0x400000

#define MAX_FRAME_MVL_STRIDE		2048
#define MAX_FRAME_MVL_HEIGHT		1088

#define CODE_BUF_SIZE				(132 * 1024)//(112 * 1024)
#define FMO_SLICE_SAVE_BUF_SIZE		32
#define WORK_BUF_SIZE				(512 * 1024) + ( FMO_SLICE_SAVE_BUF_SIZE * 1024 * 8 )
#define PARA_BUF2_SIZE				( 2 * 1024 )
#define PARA_BUF_SIZE				(10 * 1024)
#define TOTAL_VPUBUF_SIZE              0x200000

#define BWB_ENABLE                   0

#define SECOND_AXI_BASE_ADDR         0

#define	DBKY_INTERNAL_BUF_SIZE		0x4000	// DBKY_SIZE = MAX_WIDTH*16
#define	DBKC_INTERNAL_BUF_SIZE		0x4000	// DBKC_SIZE = MAX_WIDTH*16
#define	BIT_INTERNAL_BUF_SIZE		0x4000	// BIT_SIZE  = MAX_WIDTH/16*128
#define IPACDC_INTERNAL_BUF_SIZE	0x4000	// IP_SIZE   = MAX_WIDTH/16*128
#define	OVL_INTERNAL_BUF_SIZE		0x2800	// OVL_SIZE  = MAX_WIDTH*5

#define	DBKY_INTERNAL_BUF_SIZE_MP_VC1		0x2800	// DBKY_SIZE = MAX_WIDTH*8
#define	DBKC_INTERNAL_BUF_SIZE_MP_VC1		0x2800	// DBKC_SIZE = MAX_WIDTH*8
#define DBK_INTERNAL_BUF_SIZE_MP_VC1_HD   0x4000
#define	DBKY_INTERNAL_BUF_SIZE_AP_VC1		0x5000	// DBKY_SIZE = 1280*16
#define	DBKC_INTERNAL_BUF_SIZE_AP_VC1		0x5000	// DBKC_SIZE = 1280*16
#define DBK_INTERNAL_BUF_SIZE_AP_VC1_HD   0x8000
#define	BIT_INTERNAL_BUF_SIZE_VC1		0x6000	// BIT_SIZE  = MAX_WIDTH/16*128
#define IPACDC_INTERNAL_BUF_SIZE_VC1	0x3000	// IP_SIZE   = MAX_WIDTH/16*128
#define	OVL_INTERNAL_BUF_SIZE_VC1		0x2000	// OVL_SIZE  = MAX_WIDTH*5

#define SLICE_BUF_SIZE		0x040000

#define PS_BUF_SIZE			0x020000

#define IMAGE_ENDIAN			0		/* 0 (little endian), 1 (big endian)*/
#define IMAGE_32ENDIAN			0		/* 0 (64 bits endian), 1 (32 bits endian) */
#define IMAGE_INTERLEAVE		0		/* 0 (CbCr separate mode), 1 ( CbCr interleave mode) */
#define STREAM_32ENDIAN		0		/* 0 (64 bits endian), 1 (32 bits endian ) */
#define STREAM_ENDIAN			0       	/* 0 (little endian), 1 (big endian)*/

#define	MC_DMA_PIPE_OPT_EN		1		// MC cache
#define	MC_DMA_CACHE_EN			1		// MC cache

#define USE_BIT_INTERNAL_BUF	1
#define USE_IP_INTERNAL_BUF	1
#define USE_DBK_INTERNAL_BUF	1
#define USE_OVL_INTERNAL_BUF	1
#define	USE_ME_INTERNAL_BUF		1

#define USE_HOST_BIT_INTERNAL_BUF	0
#define USE_HOST_IP_INTERNAL_BUF	0
#define USE_HOST_DBK_INTERNAL_BUF	0
#define	USE_HOST_OVL_INTERNAL_BUF	0
#define USE_HOST_ME_INTERNAL_BUF	0


#define STREAM_FULL_EMPTY_CHECK_DISABLE 0

#define MAX_NUM_INSTANCE 4
#define MAX_NUM_FRAME		32

#define VPU_ISR_BIT_INIT				0x01
#define VPU_ISR_SEQ_INIT			0x02
#define VPU_ISR_SEQ_END			0x04
#define VPU_ISR_PIC_RUN				0x08
#define VPU_ISR_SET_FRAME_BUFFER	0x10
#define VPU_ISR_ENC_HEADER_SET      0x20
#define VPU_ISR_ENC_PARA_SET        0x40
#define VPU_ISR_DEC_PARA_SET		0x80
#define VPU_ISR_BUF_FLUSH			0x100
#define VPU_ISR_BUF_EMPTY			0x4000
#define VPU_ISR_ALL	(VPU_ISR_SEQ_INIT|VPU_ISR_PIC_RUN|VPU_ISR_SET_FRAME_BUFFER|VPU_ISR_ENC_HEADER_SET)
#define VPU_ISR_TRIGMODE_RISING	0x04

#define VPU_ISR_JPG_BBC         0x04
#define VPU_ISR_JPG_DONE        0x01

//#define ADD_ON_CONF 
#define ADD_ON_FRAME			2

#define ROTATOR_FUNC			0x1
#define MIRROR_FUNC				0x2
#define DERING_FUNC				0x4
#define JPG_RESIZE_FUNC			0x8
#define JPG_CLIP_FUNC			0x10
#define JPG_PART_FUNC			0x20

#define DC_TABLE_INDEX0				0
#define AC_TABLE_INDEX0				1
#define DC_TABLE_INDEX1				2
#define AC_TABLE_INDEX1				3

#define Q_COMPONENT0				0
#define Q_COMPONENT1				0x40
#define Q_COMPONENT2				0x80

#define MAX_VSIZE		8192
#define MAX_HSIZE		8192

#define MAX_ENC_PIC_WIDTH        1920 //1280 //720
#define MAX_ENC_PIC_HEIGHT       1088 //720  //576  // FIXME
#define MAX_ENC_MJPG_PIC_WIDTH   8192
#define MAX_ENC_MJPG_PIC_HEIGHT  8192


//------------------------------------------------------------------------------
// common struct and definition
//------------------------------------------------------------------------------
enum CodecStd{
	STD_AVC = 0,
	STD_VC1 = 1,
	STD_MP2 = 2,
	STD_MP4 = 3,
	STD_H263 = 4,
	STD_DIV3 = 5,
	STD_RV = 6,
	STD_AVS = 7,
	STD_MJPG = 8
};

enum {
	AVC_DEC = 0,
	VC1_DEC = 1,
	MP2_DEC = 2,
	MP4_DEC = 3,
	H263_DEC = 3,
	DIV3_DEC = 3,
	RV_DEC = 4,
	AVS_DEC = 5, 
	MJPG_DEC = 6,
	AVC_ENC = 8,
	MP4_ENC = 11,
	MJPG_ENC = 13
};

enum {
	SAMPLE_420 = 0xA,
	SAMPLE_H422 = 0x9,
	SAMPLE_V422 = 0x6,
	SAMPLE_444 = 0x5,
	SAMPLE_400 = 0x1
};

enum {
	YCBCR420 = 0,
	YCBCR422H = 1,
	YCBCR422V = 2,
	YCBCR444 = 3, 
	YCBCR400 = 4
};

enum {
	SEQ_INIT = 1,
	SEQ_END = 2,
	PIC_RUN = 3,
	SET_FRAME_BUF = 4,
	ENC_HEADER = 5,
	ENC_PARA_SET = 6,
	DEC_PARA_SET = 7,
	DEC_BUF_FLUSH = 8,
	RC_CHANGE_PARAMETER = 9,
	FIRMWARE_GET = 15
};

typedef enum {
	RETCODE_SUCCESS,
	RETCODE_FAILURE,
	RETCODE_INVALID_HANDLE,
	RETCODE_INVALID_PARAM,
	RETCODE_INVALID_COMMAND,
	RETCODE_ROTATOR_OUTPUT_NOT_SET,
	RETCODE_ROTATOR_STRIDE_NOT_SET,
	RETCODE_FRAME_NOT_COMPLETE,
	RETCODE_INVALID_FRAME_BUFFER,
	RETCODE_INSUFFICIENT_FRAME_BUFFERS,
	RETCODE_INVALID_STRIDE,
	RETCODE_WRONG_CALL_SEQUENCE,
	RETCODE_CALLED_BEFORE,
	RETCODE_NOT_INITIALIZED,
	RETCODE_MEMORY_ACCESS_VIOLATION
} CodecRetCode;

typedef enum {
	ENABLE_ROTATION,
	DISABLE_ROTATION,
	ENABLE_MIRRORING,
	DISABLE_MIRRORING,
	SET_MIRROR_DIRECTION,
	SET_ROTATION_ANGLE,
	SET_ROTATION_OUTPUT,
	SET_ROTATION_STRIDE,
	DEC_SET_SPS_RBSP,
	DEC_SET_PPS_RBSP,
	ENABLE_DERING,
	DISABLE_DERING,
	DEC_CMD_END
} CodecCommand;

typedef enum {
    // Implicit usage of heritage from CodecCommand enumeration
    ENC_CMD_START = DEC_CMD_END,
    // Encoder Command
    ENC_GET_SPS_RBSP = ENC_CMD_START,
    ENC_GET_PPS_RBSP,
    ENC_PUT_MP4_HEADER,
    ENC_PUT_AVC_HEADER,
    ENC_GET_VOS_HEADER,
    ENC_GET_VO_HEADER,
    ENC_GET_VOL_HEADER,
	ENC_GET_JPEG_HEADER,
    ENC_SET_INTRA_MB_REFRESH_NUMBER,
    ENC_ENABLE_HEC,
    ENC_DISABLE_HEC,
    ENC_SET_SLICE_INFO,
    ENC_SET_GOP_NUMBER,
    ENC_SET_INTRA_QP,
    ENC_SET_BITRATE,
    ENC_SET_FRAME_RATE,
    ENC_SET_REPORT_MBINFO,
    ENC_SET_REPORT_MVINFO,
    ENC_SET_REPORT_SLICEINFO,
    ENC_SET_PIC_PARA_ADDR,
    ENC_SET_SUB_FRAME_SYNC,
    ENC_ENABLE_SUB_FRAME_SYNC,
    ENC_DISABLE_SUB_FRAME_SYNC,
    ENC_CMD_END
} CodecCommandEnc;

typedef enum {
	MIRDIR_NONE,
	MIRDIR_VER,
	MIRDIR_HOR,
	MIRDIR_HOR_VER
}MirrorDirection;

typedef enum{
	ROTANG_0,
	ROTANG_90,
	ROTANG_180,
	ROTANG_270
}RotatorAngle;

typedef enum {
	VOL_HEADER,
	VOS_HEADER,
	VIS_HEADER
}Mp4HeaderType;

typedef enum {
    SPS_RBSP,
    PPS_RBSP
} AvcHeaderType;


enum {
	Marker			= 0xFF,
	FF_Marker		= 0x00,
	
	SOI_Marker		= 0xFFD8,			// Start of image
	EOI_Marker		= 0xFFD9,			// End of image
	
	JFIF_CODE		= 0xFFE0,			// Application
	EXIF_CODE		= 0xFFE1,
	
	DRI_Marker		= 0xFFDD,			// Define restart interval
	RST_Marker		= 0xD,				// 0xD0 ~0xD7
	
	DQT_Marker		= 0xFFDB,			// Define quantization table(s)
	DHT_Marker		= 0xFFC4,			// Define Huffman table(s)
	
	SOF_Marker		= 0xFFC0,			// Start of frame : Baseline DCT
	SOS_Marker		= 0xFFDA,			// Start of scan
};

struct CodecInst;

//------------------------------------------------------------------------------
// decode struct and definition
//------------------------------------------------------------------------------

//typedef struct CodecInst DecInst;
//typedef DecInst * DecHandle;
typedef struct GetBitContext
{
    const guchar *buffer, *buffer_end;
    int index;
    int size_in_bits;
} GetBitContext;

typedef struct {
	PhysicalAddress			code_buf;
	PhysicalAddress			work_buf;
	PhysicalAddress			para_buf;
	PhysicalAddress			slice_buf;
	PhysicalAddress			ps_buf[MAX_NUM_INSTANCE];
	PhysicalAddress			dbky_buf;
	PhysicalAddress			dbkc_buf;
	PhysicalAddress			bit_buf;
	PhysicalAddress			ipacdc_buf;
	PhysicalAddress			ovl_buf;
	PhysicalAddress         btp_buf;
} BitPrcBuffer;

typedef struct {
	PhysicalAddress bufY;
	PhysicalAddress bufCb;
	PhysicalAddress bufCr;
	PhysicalAddress bufMvCol;
} FrameBuffer;

typedef struct {
	guchar bitpro_use;
	guchar dbk_use;
	guchar ipacdc_use;
	guchar ovl_use;
}SecondAXI;

typedef struct {
    gint useBitEnable;
    gint useIpEnable;
    gint useDbkYEnable;
    gint useDbkCEnable;
    gint useOvlEnable;
    gint useBtpEnable;

    gint useHostBitEnable;
    gint useHostIpEnable;
    gint useHostDbkYEnable;
    gint useHostDbkCEnable;
    gint useHostOvlEnable;
    gint useHostBtpEnable;  

    PhysicalAddress bufBitUse;
    PhysicalAddress bufIpAcDcUse;
    PhysicalAddress bufDbkYUse;
    PhysicalAddress bufDbkCUse;
    PhysicalAddress bufOvlUse;
    PhysicalAddress bufBtpUse;
} SecAxiUse;

typedef struct CacheSizeCfg {
    guchar BufferSize : 8;
    guchar PageSizeX  : 4;
    guchar PageSizeY  : 4;
    guchar CacheSizeX : 4;
    guchar CacheSizeY : 4;
    guchar Reserved   : 8;
} CacheSizeCfg;

typedef struct {
    union {
        guint word;
        CacheSizeCfg cfg;
    } luma;
    union {
        guint word;
        CacheSizeCfg cfg;
    } chroma;
    guchar Bypass : 1;
    guchar DualConf : 1;
    guchar PageMerge : 2;
} MaverickCacheConfig;

typedef struct
{
	guint    left;
	guint    top;
	guint    right;
	guint    bottom;
} Rect;

typedef struct 
{
	guint StartX;
	guint StartY;
	guint EndX;
	guint EndY;
	guint PartCount;
	guint SamX;
	guint SamY;
}JPGAddOnConf;

typedef struct {
	gchar *pHeader;
	gint headerSize;
} JpegHeaderBufInfo;

typedef struct {
    // for Nieuport
	gint picWidth;
	gint picHeight;
	gint alignedWidth;
	gint alignedHeight;

	gint ecsPtr;
	gint format;
	gint rstIntval;

	gint userHuffTab;

	gint huffDcIdx;
	gint huffAcIdx;
	gint Qidx;

	guchar huffVal[4][162];	
	guchar huffBits[4][256];	
	guchar cInfoTab[4][6];
	guchar qMatTab[4][64];

	guint huffMin[4][16];
	guint huffMax[4][16];
	guchar huffPtr[4][16];

	gint busReqNum;
	gint compNum;
	gint mcuBlockNum;
	gint compInfo[3];

	gint frameIdx;	
	gint seqInited;

	guchar *pHeader;
	gint headerSize;
	GetBitContext gbc;
} JpgDecInfo;

typedef struct {
	//guchar bitstreamFormat;
	//PhysicalAddress bitstreamBuffer;
	//gint bitstreamBufferSize;
    PhysicalAddress streamRdPtrRegAddr;
    PhysicalAddress streamWrPtrRegAddr;
    PhysicalAddress streamBufStartAddr;
    PhysicalAddress streamBufEndAddr;
	PhysicalAddress frameDisplayFlagRegAddr;
	guint   streamBufSize;	
	guchar  codecMode;
	guchar  mp4DeblkEnable;
	guchar  reorderEnable;
	guchar  filePlayEnable;
	gushort picWidth;
	gushort picHeight;
	gushort dynamicAllocEnable;
	gushort vc1BframeDisplayValid;
	guint   streamStartByteOffset;
	PhysicalAddress psSaveBuffer;
	guint   psSaveBufferSize;
	JPGAddOnConf JPGAddOnParam;
    gint rotatorStride;
    FrameBuffer rotatorOutput;
} DecSeqOpenParam;				/* these parameters will not be changed during the decoding process */

typedef struct {
	gushort picWidth;			// {(PicX+15)/16} * 16
	gushort picHeight;			// {(PicY+15)/16} * 16
	guint   frameRateRes;
	guint   frameRateDiv;
	Rect    picCropRect;
	FrameBuffer  frameBufPool[MAX_NUM_FRAME];
	guchar  initialInfoObtained;	
	guchar  numFrameBuffers;
	guint   allUsedBuffers;
	FrameBuffer recFrame[MAX_NUM_FRAME];
	guint   stride;
	guchar  profile;
	SecondAXI second_axi_use;
	guchar  mp4_dataPartitionEnable;
	guchar  mp4_reverseibleVlcEnable;
	gint    mp4_shortVideoHeader;
	guchar  h263_annexJEnable;
	guchar  minFrameBufferCount;
	guchar  frameBufDelay;
	//guchar nextDecodedIdxNum;		//maybe useless
	gint    normalSliceSize;			//normal slice save buffer size
	gint    worstSliceSize;				//worst case slice save buffer size
	guchar  mjpg_sourceFormat;
	guint   mjpg_ecsPtr;
	guchar  mjpg_thumbNailEnable;
	guint   mjpg_consumedByte;
	JpgDecInfo JpgDecInfo;
} DecSeqGetInfo;		/* these parameters will be changed during the decoding process */

#if 0
typedef	struct{
	PhysicalAddress sliceSaveBuffer;
	gint    sliceSaveBufferSize;
} DecAvcSliceBufInfo;
#endif

#if 0
typedef	struct{
	DecAvcSliceBufInfo avcSliceBufInfo;
} DecBufInfo;
#endif

typedef struct {
	guint   stride;
	guint   bufY;
	guint   bufCb;
	guint   bufCr;
	guint   rotFlag;
} RotOption;

typedef struct {
	guchar  prescanEnable;							/* just for ring-buf mode */
	guchar  prescanMode;
	gchar   dispDelayedPic;						/* just for file-play mode */
	gchar   dispReorderBuf;						
	guchar  iframeSearchEnable;
	guchar  skipframeMode;
	gushort skipframeNum;
	guint   chunkSize;
	guint   picStartByteOffset;
	PhysicalAddress picStreamBufferAddr;			/*it is only used in dynamic allocation*/
	RotOption rotConfig;
} DecFrameCfg;

typedef struct {
	guchar  decodingSuccess;
	guint   decPicWidth;
	guint   decPicHeight;
	gchar   indexFrameDisplay;
	gchar   indexFrameDecoded;
	guchar  picType;
	gushort numOfErrMBs;
	guchar  prescanresult;
	//guchar indexFrameNextDecoded[3];
	guchar  notSufficientPsBuffer;
	guchar  notSufficientSliceBuffer;
	guchar  headerdecodingSuccess;
} DecFrameOutputInfo;

typedef struct {
	guint * paraSet;
	gint    size;
} DecParamSet;

typedef struct DecOperation{
	guint                 RegBase;
	guchar                BitstreamFormat;
	DecSeqOpenParam		  SeqOpenParam;
	DecSeqGetInfo		  SeqInfo;		
	DecFrameCfg			  FrameCfgParam;
	DecFrameOutputInfo	  FrameOutInfo;
	DecParamSet 		  SetParam;
}DecInstCtl;


/********************************
            Enc control 
*********************************/


typedef struct {
    gint mp4_dataPartitionEnable;
    gint mp4_reversibleVlcEnable;
    gint mp4_intraDcVlcThr;
    gint mp4_hecEnable;
    gint mp4_verid;
} EncMp4Param;

typedef struct {
    gint h263_annexIEnable;
    gint h263_annexJEnable;
    gint h263_annexKEnable;
    gint h263_annexTEnable;
} EncH263Param;

typedef struct {
    gint avc_constrainedIntraPredFlag;
    gint avc_disableDeblk;
    gint avc_deblkFilterOffsetAlpha;
    gint avc_deblkFilterOffsetBeta;
    gint avc_chromaQpOffset;
    gint avc_audEnable;
    gint avc_frameCroppingFlag;
    gint avc_frameCropLeft;
    gint avc_frameCropRight;
    gint avc_frameCropTop;
    gint avc_frameCropBottom;
} EncAvcParam;

typedef struct {
    gint mjpg_sourceFormat;
    gint njpg_restartInterval;
	guchar huffVal[4][162];	
	guchar huffBits[4][256];	
	guchar qMatTab[4][64];
	guchar cInfoTab[4][6];
} EncMjpgParam;

typedef struct{
    gint sliceMode;
    gint sliceSizeMode;
    gint sliceSize;
} EncSliceMode;

typedef struct {
    guchar subFrameSyncOn : 1;
    guchar sourceBufNumber : 7;
    guchar sourceBufIndexBase : 8;
} EncSubFrameSyncConfig;

typedef struct {
	gint picWidth;
	gint picHeight;
	gint alignedWidth;
	gint alignedHeight;
	gint seqInited;
	gint frameIdx;	
	gint format;

	gint rstIntval;
	gint busReqNum;
	gint mcuBlockNum;
	gint compNum;
	gint compInfo[3];

	gint huffCode[4][256];
	gint huffSize[4][256];
	guchar pHuffVal[4][162];
	guchar pHuffBits[4][256];
	guchar pCInfoTab[4][6];
	guchar pQMatTab[4][64];
} JpgEncInfo;

typedef struct {
    PhysicalAddress bitstreamBuffer;
    guint bitstreamBufferSize;
	guchar codecMode;
	guchar codecModeAux;
    gint picWidth;
    gint picHeight;
    gint picQpY;
    guint frameRateInfo;
    gint bitRate;
    gint initialDelay;
    gint vbvBufferSize;
    gint enableAutoSkip;
    gint gopSize;

    EncSliceMode slicemode;

    gint intraRefresh;
    
    gint rcIntraQp;  
    gint dynamicAllocEnable;
    gint ringBufferEnable;
    union {
        EncMp4Param mp4Param;
        EncH263Param h263Param;
        EncAvcParam avcParam;
        EncMjpgParam mjpgParam;     
    } EncStdParam;
    gint userQpMax;
    guint userGamma;
    gint RcIntervalMode;     // 0:normal, 1:frame_level, 2:slice_level, 3: user defined Mb_level
    gint MbInterval;         // use when RcintervalMode is 3
    gint MESearchRange;      // 3: 16x16, 2:32x16, 1:64x32, 0:128x64, H.263(Short Header : always 3)
    gint MEUseZeroPmv;       // 0: PMV_ENABLE, 1: PMV_DISABLE
    gint IntraCostWeight;    // Additional weight of Intra Cost for mode decision to reduce Intra MB density
} EncOpenParam;

typedef struct {
    gint minFrameBufferCount;
	gint stride;
} EncInitialInfo;

typedef struct {
    FrameBuffer sourceFrame[MAX_NUM_FRAME];
    gint forceIPicture;
    gint skipPicture;
    gint quantParam;
    PhysicalAddress picStreamBufferAddr;
    gint picStreamBufferSize;

    // Report Information
    gint enReportMBInfo;
    gint enReportMVInfo;
    gint enReportSliceInfo;
    PhysicalAddress picParaBaseAddr;
    PhysicalAddress picMbInfoAddr;
    PhysicalAddress picMvInfoAddr;
    PhysicalAddress picSliceInfoAddr;
} EncParam;

typedef struct {
    PhysicalAddress streamRdPtrRegAddr;
    PhysicalAddress streamWrPtrRegAddr;
    PhysicalAddress streamBufStartAddr;
    PhysicalAddress streamBufEndAddr;
    gint streamBufSize;
    FrameBuffer frameBufPool[MAX_NUM_FRAME];
    gint numFrameBuffers;
    gint stride;
    gint srcFrameWidth;
    gint srcFrameHeight;
    gint rotationEnable;
    gint mirrorEnable;
    MirrorDirection mirrorDirection;
    gint rotationAngle;
    gint initialInfoObtained;
    gint dynamicAllocEnable;
    gint ringBufferEnable;
    SecAxiUse secAxiUse; // FIXME
    MaverickCacheConfig cacheConfig;
    EncSubFrameSyncConfig subFrameSyncConfig;

    // Report Information
    gint enReportMBInfo;
    gint enReportMVInfo;
    gint enReportSliceInfo;
    PhysicalAddress picParaBaseAddr;
    PhysicalAddress picMbInfoAddr;
    PhysicalAddress picMvInfoAddr;
    PhysicalAddress picSliceInfoAddr;
} EncInfo;


// Report Information
typedef struct {
    gint enable;
    gint type;
    gint sz;
    PhysicalAddress addr;
} EncReportInfo;

typedef struct {
    PhysicalAddress bitstreamBuffer;
    guint bitstreamSize;
    gint bitstreamWrapAround; 
    gint picType;
    gint numOfSlices;

    // Report Information
    EncReportInfo MbInfo;
    EncReportInfo MvInfo;
    EncReportInfo SliceInfo;
} EncOutputInfo;

typedef struct {
    PhysicalAddress paraSet;
	guchar *pParaSet;
    gint size;
} EncParamSet;

typedef struct {
    PhysicalAddress buf;
    gint size;
    gint headerType;
#ifdef ENC_VOS_HEADER_LEVEL_PROFILE
    gint userProfileLevelEnable;
    gint userProfileLevelIndication;
#endif
} EncHeaderParam;

typedef struct {
    guint gopNumber;
    guint intraQp;
    guint bitrate;
    guint framerate;
} stChangeRcPara;

typedef struct {
    gint stdMode;
    gint picWidth;
    gint picHeight;
    gint kbps;
    gint rotAngle;
    gint mirDir;
    gint useRot;
    gint qpReport;
    gint saveEncHeader;
    gint dynamicAllocEnable;
    gint ringBufferEnable;
    gint rcIntraQp;
    gint mjpgChromaFormat;
    gint outNum;
	gint framerate_numerator;
	gint framerate_denominator;
	gint BitsPerPixel;
	gint aspectratio_numerator;
	gint aspectratio_denominator;
	gint bitrate;
	gint gop_size;
	gint buf_size;
} EncConfigParam;

typedef struct EncOperation{
	guint                 RegBase;
	guchar                BitstreamFormat;
	EncOpenParam          EncSeqOpenParam;
    EncInitialInfo 		  initialInfo;
	JpgEncInfo 			  jpgInfo;
	EncInfo               pEncInfo;
	EncHeaderParam 		  encHeaderParam;
	EncParam              encParam;
	EncOutputInfo         EncOutInfo;
}EncInstCtl;

guint GetNewNormalReadPtr(void *VPUhandle, guchar CodecInst, guchar dec_enc);
guint GetNewReadPtr(void *VPUhandle, guchar CodecInst,guchar dec_enc);
guint GetVPUIntStatus(void *VPUhandle, guchar dec_enc);
void EnableVPUInt(void *VPUhandle, guchar dec_enc);
void SetVPUIntMask(void *VPUhandle, guint mask, guchar dec_enc);
guint GetVPUIntMask(void *VPUhandle, guchar dec_enc);
void ClrVPUBitInt(void *VPUhandle, guchar dec_enc);
void ClrVPUIntStatus(void *VPUhandle, guint IntStatus, guint IntReason, guchar dec_enc);
guint VPU_GetDispFlag(DecInstCtl *handle, guchar inst_index);
void VPURegInit(void *VPUhandle, gushort start_addr, guchar len, guchar dec_enc);
void BitIssueCommand(DecInstCtl *handle, gint instIdx, gint cdcMode, gint cmd);
void GetInstIndex(DecInstCtl *handle, guchar *index);
CodecRetCode CheckDecOpenParam( DecSeqOpenParam * pop);
void SetDecodeFrameParam(DecFrameCfg *param);
gint DecBitstreamBufEmpty(DecInstCtl *handle, guchar inst_index);
guchar VPU_IsBusy(void *VPUhandle, guchar dec_enc);
CodecRetCode VPU_OpenClose();
CodecRetCode VPU_Init(BitPrcBuffer *PrcBuf, void *handle, guchar dec_enc);
CodecRetCode VPU_GetVersionInfo( DecInstCtl *handle, guint *versionInfo );
CodecRetCode VPU_DecOpen(guchar inst_index, DecInstCtl *handle);
CodecRetCode VPU_DecClose(DecInstCtl *handle, guchar inst_index);
CodecRetCode VPU_DecSetEscSeqInit( DecInstCtl *handle, gint escape );
CodecRetCode VPU_DecInit(DecInstCtl *handle, guchar inst_index);
CodecRetCode VPU_DecRegisterFrameBuffer(DecInstCtl *handle, BitPrcBuffer *VPUbuffer, guchar inst_index);
CodecRetCode VPU_DecUpdateBitstreamBuffer(DecInstCtl *handle, guchar inst_index, PhysicalAddress wrPtr,guint size);
CodecRetCode VPU_DecStartOneFrame(DecInstCtl *handle, guchar inst_index);
CodecRetCode VPU_DecBitBufferFlush(DecInstCtl *handle, guchar inst_idx);
CodecRetCode VPU_DecClrDispFlag( DecInstCtl *handle,  guchar inst_index, guchar frm_index );
CodecRetCode VPU_DecGiveCommand(DecInstCtl *handle,  guchar inst_index,	CodecCommand cmd,void * param);
CodecRetCode VPU_isr_pic_run(DecInstCtl *handle);
CodecRetCode VPU_isr_seq_init(DecInstCtl *handle);
void VPU_isr_seq_end(DecInstCtl *handle) ;
void VPU_isr_flush_buffer(void);
CodecRetCode SetEncOpenParamDefault(guchar format, EncOpenParam *pEncOP,  EncConfigParam *pEncConfig);

CodecRetCode VPU_EncInit(EncInstCtl *handle, guchar inst_index);
CodecRetCode VPU_EncRegisterFrameBuffer(EncInstCtl *handle,BitPrcBuffer *VPUbuffer,guchar inst_index, PhysicalAddress subSampBaseA,PhysicalAddress subSampBaseB);
void VPU_EncPutHeader(EncInstCtl *handle, guchar inst_index);
guint GetNewWritePtr(void *VPUhandle, guchar CodecInst, guchar dec_enc);
CodecRetCode VPU_EncStartOneFrame(EncInstCtl *handle, guchar inst_index );
CodecRetCode VPU_EncOpen(guchar inst_index, EncInstCtl * handle);
CodecRetCode VPU_isr_enc_seq_init(EncInstCtl *handle);
CodecRetCode VPU_isr_enc_reg_framebuf(EncInstCtl *handle );
CodecRetCode VPU_isr_enc_pic_run(EncInstCtl *handle);

guint JPU_GetStatus(void *VPUhandle, guchar dec_enc);
void JPU_ClrStatus(void *VPUhandle, guchar dec_enc, guint val);
gint JPU_IsBusy(void *VPUhandle, guchar dec_enc);


G_END_DECLS



#endif
