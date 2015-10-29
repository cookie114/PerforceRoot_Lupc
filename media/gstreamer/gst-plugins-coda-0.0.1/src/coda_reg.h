	
#ifndef CODA_REG_H
#define CODA_REG_H


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

#define BIT_FRAME_DIS_FLAG_0				0x150
#define BIT_FRAME_DIS_FLAG_1				0x154
#define BIT_FRAME_DIS_FLAG_2				0x158
#define BIT_FRAME_DIS_FLAG_3				0x15C

#define BIT_BUSY_FLAG						0x160
#define BIT_RUN_COMMAND						0x164
#define BIT_RUN_INDEX						0x168
#define BIT_RUN_COD_STD						0x16C

#define BIT_INT_ENABLE						0x170
#define BIT_INT_REASON						0x174

#define BIT_RUN_AUX_STD						0x178

/*------------------------------------------------------------------------------
   [DEC SEQ INIT] COMMAND
  ------------------------------------------------------------------------------*/
#define CMD_DEC_SEQ_BB_START				0x180
#define CMD_DEC_SEQ_BB_SIZE					0x184
#define CMD_DEC_SEQ_OPTION					0x188
#define CMD_DEC_SEQ_SRC_SIZE				0x18C
#define CMD_DEC_SEQ_START_BYTE				0x190
#define CMD_DEC_SEQ_PS_BB_START				0x194
#define CMD_DEC_SEQ_PS_BB_SIZE				0x198

#define CMD_DEC_SEQ_THM_EN					0x19C

#define CMD_DEC_SEQ_CLIP_MODE				0x1AC
#define CMD_DEC_SEQ_CLIP_FROM				0x1B0
#define CMD_DEC_SEQ_CLIP_TO					0x1B4
#define CMD_DEC_SEQ_CLIP_CNT				0x1B8
#define CMD_DEC_SEQ_SAM_XY					0x1BC

#define RET_DEC_SEQ_ASPECT					0x1B0
#define RET_DEC_SEQ_SUCCESS					0x1C0
#define RET_DEC_SEQ_SRC_SIZE				0x1C4
#define RET_DEC_SEQ_SRC_F_RATE				0x1C8
#define RET_DEC_SEQ_FRAME_NEED				0x1CC
#define RET_DEC_SEQ_FRAME_DELAY				0x1D0
#define RET_DEC_SEQ_INFO					0x1D4
#define RET_DEC_SEQ_CROP_LEFT_RIGHT			0x1D8
#define RET_DEC_SEQ_CROP_TOP_BOTTOM			0x1DC
#define RET_DEC_SEQ_NEXT_FRAME_NUM			0x1E0

#define	RET_DEC_SEQ_NUM_UNITS_IN_TICK   	0x1E4
#define RET_DEC_SEQ_TIME_SCALE          	0x1E8

#define RET_DEC_SEQ_JPG_PARA				0x1E4
#define RET_DEC_SEQ_JPG_THUMB_IND			0x1E8
#define RET_DEC_SEQ_HEADER_REPORT			0x1EC

/*------------------------------------------------------------------------------
   [DEC PIC RUN] COMMAND
  ------------------------------------------------------------------------------*/
#define CMD_DEC_PIC_ROT_MODE				0x180
#define CMD_DEC_PIC_ROT_ADDR_Y				0x184
#define CMD_DEC_PIC_ROT_ADDR_CB				0x188
#define CMD_DEC_PIC_ROT_ADDR_CR				0x18C
#define CMD_DEC_PIC_ROT_STRIDE				0x190

#define CMD_DEC_PIC_OPTION					0x194
#define CMD_DEC_PIC_SKIP_NUM				0x198
#define CMD_DEC_PIC_CHUNK_SIZE				0x19C
#define CMD_DEC_PIC_BB_START				0x1A0
#define CMD_DEC_PIC_START_BYTE				0x1A4

#define CMD_DEC_PIC_PARA_BADDR				0x1A8
#define CMD_DEC_PIC_USER_BADDR				0x1AC
#define CMD_DEC_PIC_USER_SIZE				0x1B0

#define RET_DEC_PIC_SIZE					0x1BC
#define RET_DEC_PIC_FRAME_NUM				0x1C0
#define RET_DEC_PIC_FRAME_IDX				0x1C4
#define RET_DEC_PIC_ERR_MB					0x1C8
#define RET_DEC_PIC_TYPE					0x1CC
#define RET_DEC_PIC_POST					0x1D0
#define RET_DEC_PIC_OPTION					0x1D4
#define RET_DEC_PIC_SUCCESS					0x1D8
#define RET_DEC_PIC_CUR_IDX					0x1DC
//#define RET_DEC_PIC_NEXT_IDX				0x1E0

/*------------------------------------------------------------------------------
   [SET FRAME BUF] COMMAND
  ------------------------------------------------------------------------------*/
#define CMD_SET_FRAME_BUF_NUM				0x180
#define CMD_SET_FRAME_BUF_STRIDE			0x184
#define CMD_SET_FRAME_SLICE_BB_START		0x188
#define CMD_SET_FRAME_SLICE_BB_SIZE			0x18C

#define CMD_SET_FRAME_AXI_BIT_ADDR			0x190
#define CMD_SET_FRAME_AXI_ACDC_ADDR			0x194
#define CMD_SET_FRAME_AXI_DBKY_ADDR			0x198
#define CMD_SET_FRAME_AXI_DBKC_ADDR			0x19C
#define CMD_SET_FRAME_AXI_OVL_ADDR			0x1A0
#define CMD_SET_FRAME_AXI_BTP_ADDR          0x1A4
#define CMD_SET_FRAME_CACHE_SIZE        	0x1A8
#define CMD_SET_FRAME_CACHE_CONFIG      	0x1AC
#define CMD_SET_FRAME_MB_BUF_BASE       	0x1B0

#define RET_SET_FRAME_SUCCESS           	0x1C0

/*------------------------------------------------------------------------------
   [DEC_PARA_SET] COMMAND
  ------------------------------------------------------------------------------*/
#define CMD_DEC_PARA_SET_TYPE				0x180
#define CMD_DEC_PARA_SET_SIZE				0x184

/*------------------------------------------------------------------------------
   [ENC_SEQ_INIT] COMMAND
  ------------------------------------------------------------------------------*/
#define CMD_ENC_SEQ_BB_START         		0x180
#define CMD_ENC_SEQ_BB_SIZE         		0x184
#define CMD_ENC_SEQ_OPTION          		0x188          // HecEnable,ConstIntraQp, FMO, QPREP, AUD, SLICE, MB BIT
#define CMD_ENC_SEQ_COD_STD         		0x18C
#define CMD_ENC_SEQ_SRC_SIZE        		0x190
#define CMD_ENC_SEQ_SRC_F_RATE     			0x194
#define CMD_ENC_SEQ_MP4_PARA        		0x198
#define CMD_ENC_SEQ_263_PARA        		0x19C
#define CMD_ENC_SEQ_264_PARA       		 	0x1A0
#define CMD_ENC_SEQ_SLICE_MODE      		0x1A4
#define CMD_ENC_SEQ_GOP_NUM         		0x1A8
#define CMD_ENC_SEQ_RC_PARA         		0x1AC
#define CMD_ENC_SEQ_RC_BUF_SIZE     		0x1B0
#define CMD_ENC_SEQ_INTRA_REFRESH   		0x1B4

#define CMD_ENC_SEQ_INTRA_QP              	0x1C4
#define CMD_ENC_SEQ_RC_QP_MAX             	0x1C8
#define CMD_ENC_SEQ_RC_GAMMA              	0x1CC
#define CMD_ENC_SEQ_RC_INTERVAL_MODE      	0x1D0      // MbInterval[32:2], RcIntervalMode[1:0]
#define CMD_ENC_SEQ_INTRA_WEIGHT          	0x1D4
#define CMD_ENC_SEQ_ME_OPTION             	0x1D8
#define RET_ENC_SEQ_SUCCESS         	  	0x1C0

//------------------------------------------------------------------------------
// [ENC SEQ END] COMMAND
//------------------------------------------------------------------------------

#define RET_ENC_SEQ_END_SUCCESS     		0x1C0


//------------------------------------------------------------------------------
// [ENC PIC RUN] COMMAND
//------------------------------------------------------------------------------
#define CMD_ENC_PIC_SRC_INDEX         		0x180
#define CMD_ENC_PIC_SRC_STRIDE        		0x184
#define CMD_ENC_PIC_QS                		0x18C
#define CMD_ENC_PIC_ROT_MODE          		0x190
#define CMD_ENC_PIC_OPTION            		0x194
#define CMD_ENC_PIC_BB_START          		0x198
#define CMD_ENC_PIC_BB_SIZE           		0x19C
#define CMD_ENC_PIC_PARA_BASE_ADDR    		0x1A0
#define CMD_ENC_PIC_SUB_FRAME_SYNC    		0x1A4
#define CMD_ENC_PIC_SRC_ADDR_Y       		0x1A8
#define CMD_ENC_PIC_SRC_ADDR_CB       		0x1AC
#define CMD_ENC_PIC_SRC_ADDR_CR       		0x1B0

#define RET_ENC_PIC_FRAME_NUM         		0x1C0
#define RET_ENC_PIC_TYPE              		0x1C4
#define RET_ENC_PIC_FRAME_IDX         		0x1C8
#define RET_ENC_PIC_SLICE_NUM         		0x1CC
#define RET_ENC_PIC_FLAG              		0x1D0
#define RET_ENC_PIC_SUCCESS           		0x1D8

//------------------------------------------------------------------------------
// [ENC HEADER] COMMAND
//------------------------------------------------------------------------------
#define CMD_ENC_HEADER_CODE          		0x180
#define CMD_ENC_HEADER_BB_START       		0x184
#define CMD_ENC_HEADER_BB_SIZE        		0x188
#define CMD_ENC_HEADER_FRAME_CROP_H   		0x18C
#define CMD_ENC_HEADER_FRAME_CROP_V   		0x190

#define RET_ENC_HEADER_SUCCESS        		0x1C0


//------------------------------------------------------------------------------
// [ENC_PARA_SET] COMMAND
//------------------------------------------------------------------------------
#define CMD_ENC_PARA_SET_TYPE        		0x180
#define RET_ENC_PARA_SET_SIZE         		0x1C0
#define RET_ENC_PARA_SET_SUCCESS      		0x1C4

//------------------------------------------------------------------------------
// [ENC PARA CHANGE] COMMAND :
//------------------------------------------------------------------------------
#define CMD_ENC_SEQ_PARA_CHANGE_ENABLE    	0x180      // FrameRateEn[3], BitRateEn[2], IntraQpEn[1], GopEn[0]
#define CMD_ENC_SEQ_PARA_RC_GOP           	0x184
#define CMD_ENC_SEQ_PARA_RC_INTRA_QP      	0x188      
#define CMD_ENC_SEQ_PARA_RC_BITRATE       	0x18C
#define CMD_ENC_SEQ_PARA_RC_FRAME_RATE    	0x190
#define CMD_ENC_SEQ_PARA_INTRA_MB_NUM     	0x194      // update param
#define CMD_ENC_SEQ_PARA_SLICE_MODE       	0x198      // update param
#define CMD_ENC_SEQ_PARA_HEC_MODE         	0x19C      // update param

#define RET_ENC_SEQ_PARA_CHANGE_SECCESS   	0x1C0

// Magellan ENCODER ONLY
#define CMD_SET_FRAME_SUBSAMP_A        	 	0x188
#define CMD_SET_FRAME_SUBSAMP_B         	0x18C
//------------------------------------------------------------------------------
// [SET PIC INFO] COMMAND
//------------------------------------------------------------------------------
#define GDI_PRI_RD_PRIO_L             		0x1000
#define GDI_PRI_RD_PRIO_H             		0x1004
#define GDI_PRI_WR_PRIO_L             		0x1008
#define GDI_PRI_WR_PRIO_H             		0x100c
#define GDI_PRI_RD_LOCK_CNT           		0x1010
#define GDI_PRI_WR_LOCK_CNT           		0x1014
#define GDI_SEC_RD_PRIO_L             		0x1018
#define GDI_SEC_RD_PRIO_H             		0x101c
#define GDI_SEC_WR_PRIO_L             		0x1020
#define GDI_SEC_WR_PRIO_H             		0x1024
#define GDI_SEC_RD_LOCK_CNT           		0x1028
#define GDI_SEC_WR_LOCK_CNT           		0x102c
#define GDI_SEC_CLIENT_EN             		0x1030
#define GDI_CONTROL                   		0x1034
#define GDI_PIC_INIT_HOST             		0x1038
#define GDI_PINFO_REQ                 		0x1060
#define GDI_PINFO_ACK                 		0x1064
#define GDI_PINFO_ADDR                		0x1068
#define GDI_PINFO_DATA                		0x106c
#define GDI_BWB_ENABLE                		0x1070
#define GDI_BWB_SIZE                  		0x1074
#define GDI_BWB_STD_STRUCT            		0x1078
#define GDI_BWB_STATUS                		0x107c
#define GDI_STATUS                    		0x1080
#define GDI_DEBUG_0                   		0x1084
#define GDI_DEBUG_1                   		0x1088
#define GDI_DEBUG_2                   		0x108c
#define GDI_DEBUG_3                   		0x1090
#define GDI_DEBUG_PROBE_ADDR          		0x1094
#define GDI_DEBUG_PROBE_DATA          		0x1098
// write protect
#define GDI_WPROT_ERR_CLR			  		0x10A0
#define GDI_WPROT_ERR_RSN			  		0x10A4
#define GDI_WPROT_ERR_ADR			  		0x10A8
#define GDI_WPROT_RGN_EN			  		0x10AC
#define GDI_WPROT_RGN0_STA			  		0x10B0
#define GDI_WPROT_RGN0_END			  		0x10B4
#define GDI_WPROT_RGN1_STA			  		0x10B8
#define GDI_WPROT_RGN1_END			  		0x10BC
#define GDI_WPROT_RGN2_STA			  		0x10C0
#define GDI_WPROT_RGN2_END			  		0x10C4
#define GDI_WPROT_RGN3_STA			  		0x10C8
#define GDI_WPROT_RGN3_END			  		0x10CC	
#define GDI_WPROT_RGN4_STA			  		0x10D0
#define GDI_WPROT_RGN4_END			  		0x10D4
#define GDI_WPROT_RGN5_STA			  		0x10D8
#define GDI_WPROT_RGN5_END			  		0x10DC

#define GDI_BUS_CTRL                  		0x10f0
#define GDI_BUS_STATUS                		0x10f4
#define GDI_DCU_PIC_SIZE              		0x10a8 // correct?
#define GDI_SIZE_ERR_FLAG             		0x10e0
#define GDI_INFO_CONTROL              		0x1400
#define GDI_INFO_PIC_SIZE             		0x1404
#define GDI_INFO_BASE_Y               		0x1408
#define GDI_INFO_BASE_CB              		0x140c
#define GDI_INFO_BASE_CR              		0x1410
#define GDI_XY2_CAS_0                 		0x1800
#define GDI_XY2_CAS_F                 		0x183c
#define GDI_XY2_BA_0                  		0x1840
#define GDI_XY2_BA_1                  		0x1844
#define GDI_XY2_BA_2                  		0x1848
#define GDI_XY2_BA_3                  		0x184c
#define GDI_XY2_RAS_0                 		0x1850
#define GDI_XY2_RAS_F                 		0x188c

#define GDI_XY2_RBC_CONFIG            		0x1890
#define GDI_RBC2_AXI_0                		0x18a0
#define GDI_RBC2_AXI_1F               		0x191c

/*--------------------------------------------------------------------
NIEUPORT REGISTERS
--------------------------------------------------------------------*/
// MBC
#define MJPEG_PIC_START_REG			  		0x3000	// [0] - pic start
#define MJPEG_PIC_STATUS_REG		  		0x3004	// [3] - overflow, [2] - bbc interrupt, [1] - error, [0] - done
#define MJPEG_PIC_ERRMB_REG			  		0x3008	// [27:24] - error restart idx, [23:12] - error MCU pos X, [11:0] - error MCU pos Y
#define MJPEG_PIC_SETMB_REG			  		0x300C	// [27:16] - MCU pos X, [11:0] - MCU pos Y

#define MJPEG_PIC_CTRL_REG			  		0x3010	// [6] - user huffman en, [4] - TC direction, [3] - encoder enable, [1:0] - operation mode
#define MJPEG_PIC_SIZE_REG			  		0x3014
#define MJPEG_MCU_INFO_REG			  		0x3018
#define MJPEG_ROT_INFO_REG			 		0x301C	// [4] - rot-mir enable, [3:0] - rot-mir mode

#define MJPEG_SCL_INFO_REG			  		0x3020
#define MJPEG_IF_INFO_REG			  		0x3024	// [1] - sensor interface clear, [0] - display interface clear
#define MJPEG_OP_INFO_REG			  		0x302C	// [31:16] - # of line in 1 partial buffer, [5:3] - # of partial buffers [2:0] - # of request

#define MJPEG_DPB_CONFIG_REG          		0x3030
#define MJPEG_DPB_BASE00_REG		  		0x3034
#define MJPEG_DPB_BASE01_REG		  		0x3038
#define MJPEG_DPB_BASE02_REG		  		0x303C

#define MJPEG_DPB_BASE10_REG		  		0x3040
#define MJPEG_DPB_BASE11_REG		  		0x3044
#define MJPEG_DPB_BASE12_REG		  		0x3048
#define MJPEG_DPB_BASE20_REG		  		0x304C

#define MJPEG_DPB_BASE21_REG		  		0x3050
#define MJPEG_DPB_BASE22_REG		  		0x3054
#define MJPEG_DPB_BASE30_REG		  		0x3058
#define MJPEG_DPB_BASE31_REG		  		0x305C

#define MJPEG_DPB_BASE32_REG		  		0x3060
#define MJPEG_DPB_YSTRIDE_REG		  		0x3064
#define MJPEG_DPB_CSTRIDE_REG		  		0x3068

#define MJPEG_HUFF_CTRL_REG			  		0x3080
#define MJPEG_HUFF_ADDR_REG			  		0x3084
#define MJPEG_HUFF_DATA_REG			  		0x3088

#define MJPEG_QMAT_CTRL_REG			  		0x3090
#define MJPEG_QMAT_ADDR_REG			  		0x3094
#define MJPEG_QMAT_DATA_REG			  		0x3098

#define MJPEG_COEF_CTRL_REG			  		0x30A0
#define MJPEG_COEF_ADDR_REG			  		0x30A4
#define MJPEG_COEF_DATA_REG			  		0x30A8

#define MJPEG_RST_INTVAL_REG		  		0x30B0
#define MJPEG_RST_INDEX_REG			  		0x30B4
#define MJPEG_RST_COUNT_REG			  		0x30B8

#define	MJPEG_INTR_MASK_REG			  		0x30C0
#define MJPEG_CYCLE_INFO_REG		  		0x30C8

#define MJPEG_DPCM_DIFF_Y_REG		  		0x30F0
#define MJPEG_DPCM_DIFF_CB_REG		  		0x30F4
#define MJPEG_DPCM_DIFF_CR_REG		  		0x30F8

// GBU	
#define MJPEG_GBU_CTRL_REG			  		0x3100
#define MJPEG_GBU_BT_PTR_REG		  		0x3110
#define MJPEG_GBU_WD_PTR_REG		  		0x3114
#define MJPEG_GBU_TT_CNT_REG		  		0x3118
//#define MJPEG_GBU_TT_CNT_REG+4		    0x311C

#define MJPEG_GBU_BBSR_REG			  		0x3140
#define MJPEG_GBU_BBER_REG			  		0x3144
#define MJPEG_GBU_BBIR_REG			  		0x3148
#define MJPEG_GBU_BBHR_REG			  		0x314C

#define MJPEG_GBU_BCNT_REG			  		0x3158
#define MJPEG_GBU_FF_RPTR_REG		  		0x3160
#define MJPEG_GBU_FF_WPTR_REG		  		0x3164

// BBC
#define MJPEG_BBC_END_ADDR_REG		  		0x3208
#define MJPEG_BBC_WR_PTR_REG		  		0x320C
#define MJPEG_BBC_RD_PTR_REG		  		0x3210

#define MJPEG_BBC_EXT_ADDR_REG		  		0x3214
#define MJPEG_BBC_INT_ADDR_REG		  		0x3218
#define MJPEG_BBC_DATA_CNT_REG		  		0x321C
#define MJPEG_BBC_COMMAND_REG		  		0x3220
#define MJPEG_BBC_BUSY_REG			  		0x3224

#define MJPEG_BBC_CTRL_REG			  		0x3228
#define MJPEG_BBC_CUR_POS_REG		  		0x322C

#define MJPEG_BBC_BAS_ADDR_REG		  		0x3230
#define MJPEG_BBC_STRM_CTRL_REG		  		0x3234

#define MJPEG_BBC_FLUSH_CMD_REG		  		0x3238

/*------------------------------------------------------------------------------
   [FIRMWARE VERSION] COMMAND  
   [32:16] project number => 
   [16:0]  version => xxxx.xxxx.xxxxxxxx 
  ------------------------------------------------------------------------------*/
#define RET_VER_NUM							0x1c0


#endif



