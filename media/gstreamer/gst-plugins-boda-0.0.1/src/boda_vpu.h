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

#ifndef BODA_VPU_H
#define BODA_VPU_H

#include "hal_arch.h"
#include "boda_reg.h"

G_BEGIN_DECLS

#define BODA7503
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
#define VPU_ISR_PARA_SET			0x80
#define VPU_ISR_BUF_FLUSH			0x100
#define VPU_ISR_BUF_EMPTY			0x4000
#define VPU_ISR_ALL	(VPU_ISR_SEQ_INIT|VPU_ISR_PIC_RUN|VPU_ISR_SET_FRAME_BUFFER)
#define VPU_ISR_TRIGMODE_RISING	0x04

//#define ADD_ON_CONF 
#define ADD_ON_FRAME			2

#define ROTATOR_FUNC			0x1
#define MIRROR_FUNC				0x2
#define DERING_FUNC				0x4
#define JPG_RESIZE_FUNC			0x8
#define JPG_CLIP_FUNC			0x10
#define JPG_PART_FUNC			0x20

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
	MJPG_DEC = 6
};

enum {
	YCBCR420 = 0,
	YCBCR422H = 1,
	YCBCR422V = 2,
	YCBCR444 = 3, 
	YCBCR400 = 4
};

enum {
	DEC_SEQ_INIT = 1,
	DEC_SEQ_END = 2,
	DEC_PIC_RUN = 3,
	SET_FRAME_BUF = 4,
	DEC_PARA_SET = 7,
	DEC_BUF_FLUSH = 8,
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
	RETCODE_NOT_INITIALIZED
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
	DISABLE_DERING
} CodecCommand;

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
	SPS_HEADER,
	PPS_HEADER
}AVCHeaderType;

struct CodecInst;

//------------------------------------------------------------------------------
// decode struct and definition
//------------------------------------------------------------------------------

//typedef struct CodecInst DecInst;
//typedef DecInst * DecHandle;

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
	//guchar bitstreamFormat;
	//PhysicalAddress bitstreamBuffer;
	//gint bitstreamBufferSize;
	PhysicalAddress streamBufStartAddr;
	//PhysicalAddress streamBufEndAddr;
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
	//guchar  minFrameBufferCount;
	guchar  frameBufDelay;
	//guchar nextDecodedIdxNum;		//maybe useless
	gint    normalSliceSize;			//normal slice save buffer size
	gint    worstSliceSize;				//worst case slice save buffer size
	guchar  mjpg_sourceFormat;
	guchar  mjpg_thumbNailEnable;
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

guint GetNewNormalReadPtr(DecInstCtl *handle, guchar CodecInst);
guint GetNewReadPtr(DecInstCtl *handle, guchar CodecInst);
guint GetNewStartPtr(DecInstCtl *handle, guchar CodecInst);
guint GetVPUIntStatus(DecInstCtl *handle);
void EnableVPUInt(DecInstCtl *handle);
void SetVPUIntMask(DecInstCtl *handle, guint mask);
guint GetVPUIntMask(DecInstCtl *handle);
void ClrVPUBitInt(DecInstCtl *handle);
void ClrVPUIntStatus(DecInstCtl *handle, guint IntStatus, guint IntReason);
guint VPU_GetDispFlag(DecInstCtl *handle, guchar inst_index);
void VPURegInit(DecInstCtl *handle, gushort start_addr, guchar len);
void BitIssueCommand(DecInstCtl *handle, gint instIdx, gint cdcMode, gint cmd);
void GetInstIndex(DecInstCtl *handle, guchar *index);
CodecRetCode CheckDecOpenParam( DecSeqOpenParam * pop);
void SetDecodeFrameParam(DecFrameCfg *param);
gint DecBitstreamBufEmpty(DecInstCtl *handle, guchar inst_index);
guchar VPU_IsBusy(DecInstCtl *handle);
CodecRetCode VPU_OpenClose();
CodecRetCode VPU_Init(BitPrcBuffer *PrcBuf, DecInstCtl *handle);
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

G_END_DECLS



#endif
