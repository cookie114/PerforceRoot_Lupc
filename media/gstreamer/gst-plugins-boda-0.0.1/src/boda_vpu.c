#include "boda_vpu.h"

#ifdef BODA7503
	#include "BODA7503.h"
#endif


#define	VpuReadReg(offset)	HAL_GET_UINT32((guint *)(handle->RegBase+offset))
#define VpuWriteReg(offset, value) HAL_PUT_UINT32((guint *)(handle->RegBase+offset), value)

guint GetNewNormalReadPtr(DecInstCtl *handle, guchar CodecInst)
{
    return ( VpuReadReg(BIT_RD_PTR_0 + 8 * CodecInst)  );
}

guint GetNewReadPtr(DecInstCtl *handle, guchar CodecInst)
{
	//return ( VpuReadReg(BIT_RD_PTR_0 + 8 * CodecInst)  );
	return ( VpuReadReg(BIT_EXACT_RD_PTR) ) ;
}

guint GetNewStartPtr(DecInstCtl *handle, guchar CodecInst)
{
	return ( VpuReadReg(BIT_EXACT_START_ADDR));
}

guint GetVPUIntStatus(DecInstCtl *handle)
{
	return ( VpuReadReg(BIT_INT_REASON) );
}

void EnableVPUInt(DecInstCtl *handle)
{
	VpuWriteReg(BIT_INT_ENABLE, VPU_ISR_ALL);
}

void SetVPUIntMask(DecInstCtl *handle, guint mask)
{
	VpuWriteReg(BIT_INT_ENABLE, mask);
}

guint GetVPUIntMask(DecInstCtl *handle)
{
	return ( VpuReadReg(BIT_INT_ENABLE) );
}

void ClrVPUBitInt(DecInstCtl *handle)
{
	VpuWriteReg( BIT_INT_CLEAR, 0x1 );
}

void ClrVPUIntStatus(DecInstCtl *handle, guint IntStatus, guint IntReason)
{
	VpuWriteReg(BIT_INT_REASON, IntStatus & (~IntReason));

}

#if 0
CodecRetCode CheckDecInstanceValidity(CodecInst * pCodecInst)
{
	CodecRetCode ret;

	if (!pCodecInst->inUse) {
		return RETCODE_INVALID_HANDLE;
	}
	if (pCodecInst->codecMode != AVC_DEC && 
		pCodecInst->codecMode != VC1_DEC &&
		pCodecInst->codecMode != MP2_DEC &&
		pCodecInst->codecMode != MP4_DEC &&
		pCodecInst->codecMode != AVS_DEC &&
		pCodecInst->codecMode != H263_DEC &&
		pCodecInst->codecMode != DIV3_DEC &&
		pCodecInst->codecMode != RV_DEC &&
		pCodecInst->codecMode != MJPG_DEC) {
		return RETCODE_INVALID_HANDLE;
	}

	return RETCODE_SUCCESS;
}

void FreeCodecInstance(CodecInst * pCodecInst)
{
	pCodecInst->inUse = 0;
}
#endif

void VPURegInit(DecInstCtl *handle, gushort start_addr, guchar len)
{
	/* Set the registers value 0 from 0x180 to 0x1FC */
	guchar i;

	for ( i = 0; i < len; i++ ) {
		VpuWriteReg(start_addr+i*4, 0);
	}
}

void BitIssueCommand(DecInstCtl *handle, gint instIdx, gint cdcMode, gint cmd)
{
	VpuWriteReg(BIT_BUSY_FLAG, 1);
	VpuWriteReg(BIT_RUN_INDEX, instIdx);
	VpuWriteReg(BIT_RUN_COD_STD, cdcMode);
	VpuWriteReg(BIT_RUN_COMMAND, cmd);
}

void GetInstIndex(DecInstCtl *handle, guchar *index)
{
	*index = VpuReadReg(BIT_RUN_INDEX);
}

guint VPU_GetDispFlag(DecInstCtl *handle, guchar inst_index)
{
	return (VpuReadReg(BIT_FRAME_DIS_FLAG_0+4*inst_index));
}

CodecRetCode CheckDecOpenParam( DecSeqOpenParam * pop)
{
	if (pop == 0) {
		GST_ERROR("set param null!\n");
		return RETCODE_INVALID_PARAM;
	}
	if (pop->streamBufStartAddr % 4) { // not 4-byte aligned
		GST_ERROR("stream buffer start address not 4-byte aligned, %x\n", pop->streamBufStartAddr);
		return RETCODE_INVALID_PARAM;
	}
	if (pop->streamBufSize % 1024 ||
			pop->streamBufSize < 1024 ||
			pop->streamBufSize > 16383 * 1024) {
		GST_ERROR("stream buffer size not valid %x\n", pop->streamBufSize);
		return RETCODE_INVALID_PARAM;
	}
	
	return RETCODE_SUCCESS;
}

#if 0
void SetDecodeFrameParam(DecFrameCfg *param)
{
	param->chunkSize = 0;
	param->dispDelayedPic = 0;
	param->iframeSearchEnable = 0;
	param->picStartByteOffset = 0;
	param->picStreamBufferAddr = 0;
	param->prescanEnable = 0;
	param->prescanMode = 0;
	param->skipframeMode = 0;
	param->skipframeNum = 0;
	
	return;
}
#endif

gint DecBitstreamBufEmpty(DecInstCtl *handle, guchar inst_index)
{
	return ( VpuReadReg(BIT_RD_PTR_0 + 8*inst_index) == VpuReadReg(BIT_WR_PTR_0 + 8*inst_index) );
}

#if 0
void SetParaSet(DecHandle handle, INT32 paraSetType, DecParamSet * para)
{
	CodecInst * pCodecInst;
	DecInfo * pDecInfo;
	INT32 i;
	UINT32 * src;

	pCodecInst = handle;
	pDecInfo = &(pCodecInst->decInfo);

	src = para->paraSet;
	for (i = 0; i < para->size; i += 4) {
		VpuWriteReg(paraBuffer + i, *src++);
	}
	VpuWriteReg(CMD_DEC_PARA_SET_TYPE, paraSetType); // 0: SPS, 1: PPS
	VpuWriteReg(CMD_DEC_PARA_SET_SIZE, para->size);
	BitIssueCommand(handle, pCodecInst->instIndex, pCodecInst->codecMode, DEC_PARA_SET);
	while (VpuReadReg(BIT_BUSY_FLAG));
}
#endif

/* the VPU API function */

guchar VPU_IsBusy(DecInstCtl *handle)
{
	return VpuReadReg(BIT_BUSY_FLAG) != 0;
}


CodecRetCode VPU_Init(BitPrcBuffer *PrcBuf, DecInstCtl *handle)
{
	PhysicalAddress codeBuffer;
	guint top_reg_base_addr;
	guint i;
	guint data, dataH, dataL;
	guint cur_pc, val;

//#ifdef USE_SECOND_AXI
#if 0
	top_reg_base_addr = (guint)hal_paddr_to_vaddr(SOC_REG_KA_TO_PA(SOC_REG_BASE_SYS));
	HAL_PUT_UINT32((guint*)(top_reg_base_addr+0x0100), ((HAL_GET_UINT32((guint*)(top_reg_base_addr+0x0100))&0xffffff) | 0x11000000));
#endif

	if ( VpuReadReg(BIT_CODEC_BUSY) ){
		val = VpuReadReg( BIT_DEC_FUNC_CTRL );
		val |= 0x3C;
		VpuWriteReg(BIT_DEC_FUNC_CTRL, val);	
		g_usleep(10000);
	}
	
	if ( VpuReadReg(BIT_INT_STS) )
		ClrVPUBitInt(handle);
	
	if ( VpuReadReg(BIT_CUR_PC) ){
		VpuWriteReg(BIT_CODE_RUN, 0);
		VpuWriteReg(BIT_CODE_RESET, 1);
		g_usleep(10000);	
	}

	VPURegInit(handle, 0x100, 64);

	codeBuffer = PrcBuf->code_buf;

	cur_pc = VpuReadReg(BIT_CUR_PC);
	
	if ( !cur_pc ){
		/* copy full microcode to code buffer reserved on SDRAM */
		for (i=0; i<sizeof(bit_code)/sizeof(bit_code[0]); i+=4) {
			dataH = (bit_code[i+0] << 16) | bit_code[i+1];
			dataL = (bit_code[i+2] << 16) | bit_code[i+3];
			*(guint *)(codeBuffer + i * 2) = dataL;	
			*(guint *)(codeBuffer + i * 2 + 4) = dataH;		
		}
		//hal_cache_flush(codeBuffer, sizeof(bit_code));
	}else {
		GST_ERROR("After Reset, Cur Pc is not zero\n");
		return RETCODE_FAILURE;
	}

	VpuWriteReg(BIT_WORK_BUF_ADDR, (guint)hal_vaddr_to_paddr((gpointer)(PrcBuf->work_buf)));
	VpuWriteReg(BIT_PARA_BUF_ADDR, (guint)hal_vaddr_to_paddr((gpointer)(PrcBuf->para_buf)));
	VpuWriteReg(BIT_CODE_BUF_ADDR, (guint)hal_vaddr_to_paddr((gpointer)codeBuffer));
	
	VpuWriteReg(BIT_CODE_RUN, 0);
	
	for( i = 0; i < 1024; i++ ) {
		data = bit_code[i];
		VpuWriteReg(BIT_CODE_DOWN, i << 16 | data);
	}


	data = STREAM_ENDIAN;
	data |= STREAM_32ENDIAN << 1;
	data |= STREAM_FULL_EMPTY_CHECK_DISABLE << 2;
	data |= 1 << 3;
	VpuWriteReg(BIT_BIT_STREAM_CTRL, data);

	data = IMAGE_ENDIAN;
	data |= IMAGE_32ENDIAN << 1;
	data |= IMAGE_INTERLEAVE << 2;
	data |= 3 << 3;
	VpuWriteReg(BIT_FRAME_MEM_CTRL, data);
	
	VpuWriteReg(BIT_INT_ENABLE, 0);

	VpuWriteReg(BIT_AXI_SRAM_USE, 0);
	
	VpuWriteReg(BIT_BUSY_FLAG, 1);
	VpuWriteReg(BIT_CODE_RUN, 1);
	#if 1
	while (VpuReadReg(BIT_BUSY_FLAG))
 		;	
	#endif
	
	return RETCODE_SUCCESS;
}


CodecRetCode VPU_GetVersionInfo( DecInstCtl *handle, guint *versionInfo )
{
	guint ver;
	
	if (VpuReadReg(BIT_CUR_PC) == 0)
		return RETCODE_NOT_INITIALIZED;


	VpuWriteReg( RET_VER_NUM , 0 );

	VpuWriteReg( BIT_BUSY_FLAG, 0x1 );
	BitIssueCommand(handle, 0, 0, FIRMWARE_GET );
 	while (VpuReadReg(BIT_BUSY_FLAG))
 			;
		
	ver = VpuReadReg( RET_VER_NUM );
	
	if( ver == 0 )
		return RETCODE_FAILURE;

	*versionInfo = ver;

	return RETCODE_SUCCESS;
}

CodecRetCode VPU_DecOpen(guchar inst_index, DecInstCtl *handle)
{
	CodecRetCode ret;

	if (VpuReadReg(BIT_CUR_PC) == 0){
		return RETCODE_NOT_INITIALIZED;
	}

	ret = CheckDecOpenParam(&handle->SeqOpenParam);
	if (ret != RETCODE_SUCCESS) {
		return ret;
	}

	handle->SeqInfo.initialInfoObtained = 0;
	handle->SeqOpenParam.vc1BframeDisplayValid = 0;

	VpuWriteReg(BIT_RD_PTR_0+inst_index*8 , handle->SeqOpenParam.streamBufStartAddr);
	VpuWriteReg(BIT_WR_PTR_0+inst_index*8 , handle->SeqOpenParam.streamBufStartAddr);
	//LOG_PRINTF("3 Writep %x\n", VpuReadReg(0x124));

	VpuWriteReg(BIT_INT_ENABLE, VPU_ISR_ALL);


	return RETCODE_SUCCESS;
}


CodecRetCode VPU_DecClose(DecInstCtl *handle, guchar inst_index)
{
	BitIssueCommand(handle, inst_index, handle->SeqOpenParam.codecMode, DEC_SEQ_END);
#if 1
	while (VpuReadReg(BIT_BUSY_FLAG))
 			;	
#endif

	return RETCODE_SUCCESS;
}

CodecRetCode VPU_DecSetEscSeqInit( DecInstCtl *handle, gint escape )
{

	VpuWriteReg(BIT_DEC_FUNC_CTRL,  escape & 0x01  );	

	return RETCODE_SUCCESS;
}


CodecRetCode VPU_DecInit(DecInstCtl *handle, guchar inst_index)
{
	DecSeqOpenParam *SeqOpenParam = &handle->SeqOpenParam;
	//DecSeqGetInfo *SeqGetInfo = &handle->SeqInfo;
	guint picSize;
	guint val;
#if 0
	if ( SeqGetInfo->initialInfoObtained) {
		printf("has got the seq info before\n");
		return RETCODE_CALLED_BEFORE;
	}
	#endif

	if ( handle->BitstreamFormat == STD_AVC ) {
		SeqOpenParam->codecMode = AVC_DEC;
	} else if ( handle->BitstreamFormat == STD_VC1) {
		SeqOpenParam->codecMode = VC1_DEC;
	} else if ( handle->BitstreamFormat == STD_MP2 ) {
		SeqOpenParam->codecMode = MP2_DEC;
	} else if ( handle->BitstreamFormat == STD_MP4 ) {
		SeqOpenParam->codecMode = MP4_DEC;
	} else if ( handle->BitstreamFormat == STD_H263 ) {
		SeqOpenParam->codecMode = H263_DEC;
	} else if ( handle->BitstreamFormat == STD_DIV3 ) {
		SeqOpenParam->codecMode = DIV3_DEC;
	} else if ( handle->BitstreamFormat == STD_RV) {
		SeqOpenParam->codecMode = RV_DEC;
	} else if ( handle->BitstreamFormat == STD_AVS ) {
		SeqOpenParam->codecMode = AVS_DEC;
	} else if ( handle->BitstreamFormat == STD_MJPG ) {
		SeqOpenParam->codecMode = MJPG_DEC;
	}

	VpuWriteReg(BIT_RD_PTR_0+inst_index*8 , SeqOpenParam->streamBufStartAddr);
	VpuWriteReg(BIT_EXACT_RD_PTR , SeqOpenParam->streamBufStartAddr);		

	if (DecBitstreamBufEmpty(handle, inst_index)) {
		//printf("ves buf is empty\n");
		//printf("readp is %x\n", (OMX_U32)(VpuReadReg(BIT_RD_PTR_0 + 8*inst_index)));
		return RETCODE_WRONG_CALL_SEQUENCE;
	}


	VPURegInit(handle, 0x180, 32);
	VpuWriteReg(BIT_FRAME_DIS_FLAG_0+4*inst_index, 0);

	VpuWriteReg(CMD_DEC_SEQ_BB_START, SeqOpenParam->streamBufStartAddr);
	VpuWriteReg(CMD_DEC_SEQ_BB_SIZE, SeqOpenParam->streamBufSize/1024); // size in KBytes
	
	if( handle->SeqOpenParam.filePlayEnable == 1 )
		VpuWriteReg(CMD_DEC_SEQ_START_BYTE, SeqOpenParam->streamStartByteOffset);

	val = ((SeqOpenParam->dynamicAllocEnable << 3) & 0x8) | ((SeqOpenParam->filePlayEnable << 2) & 0x4) | ((SeqOpenParam->reorderEnable << 1) & 0x2) | (SeqOpenParam->mp4DeblkEnable & 0x1);	
	VpuWriteReg(CMD_DEC_SEQ_OPTION, val);					

	if( SeqOpenParam->codecMode == AVC_DEC ) {
		VpuWriteReg( CMD_DEC_SEQ_PS_BB_START, SeqOpenParam->psSaveBuffer );
		VpuWriteReg( CMD_DEC_SEQ_PS_BB_SIZE, SeqOpenParam->psSaveBufferSize / 1024 );
	}

	if ( handle->BitstreamFormat == STD_DIV3 ) {
		VpuWriteReg(BIT_RUN_AUX_STD, 1);
	} else  {
		VpuWriteReg(BIT_RUN_AUX_STD, 0);
	}

	/* assume the pic source size is 720*576, the register can be omitted*/
	picSize = 720 << 16 | 576;
	VpuWriteReg( CMD_DEC_SEQ_SRC_SIZE, picSize );
	
	BitIssueCommand(handle, inst_index, SeqOpenParam->codecMode, DEC_SEQ_INIT);

	return RETCODE_SUCCESS;
}


CodecRetCode VPU_DecRegisterFrameBuffer(DecInstCtl *handle, BitPrcBuffer *VPUbuffer, guchar inst_index)
{
	DecSeqGetInfo* GetInfo = &handle->SeqInfo;
	FrameBuffer * bufArray;
	gint i;
	//CodecRetCode ret;
	guint address[150];
	guchar tmp;	
#ifdef USE_SECOND_AXI	
	guint vc1_dbky_size;
#endif

	bufArray = GetInfo->frameBufPool;

	if (!GetInfo->initialInfoObtained) {
		return RETCODE_WRONG_CALL_SEQUENCE;
	}

	if (bufArray == 0) {
		return RETCODE_INVALID_FRAME_BUFFER;
	}

	if (GetInfo->stride < GetInfo->picWidth || GetInfo->stride % 8 != 0 ) {
		//printf("current stride%x\n", (unsigned int)GetInfo->stride);
		return RETCODE_INVALID_STRIDE;
	}
	
	VPURegInit(handle, 0x180, 32);
	for ( i = 0; i < 150; i++ )
		address[i] = 0;

	// Let the decoder know the addresses of the frame buffers.
	for (i = 0; i < GetInfo->numFrameBuffers; i++) {
		address[i*3] =  bufArray[i].bufY;
		address[i*3+1] =  bufArray[i].bufCb;
		address[i*3+2] =  bufArray[i].bufCr;
		if( handle->SeqOpenParam.codecMode == AVC_DEC )
			address[i+96] =  bufArray[i].bufMvCol;
	}

	// Let the decoder know the addresses of the frame buffers.
	tmp = (GetInfo->numFrameBuffers * 3 + 1 ) >> 1;
	for (i = 0; i < tmp; i++) {
		*(guint *)(VPUbuffer->para_buf + i * 2 * 4) =  address[i*2+1];
		*(guint *)(VPUbuffer->para_buf + i * 2 * 4 + 4) =  address[i*2];
	}
	tmp = ( GetInfo->numFrameBuffers + 1 ) >> 1;
	for (i = 0; i < tmp; i++) {
		if( handle->SeqOpenParam.codecMode == AVC_DEC ) {
			*(guint *)(VPUbuffer->para_buf + ( i + 48 ) * 8) =  address[97+i*2];
			*(guint *)(VPUbuffer->para_buf + ( i + 48 ) * 8 + 4) =  address[96+i*2];
		}
	}
	
	if( handle->SeqOpenParam.codecMode != AVC_DEC && handle->SeqOpenParam.codecMode != MP2_DEC && handle->SeqOpenParam.codecMode != MJPG_DEC ) {
		*(guint *)(VPUbuffer->para_buf + 388) = bufArray[0].bufMvCol;
	}

	//hal_cache_flush(VPUbuffer->para_buf, 2048);


	// Tell the decoder how much frame buffers were allocated.
	VpuWriteReg(CMD_SET_FRAME_BUF_NUM, GetInfo->numFrameBuffers);
	VpuWriteReg(CMD_SET_FRAME_BUF_STRIDE, GetInfo->stride);

#ifdef USE_SECOND_AXI
	VPUbuffer->dbky_buf = 0;
	VPUbuffer->dbkc_buf = 0;			
	VPUbuffer->bit_buf = 0;
	VPUbuffer->ipacdc_buf = 0;
	VPUbuffer->ovl_buf = 0;

	//MEMSET(&(GetInfo->second_axi_use), 0, sizeof(GetInfo->second_axi_use));

	switch(handle->BitstreamFormat){
		case STD_AVC:
			GetInfo->second_axi_use.bitpro_use = 1;
			GetInfo->second_axi_use.dbk_use = 1;
			GetInfo->second_axi_use.ipacdc_use = 1;
			GetInfo->second_axi_use.ovl_use = 0;
			VPUbuffer->dbky_buf = SECOND_AXI_BASE_ADDR;
			VPUbuffer->dbkc_buf = VPUbuffer->dbky_buf + DBKY_INTERNAL_BUF_SIZE;			
			VPUbuffer->bit_buf = VPUbuffer->dbkc_buf + DBKC_INTERNAL_BUF_SIZE;
			VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE;						
			break;
		case STD_VC1:
			if ( GetInfo->profile == 0 || GetInfo->profile == 1 ){
				/*Simple and main profile*/
				GetInfo->second_axi_use.bitpro_use = 1;
				GetInfo->second_axi_use.dbk_use = 1;
				if ( GetInfo->picWidth > 1280){
					GetInfo->second_axi_use.ipacdc_use = 0;
					vc1_dbky_size = DBK_INTERNAL_BUF_SIZE_MP_VC1_HD;
					GetInfo->second_axi_use.ovl_use = 0;
				}else{
					GetInfo->second_axi_use.ipacdc_use = 1;
					GetInfo->second_axi_use.ovl_use = 1;
					vc1_dbky_size = DBKY_INTERNAL_BUF_SIZE_MP_VC1;
				}
				VPUbuffer->dbky_buf = SECOND_AXI_BASE_ADDR;
				VPUbuffer->dbkc_buf = VPUbuffer->dbky_buf + vc1_dbky_size;			
				VPUbuffer->bit_buf = VPUbuffer->dbkc_buf + vc1_dbky_size;
				VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE_VC1;	
				VPUbuffer->ovl_buf = VPUbuffer->ipacdc_buf + IPACDC_INTERNAL_BUF_SIZE_VC1;
			}else{
			    /* GetInfo->profile == 2   Advanced profile */
				GetInfo->second_axi_use.dbk_use = 1;
				GetInfo->second_axi_use.ipacdc_use = 0;
				GetInfo->second_axi_use.ovl_use = 0;	
				if ( GetInfo->picWidth > 1280){
					GetInfo->second_axi_use.bitpro_use = 0;
					vc1_dbky_size = DBK_INTERNAL_BUF_SIZE_AP_VC1_HD;
				}else{
					GetInfo->second_axi_use.bitpro_use = 1;		
					vc1_dbky_size = DBKY_INTERNAL_BUF_SIZE_AP_VC1;
				}
				VPUbuffer->dbky_buf = SECOND_AXI_BASE_ADDR;
				VPUbuffer->dbkc_buf = VPUbuffer->dbky_buf + vc1_dbky_size;			
				VPUbuffer->bit_buf = VPUbuffer->dbkc_buf + vc1_dbky_size;
				VPUbuffer->ipacdc_buf = 0;	
				VPUbuffer->ovl_buf = 0;
			}
			break;
		case STD_MP2:
			GetInfo->second_axi_use.bitpro_use = 1;
			GetInfo->second_axi_use.dbk_use = 0;
			GetInfo->second_axi_use.ipacdc_use = 0;
			GetInfo->second_axi_use.ovl_use = 0;
			VPUbuffer->bit_buf = SECOND_AXI_BASE_ADDR;
			break;
		case STD_MP4:
			GetInfo->second_axi_use.bitpro_use = 1;
			GetInfo->second_axi_use.dbk_use = 0;
			GetInfo->second_axi_use.ipacdc_use = 1;
			GetInfo->second_axi_use.ovl_use = 0;
			VPUbuffer->bit_buf = SECOND_AXI_BASE_ADDR;
			VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE;
			//VPUbuffer->ipacdc_buf = SECOND_AXI_BASE_ADDR;
			break;
		case STD_H263:
			GetInfo->second_axi_use.bitpro_use = 1;
			GetInfo->second_axi_use.dbk_use = 1;
			GetInfo->second_axi_use.ipacdc_use = 1;
			GetInfo->second_axi_use.ovl_use = 0;
			VPUbuffer->dbky_buf = SECOND_AXI_BASE_ADDR;
			VPUbuffer->dbkc_buf = VPUbuffer->dbky_buf + DBKY_INTERNAL_BUF_SIZE;			
			VPUbuffer->bit_buf = VPUbuffer->dbkc_buf + DBKC_INTERNAL_BUF_SIZE;
			VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE;						
			break;
		case STD_DIV3:
			GetInfo->second_axi_use.bitpro_use = 1;
			GetInfo->second_axi_use.dbk_use = 0;
			GetInfo->second_axi_use.ipacdc_use = 1;
			GetInfo->second_axi_use.ovl_use = 0;
			VPUbuffer->bit_buf = SECOND_AXI_BASE_ADDR;
			VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE;						
			break;
		case STD_RV:
			GetInfo->second_axi_use.bitpro_use = 1;
			GetInfo->second_axi_use.dbk_use = 1;
			GetInfo->second_axi_use.ipacdc_use = 1;
			GetInfo->second_axi_use.ovl_use = 0;
			#if 1
			VPUbuffer->dbky_buf = SECOND_AXI_BASE_ADDR;
			VPUbuffer->dbkc_buf = VPUbuffer->dbky_buf + DBKY_INTERNAL_BUF_SIZE;			
			VPUbuffer->bit_buf = VPUbuffer->dbkc_buf + DBKC_INTERNAL_BUF_SIZE;
			VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE;		
			#else
			VPUbuffer->bit_buf = SECOND_AXI_BASE_ADDR;
			#endif
			break;
		case STD_AVS:
			GetInfo->second_axi_use.bitpro_use = 1;
			GetInfo->second_axi_use.dbk_use = 1;
			GetInfo->second_axi_use.ipacdc_use = 1;
			GetInfo->second_axi_use.ovl_use = 0;
			VPUbuffer->dbky_buf = SECOND_AXI_BASE_ADDR;
			VPUbuffer->dbkc_buf = VPUbuffer->dbky_buf + DBKY_INTERNAL_BUF_SIZE;			
			VPUbuffer->bit_buf = VPUbuffer->dbkc_buf + DBKC_INTERNAL_BUF_SIZE;
			VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE;						
			break;
		case STD_MJPG:
			GetInfo->second_axi_use.bitpro_use = 0;
			GetInfo->second_axi_use.dbk_use = 0;
			GetInfo->second_axi_use.ipacdc_use = 0;
			GetInfo->second_axi_use.ovl_use = 0;
			break;
		default:
			break;
			
	}

	VpuWriteReg(BIT_AXI_SRAM_USE, GetInfo->second_axi_use.ovl_use << 10 | GetInfo->second_axi_use.dbk_use << 9 | 
		GetInfo->second_axi_use.ipacdc_use << 8 | GetInfo->second_axi_use.bitpro_use << 7 | GetInfo->second_axi_use.ovl_use << 3 | 
		GetInfo->second_axi_use.dbk_use << 2 | GetInfo->second_axi_use.ipacdc_use << 1 | GetInfo->second_axi_use.bitpro_use );


	VpuWriteReg( CMD_SET_FRAME_AXI_BIT_ADDR, VPUbuffer->bit_buf & 0x1FFFFFFF);
	VpuWriteReg( CMD_SET_FRAME_AXI_ACDC_ADDR, VPUbuffer->ipacdc_buf & 0x1FFFFFFF);
	VpuWriteReg( CMD_SET_FRAME_AXI_DBKY_ADDR, VPUbuffer->dbky_buf & 0x1FFFFFFF);
	VpuWriteReg( CMD_SET_FRAME_AXI_DBKC_ADDR, VPUbuffer->dbkc_buf & 0x1FFFFFFF);
	VpuWriteReg( CMD_SET_FRAME_AXI_OVL_ADDR, VPUbuffer->ovl_buf & 0x1FFFFFFF);	

#endif

	if( handle->SeqOpenParam.codecMode == AVC_DEC ) {
		VpuWriteReg( CMD_SET_FRAME_SLICE_BB_START, VPUbuffer->slice_buf & 0x1fffffff );
		VpuWriteReg( CMD_SET_FRAME_SLICE_BB_SIZE, SLICE_BUF_SIZE/1024 );
	}

	BitIssueCommand(handle, inst_index, handle->SeqOpenParam.codecMode, SET_FRAME_BUF);

	return RETCODE_SUCCESS;
}


/* maybe it is useless */

#if 0
CodecRetCode VPU_DecGetBitstreamBuffer( DecOper *operation,
		PhysicalAddress * prdPrt,
		PhysicalAddress * pwrPtr,
		UINT32 * size)
{
	CodecInst * pCodecInst;
	DecInfo * pDecInfo;
	PhysicalAddress rdPtr;
	PhysicalAddress wrPtr;
	UINT32 room;
	CodecRetCode ret;

	pCodecInst = &operation->VPU_handle;
	ret = CheckDecInstanceValidity(pCodecInst);
	if (ret != RETCODE_SUCCESS)
		return ret;


	if ( prdPrt == 0 || pwrPtr == 0 || size == 0) {
		return RETCODE_INVALID_PARAM;
	}

	pDecInfo = &pCodecInst->decInfo;


	rdPtr = VpuReadReg(pDecInfo->streamRdPtrRegAddr);
	wrPtr = pDecInfo->streamWrPtr;
	
	if (wrPtr < rdPtr) {
		room = rdPtr - wrPtr - 1;
	}
	else {
		room = ( pDecInfo->streamBufEndAddr - wrPtr ) + ( rdPtr - pDecInfo->streamBufStartAddr ) - 1;	
	}

	*prdPrt = rdPtr;
	*pwrPtr = wrPtr;
	*size = room;

	return RETCODE_SUCCESS;
}
#endif

CodecRetCode VPU_DecUpdateBitstreamBuffer(DecInstCtl *handle, guchar inst_index, PhysicalAddress wrPtr,guint size)
{
	guint val;

	val = VpuReadReg( (BIT_WR_PTR_0 + 8*inst_index));
	if ( val != (wrPtr & 0xFFFFFE00) ){
		VpuWriteReg( (BIT_WR_PTR_0 + 8*inst_index), wrPtr & 0xFFFFFE00);
		//LOG_PRINTF("1 Writep %x\n", VpuReadReg(0x124));
	}

	if (size == 0) 
	{
		/* the stream has finished */
		VpuWriteReg( (BIT_WR_PTR_0 + 8*inst_index), wrPtr);

		val = VpuReadReg( BIT_DEC_FUNC_CTRL );
		val |= 1 << ( inst_index + 2);
		VpuWriteReg(BIT_DEC_FUNC_CTRL, val);

	}

	return RETCODE_SUCCESS;
}


CodecRetCode VPU_DecStartOneFrame(DecInstCtl *handle, guchar inst_index)
{
	DecSeqOpenParam *SeqOpenParam = &handle->SeqOpenParam;
	DecFrameCfg *param= &handle->FrameCfgParam;
	DecSeqGetInfo* GetInfo = &handle->SeqInfo;
	FrameBuffer * bufArray;
	guint val;//, rot_mir_flag;
	//CodecRetCode ret;

	bufArray = GetInfo->frameBufPool;

	if (handle->SeqInfo.frameBufPool == 0) { // This means frame buffers have not been registered.
		return RETCODE_WRONG_CALL_SEQUENCE;
	}

	if( param->iframeSearchEnable == 1 ) // if iframeSearch is Enable, other bit is ignore;
		val =  param->iframeSearchEnable << 2  & 0x4;
	else
		val =  param->skipframeMode << 3  |  param->iframeSearchEnable << 2  |  param->prescanMode << 1  | param->prescanEnable;
	
	VpuWriteReg( CMD_DEC_PIC_OPTION, val );
	VpuWriteReg( CMD_DEC_PIC_SKIP_NUM, param->skipframeNum );

	if( handle->SeqOpenParam.codecMode == AVC_DEC ) {
		if( SeqOpenParam->reorderEnable == 1 ) {
			VpuWriteReg( BIT_DEC_FUNC_CTRL, param->dispReorderBuf << 1 | VpuReadReg( BIT_DEC_FUNC_CTRL ) );	
		}
	}
	if( handle->SeqOpenParam.codecMode == VC1_DEC ) {
		if( SeqOpenParam->filePlayEnable == 1 ) {
			VpuWriteReg( BIT_DEC_FUNC_CTRL, param->dispReorderBuf << 1 | VpuReadReg( BIT_DEC_FUNC_CTRL ) );	
		}
	}
	else if( handle->SeqOpenParam.codecMode == MP2_DEC ) {
		VpuWriteReg( BIT_DEC_FUNC_CTRL, param->dispReorderBuf << 1 | VpuReadReg( BIT_DEC_FUNC_CTRL ) );	
	}
			
	if( SeqOpenParam->filePlayEnable == 1 )
	{
		VpuWriteReg( CMD_DEC_PIC_CHUNK_SIZE, param->chunkSize );
		if( SeqOpenParam->dynamicAllocEnable == 1 )
			VpuWriteReg( CMD_DEC_PIC_BB_START, param->picStreamBufferAddr );					
		
		VpuWriteReg(CMD_DEC_PIC_START_BYTE, param->picStartByteOffset);
	}


	if ( param->rotConfig.rotFlag || handle->SeqOpenParam.codecMode == MJPG_DEC) {
		VpuWriteReg(CMD_DEC_PIC_ROT_ADDR_Y, param->rotConfig.bufY);
		VpuWriteReg(CMD_DEC_PIC_ROT_ADDR_CB, param->rotConfig.bufCb);
		VpuWriteReg(CMD_DEC_PIC_ROT_ADDR_CR, param->rotConfig.bufCr);
		VpuWriteReg(CMD_DEC_PIC_ROT_STRIDE, param->rotConfig.stride);	 
		VpuWriteReg(CMD_DEC_PIC_ROT_MODE, param->rotConfig.rotFlag);
	} else{
		VpuWriteReg(CMD_DEC_PIC_ROT_MODE, 0);
	}

//	hal_cache_flush(0x89000000, 0x300000);
//	hal_cache_flush_all();
//	hal_cache_invalidate(0x88000000, 0x1800000);
//	hal_icache_invalidate_all();
	
	BitIssueCommand(handle, inst_index, handle->SeqOpenParam.codecMode, DEC_PIC_RUN);
	//LOG_PRINTF("buf used %x\n", VpuReadReg(0x150));

	return RETCODE_SUCCESS;
}


CodecRetCode VPU_DecBitBufferFlush(DecInstCtl *handle, guchar inst_idx)
{
	//CodecRetCode ret;

	BitIssueCommand(handle, inst_idx, handle->SeqOpenParam.codecMode, DEC_BUF_FLUSH);
	

	#if 1
	while (VpuReadReg(BIT_BUSY_FLAG))
		;
	#endif

	VpuWriteReg((BIT_WR_PTR_0+8*inst_idx), handle->SeqOpenParam.streamBufStartAddr);
	//LOG_PRINTF("0 Writep %x\n", VpuReadReg(0x124));

	return RETCODE_SUCCESS;
}

CodecRetCode VPU_DecClrDispFlag( DecInstCtl *handle,  guchar inst_index, guchar frm_index )
{
	DecSeqGetInfo *SeqInfo=&handle->SeqInfo;
	guint ClrFlag;


#if 1
	if (frm_index > (SeqInfo->numFrameBuffers - 1)) {
		return	RETCODE_INVALID_PARAM;
	}

	ClrFlag = ~(1 << frm_index);
#else
	ClrFlag = ~frm_index;
#endif
	ClrFlag &= VpuReadReg(BIT_FRAME_DIS_FLAG_0+4*inst_index);
	VpuWriteReg(BIT_FRAME_DIS_FLAG_0+4*inst_index, ClrFlag);

	return RETCODE_SUCCESS;
	
}

CodecRetCode VPU_DecGiveCommand(DecInstCtl *handle,  guchar inst_index,	CodecCommand cmd, gpointer param)
{

	switch (cmd) 
	{
		case DEC_SET_SPS_RBSP:
			{
				if (handle->SeqOpenParam.codecMode != AVC_DEC) {
					return RETCODE_INVALID_COMMAND;
				}
				if (param == 0) {
					return RETCODE_INVALID_PARAM;
				}
				//SetParaSet(handle, 0, param);
				break;
			}

		case DEC_SET_PPS_RBSP:
			{
				if (handle->SeqOpenParam.codecMode != AVC_DEC) {
					return RETCODE_INVALID_COMMAND;
				}
				if (param == 0) {
					return RETCODE_INVALID_PARAM;
				}
				//SetParaSet(handle, 1, param);
				break;
			}
		case ENABLE_ROTATION:
			handle->FrameCfgParam.rotConfig.rotFlag |= 0x10;
			break;
		case DISABLE_ROTATION:
			handle->FrameCfgParam.rotConfig.rotFlag &=0xFFFFFFEF;
			break;
		case ENABLE_MIRRORING:
			handle->FrameCfgParam.rotConfig.rotFlag |= 0x10;
			break;
		case DISABLE_MIRRORING:
			handle->FrameCfgParam.rotConfig.rotFlag &=0xFFFFFFEF;
			break;
		case ENABLE_DERING:
			handle->FrameCfgParam.rotConfig.rotFlag |= 0x20;
			break;
		case DISABLE_DERING:
			handle->FrameCfgParam.rotConfig.rotFlag &=0xFFFFFFDF;
			break;
		case SET_MIRROR_DIRECTION:
			{
				MirrorDirection info = *(MirrorDirection*)param;		
				handle->FrameCfgParam.rotConfig.rotFlag &= 0xFFFFFFF3;
				switch (info)
				{
					case MIRDIR_VER:
						handle->FrameCfgParam.rotConfig.rotFlag |= 0x4;
						break;
					case MIRDIR_HOR:
						handle->FrameCfgParam.rotConfig.rotFlag |= 0x8;
						break;
					case MIRDIR_HOR_VER:
						handle->FrameCfgParam.rotConfig.rotFlag |= 0xC;
						break;
					default:
						break;
				}
				break;
			}
		case SET_ROTATION_ANGLE:
			{
				RotatorAngle info =  *(RotatorAngle*)param;
				handle->FrameCfgParam.rotConfig.rotFlag &= 0xFFFFFFFC;
				switch(info)
				{
					case ROTANG_90:
						handle->FrameCfgParam.rotConfig.rotFlag |= 1;
						break;
					case ROTANG_180:
						handle->FrameCfgParam.rotConfig.rotFlag |= 2;
						break;
					case ROTANG_270:
						handle->FrameCfgParam.rotConfig.rotFlag |= 3;
						break;
					default:
						break;
				}
				break;
			}
		case SET_ROTATION_OUTPUT:
			{
				FrameBuffer * frame;

				if (param == 0) {
					return RETCODE_INVALID_PARAM;
				}
				frame = (FrameBuffer *)param;
				handle->FrameCfgParam.rotConfig.bufY = frame->bufY;
				handle->FrameCfgParam.rotConfig.bufCb = frame->bufCb;
				handle->FrameCfgParam.rotConfig.bufCr = frame->bufCr;
				break;
			}
		case SET_ROTATION_STRIDE:
			handle->FrameCfgParam.rotConfig.stride = *(guint *)param;
			break;
		default:
			return RETCODE_INVALID_COMMAND;
	}
	return RETCODE_SUCCESS;
}

CodecRetCode VPU_isr_pic_run(DecInstCtl *handle)
{
	DecFrameOutputInfo *FrmOutInfo=&handle->FrameOutInfo;
	DecSeqGetInfo *SeqInfo=&handle->SeqInfo;
	//CodecRetCode		ret;
	guint		val = 0;
	//OMX_S32			i = 0;
	//OMX_U8       index;


	val = VpuReadReg(RET_DEC_PIC_SIZE);
	SeqInfo->picWidth = val >> 16 & 0xffff ;
	SeqInfo->picHeight =  val & 0xffff ;
	
	val = VpuReadReg( RET_DEC_PIC_SUCCESS );
	FrmOutInfo->notSufficientPsBuffer = val >> 3 & 0x1;
	FrmOutInfo->notSufficientSliceBuffer = val >> 2 & 0x1;

	if ( (val & 0x01) == 0) {
		//pendingInst = 0;
		FrmOutInfo->headerdecodingSuccess = 0;
		return RETCODE_FAILURE;
	}

	if ( handle->FrameCfgParam.prescanEnable == 1){
		FrmOutInfo->prescanresult = VpuReadReg(RET_DEC_PIC_OPTION );
		if ( FrmOutInfo->prescanresult == 0 )
			return RETCODE_FRAME_NOT_COMPLETE;			//not enough frame buffer
	}
	else
		FrmOutInfo->prescanresult = 0;	

	FrmOutInfo->indexFrameDisplay = VpuReadReg(RET_DEC_PIC_FRAME_IDX);
	FrmOutInfo->indexFrameDecoded = VpuReadReg(RET_DEC_PIC_CUR_IDX);
	val = VpuReadReg(RET_DEC_PIC_TYPE);
	FrmOutInfo->picType = val & 0x3F;
	//FrmOutInfo->picParam = ( val >> 18 ) & 0x3FF;
	FrmOutInfo->numOfErrMBs = VpuReadReg(RET_DEC_PIC_ERR_MB);

#if 0
	val = VpuReadReg(RET_DEC_PIC_NEXT_IDX);

	for (i = 0 ; i < 3 ; i++) {
		if (i < pDecInfo->initialInfo.nextDecodedIdxNum) {
			info->indexFrameNextDecoded[i] = ((val >> (i * 5)) & 0x1f);
		} else {
			info->indexFrameNextDecoded[i] = -1;
		}
	}
#endif

	if (handle->SeqOpenParam.codecMode == VC1_DEC && FrmOutInfo->indexFrameDisplay != -3) {
		if (handle->SeqOpenParam.vc1BframeDisplayValid == 0) {
			if ( FrmOutInfo->picType == 2) {
				FrmOutInfo->indexFrameDisplay = -3;
			} else {
				handle->SeqOpenParam.vc1BframeDisplayValid = 1;
			}
		}
	}

	#if 0
	GetInstIndex(handle, &index);
	//LOG_PRINTF("%d frame has been decoded %x!\n", (VpuReadReg(RET_DEC_PIC_FRAME_NUM)+1), VpuReadReg(0x120+index*8));
	if ( handle->FrameCfgParam.prescanEnable == 1){
		//if ( FrmOutInfo->prescanresult == 1 )
		{
	//		LOG_PRINTF("***************inst: %d************\n", index);
	//		LOG_PRINTF("%d frame has been decoded %x!\n", (VpuReadReg(RET_DEC_PIC_FRAME_NUM)+1), VpuReadReg(0x120+index*8));			
		}
	} else {
		//LOG_PRINTF("%d frame has been decoded %x!\n", (VpuReadReg(RET_DEC_PIC_FRAME_NUM)+1), VpuReadReg(0x120+index*8));
	}
	#endif
	//pendingInst = 0;

	return RETCODE_SUCCESS;	
}

CodecRetCode VPU_isr_seq_init(DecInstCtl *handle)
{
	DecSeqGetInfo *GetInfo=&handle->SeqInfo;
	guint val, val2;
	//CodecRetCode ret;

	if (VpuReadReg(RET_DEC_SEQ_SUCCESS) == 0)
		return RETCODE_FAILURE;
	
	val = VpuReadReg(RET_DEC_SEQ_SRC_SIZE);
	GetInfo->picWidth =  val >> 16 & 0xffff ;
	GetInfo->picHeight =  val & 0xffff ;

	if ( handle->BitstreamFormat != STD_AVC){
		val = VpuReadReg(RET_DEC_SEQ_SRC_F_RATE);
		GetInfo->frameRateDiv = ((val >> 16 ) & 0xffff) + 1;
		GetInfo->frameRateRes = val & 0xffff;
	}else{
		GetInfo->frameRateDiv = VpuReadReg(RET_DEC_SEQ_NUM_UNITS_IN_TICK);
		GetInfo->frameRateRes = VpuReadReg(RET_DEC_SEQ_TIME_SCALE);
	}
	printf("div %d, res %d\n", GetInfo->frameRateDiv, GetInfo->frameRateRes);

	GetInfo->numFrameBuffers = VpuReadReg(RET_DEC_SEQ_FRAME_NEED);
	//printf("numframebuffers %x\n", (unsigned int)GetInfo->numFrameBuffers);
	
	GetInfo->frameBufDelay = VpuReadReg(RET_DEC_SEQ_FRAME_DELAY);

	GetInfo->profile = VpuReadReg(RET_DEC_SEQ_HEADER_REPORT)&0xff;
	
	if (handle->SeqOpenParam.codecMode == AVC_DEC)
	{
		val = VpuReadReg(RET_DEC_SEQ_CROP_LEFT_RIGHT);	
		val2 = VpuReadReg(RET_DEC_SEQ_CROP_TOP_BOTTOM);
		if( val == 0 && val2 == 0 )
		{
			GetInfo->picCropRect.left = 0;
			GetInfo->picCropRect.right = 0;
			GetInfo->picCropRect.top = 0;
			GetInfo->picCropRect.bottom = 0;
		}
		else
		{
			GetInfo->picCropRect.left = ( val>>10 & 0x3FF )*2;
			GetInfo->picCropRect.right = GetInfo->picWidth -   (val & 0x3FF )*2 ;
			GetInfo->picCropRect.top = ( val2>>10 & 0x3FF )*2;
			GetInfo->picCropRect.bottom = GetInfo->picHeight -  ( val2 & 0x3FF )*2 ;
		}
		
		val = (GetInfo->picWidth * GetInfo->picHeight * 3 / 2) / 1024;
		GetInfo->normalSliceSize = val / 4;
		GetInfo->worstSliceSize = val / 2;
	}else if ( handle->SeqOpenParam.codecMode == MJPG_DEC) {
		VpuWriteReg(CMD_DEC_SEQ_CLIP_MODE, 0);
		VpuWriteReg(CMD_DEC_SEQ_SAM_XY, 0);
		GetInfo->mjpg_sourceFormat = VpuReadReg(RET_DEC_SEQ_JPG_PARA) & 7;
		GetInfo->mjpg_thumbNailEnable = VpuReadReg(RET_DEC_SEQ_JPG_THUMB_IND) & 1;
	}

	GetInfo->initialInfoObtained = 1;

	return RETCODE_SUCCESS;
}

void VPU_isr_seq_end(DecInstCtl *handle) 
{
	//FreeCodecInstance(pCodecInst);

	return;
}

void VPU_isr_flush_buffer(void)
{
    return;
}



