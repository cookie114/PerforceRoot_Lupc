	
#ifndef BODA_REG_H
#define BODA_REG_H


/*------------------------------------------------------------------------------
   REGISTER BASE
  ------------------------------------------------------------------------------*/
//#define BIT_BASE                0x10000000
//#define VPU_BASE_ADDRESS					0xB9000000

/*------------------------------------------------------------------------------
   HARDWARE REGISTER
  ------------------------------------------------------------------------------*/
#define BIT_CODE_RUN						0x000
#define BIT_CODE_DOWN						0x004
#define BIT_INT_REQ							0x008
#define BIT_INT_CLEAR						0x00C
#define BIT_INT_STS							0x010
#define BIT_CODE_RESET						0x014							
#define BIT_CUR_PC							0x018
#define BIT_CODEC_BUSY						0x020

/*------------------------------------------------------------------------------
   GLOBAL REGISTER
  ------------------------------------------------------------------------------*/
#define BIT_CODE_BUF_ADDR					0x100
#define BIT_WORK_BUF_ADDR					0x104
#define BIT_PARA_BUF_ADDR					0x108
#define BIT_BIT_STREAM_CTRL					0x10C
#define BIT_FRAME_MEM_CTRL					0x110
#define BIT_DEC_FUNC_CTRL					0x114

#define BIT_RD_PTR_0						0x120
#define BIT_WR_PTR_0						0x124
#define BIT_RD_PTR_1						0x128
#define BIT_WR_PTR_1						0x12C
#define BIT_RD_PTR_2						0x130
#define BIT_WR_PTR_2						0x134
#define BIT_RD_PTR_3						0x138
#define BIT_WR_PTR_3 						0x13C

#define BIT_AXI_SRAM_USE					0x140
#define BIT_EXACT_RD_PTR                    0x144
#define BIT_EXACT_START_ADDR				0x148

#define BIT_FRAME_DIS_FLAG_0				0x150
#define BIT_FRAME_DIS_FLAG_1				0x154
#define BIT_FRAME_DIS_FLAG_2				0x158
#define BIT_FRAME_DIS_FLAG_3				0x15C

#define BIT_BUSY_FLAG						0x160
#define BIT_RUN_COMMAND					0x164
#define BIT_RUN_INDEX						0x168
#define BIT_RUN_COD_STD					0x16C

#define BIT_INT_ENABLE						0x170
#define BIT_INT_REASON						0x174

#define BIT_RUN_AUX_STD					0x178

/*------------------------------------------------------------------------------
   [DEC SEQ INIT] COMMAND
  ------------------------------------------------------------------------------*/
#define CMD_DEC_SEQ_BB_START				0x180
#define CMD_DEC_SEQ_BB_SIZE				0x184
#define CMD_DEC_SEQ_OPTION				0x188
#define CMD_DEC_SEQ_SRC_SIZE				0x18C
#define CMD_DEC_SEQ_START_BYTE			0x190
#define CMD_DEC_SEQ_PS_BB_START			0x194
#define CMD_DEC_SEQ_PS_BB_SIZE			0x198

#define CMD_DEC_SEQ_THM_EN				0x19C

#define CMD_DEC_SEQ_CLIP_MODE			0x1AC
#define CMD_DEC_SEQ_CLIP_FROM			0x1B0
#define CMD_DEC_SEQ_CLIP_TO				0x1B4
#define CMD_DEC_SEQ_CLIP_CNT			0x1B8
#define CMD_DEC_SEQ_SAM_XY				0x1BC

#define RET_DEC_SEQ_ASPECT				0x1B0
#define RET_DEC_SEQ_SUCCESS				0x1C0
#define RET_DEC_SEQ_SRC_SIZE				0x1C4
#define RET_DEC_SEQ_SRC_F_RATE			0x1C8
#define RET_DEC_SEQ_FRAME_NEED			0x1CC
#define RET_DEC_SEQ_FRAME_DELAY			0x1D0
#define RET_DEC_SEQ_INFO					0x1D4
#define RET_DEC_SEQ_CROP_LEFT_RIGHT		0x1D8
#define RET_DEC_SEQ_CROP_TOP_BOTTOM		0x1DC
#define RET_DEC_SEQ_NEXT_FRAME_NUM		0x1E0

#define	RET_DEC_SEQ_NUM_UNITS_IN_TICK   0x1E4
#define RET_DEC_SEQ_TIME_SCALE          0x1E8

#define RET_DEC_SEQ_JPG_PARA				0x1E4
#define RET_DEC_SEQ_JPG_THUMB_IND			0x1E8
#define RET_DEC_SEQ_HEADER_REPORT		0x1EC

#define RET_DEC_SEQ_FRATE_N				0x1EC
#define RET_DEC_SEQ_FRATE_D				0x1F0

/*------------------------------------------------------------------------------
   [DEC PIC RUN] COMMAND
  ------------------------------------------------------------------------------*/
#define CMD_DEC_PIC_ROT_MODE				0x180
#define CMD_DEC_PIC_ROT_ADDR_Y			0x184
#define CMD_DEC_PIC_ROT_ADDR_CB			0x188
#define CMD_DEC_PIC_ROT_ADDR_CR			0x18C
#define CMD_DEC_PIC_ROT_STRIDE			0x190

#define CMD_DEC_PIC_OPTION					0x194
#define CMD_DEC_PIC_SKIP_NUM				0x198
#define CMD_DEC_PIC_CHUNK_SIZE			0x19C
#define CMD_DEC_PIC_BB_START				0x1A0
#define CMD_DEC_PIC_START_BYTE			0x1A4

#define CMD_DEC_PIC_PARA_BADDR			0x1A8
#define CMD_DEC_PIC_USER_BADDR			0x1AC
#define CMD_DEC_PIC_USER_SIZE			0x1B0

#define RET_DEC_PIC_SIZE				0x1BC
#define RET_DEC_PIC_FRAME_NUM				0x1C0
#define RET_DEC_PIC_FRAME_IDX				0x1C4
#define RET_DEC_PIC_ERR_MB					0x1C8
#define RET_DEC_PIC_TYPE					0x1CC
#define RET_DEC_PIC_POST				0x1D0
#define RET_DEC_PIC_OPTION					0x1D4
#define RET_DEC_PIC_SUCCESS				0x1D8
#define RET_DEC_PIC_CUR_IDX				0x1DC
//#define RET_DEC_PIC_NEXT_IDX				0x1E0

/*------------------------------------------------------------------------------
   [SET FRAME BUF] COMMAND
  ------------------------------------------------------------------------------*/
#define CMD_SET_FRAME_BUF_NUM			0x180
#define CMD_SET_FRAME_BUF_STRIDE			0x184
#define CMD_SET_FRAME_SLICE_BB_START		0x188
#define CMD_SET_FRAME_SLICE_BB_SIZE		0x18C

#define CMD_SET_FRAME_AXI_BIT_ADDR		0x190
#define CMD_SET_FRAME_AXI_ACDC_ADDR		0x194
#define CMD_SET_FRAME_AXI_DBKY_ADDR		0x198
#define CMD_SET_FRAME_AXI_DBKC_ADDR		0x19C
#define CMD_SET_FRAME_AXI_OVL_ADDR		0x1A0
/*------------------------------------------------------------------------------
   [DEC_PARA_SET] COMMAND
  ------------------------------------------------------------------------------*/
#define CMD_DEC_PARA_SET_TYPE				0x180
#define CMD_DEC_PARA_SET_SIZE				0x184

/*------------------------------------------------------------------------------
   [FIRMWARE VERSION] COMMAND  
   [32:16] project number => 
   [16:0]  version => xxxx.xxxx.xxxxxxxx 
  ------------------------------------------------------------------------------*/
#define RET_VER_NUM							0x1c0


#endif



