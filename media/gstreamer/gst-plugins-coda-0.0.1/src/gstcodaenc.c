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

 #ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>              /* for fseeko() */
#include <string.h>
#include <sched.h>

#include <gst/gst.h>
#include <gst/video/video.h>

#include "gstcodaenc.h"
#include "coda_enc.h"

#include <glib.h>
#include <glib/gstdio.h>

GST_DEBUG_CATEGORY_STATIC (codaenc_debug);
#define GST_CAT_DEFAULT codaenc_debug
#if 0
static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/mpeg,"
        "width = (int) [ 0, MAX ], " 
        "height = (int) [ 0, MAX ], "
        "framerate = (fraction) [ 0/1, MAX ]")
    );
#endif
static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw-yuv, "     
    "format = (fourcc) { I420, Y42B, Y444, Y800 }, "   
    //"format = (fourcc) I420, "   
    "width = (int) [ 16, 4096 ], "      
    "height = (int) [ 16, 4096 ], "    
    "framerate = (fraction) [ 0/1, 2147483647/1 ]")  
    );

enum
{
  ARG_0,
  ARG_BIT_RATE,
  ARG_GOP_SIZE,
  ARG_ME_METHOD,
  ARG_BUFSIZE,
  ARG_RTP_PAYLOAD_SIZE,
  ARG_CFG_BASE
};

static GstElementClass *parent_class;   /* NULL */

static void gst_coda_enc_base_init (GstCodaEncClass * g_class);
static void gst_coda_enc_class_init (GstCodaEncClass * klass);
static void gst_coda_enc_init (GstCodaEnc * codaenc);

static void gst_coda_enc_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_coda_enc_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

static GstFlowReturn gst_coda_enc_chain (GstPad * pad, GstBuffer * buffer);
static void gst_coda_enc_setcaps (GstPad * pad, GstCaps * caps);
static gboolean gst_coda_enc_getcaps (GstPad * pad, GstCaps * caps);
static gboolean gst_coda_enc_sink_event (GstPad * pad, GstEvent * event);
static gboolean gst_coda_enc_src_event (GstPad * pad, GstEvent * event);
static GstStateChangeReturn gst_coda_enc_change_state (GstElement * element,
    GstStateChange transition);
static gboolean gst_coda_enc_src_query (GstPad * pad, GstQuery * query);
static void gst_coda_enc_negotiate (GstCodaEnc * enc, gint width, gint height);
static void gst_coda_enc_loop(void *param);
static void gst_coda_enc_frame(GstCodaEnc *enc);
static void copydata2buffer(GstCodaEnc * enc, guchar *data, guint size, GstClockTime pts);
static gboolean gst_coda_enc_finish(GstCodaEnc *enc);

GType
gst_coda_enc_get_type (void)
{
  static GType type = 0;

  if (!type) {
    static const GTypeInfo coda_enc_info = {
      sizeof (GstCodaEncClass),
      (GBaseInitFunc) gst_coda_enc_base_init,
      NULL,
      (GClassInitFunc) gst_coda_enc_class_init,
      NULL,
      NULL,
      sizeof (GstCodaEnc),
      0,
      (GInstanceInitFunc) gst_coda_enc_init,
    };

    type = g_type_register_static (GST_TYPE_ELEMENT, "GstCodaEnc",
        &coda_enc_info, 0);
  }
  return type;
}

static void vbv_update_writep(Vbv_Ctrl *pvbv, guint new_writep)
{
	pvbv->writep = new_writep;
	//pvbv->valid_size = (pvbv->writep >= pvbv->readp)?(pvbv->writep - pvbv->readp):(pvbv->writep - pvbv->start + pvbv->end + 1 - pvbv->readp);
	if (pvbv->writep == pvbv->readp)
		pvbv->valid_size = 0;
	else
		pvbv->valid_size = (pvbv->writep >= pvbv->readp)?(pvbv->end+1-pvbv->start+pvbv->readp- pvbv->writep):(pvbv->readp - pvbv->writep);
	
}

static gboolean vbv_get_valid(Vbv_Ctrl *pvbv)
{
	if(pvbv->valid_size > VBV_RESERVE_DATA )
		return TRUE;
	else
		return FALSE;
}

static gint gst_coda_enc_allocate_buffer(GstCodaEnc *enc)
{
	//struct video_picture_meta* pmeta;	
	struct venc_private *ppriv = (struct venc_private *)enc->priv;
	EncInstCtl *handle = ppriv->inst_handle;
	EncOpenParam *OpenParam=&handle->EncSeqOpenParam;
	EncInitialInfo *EncGetInfo=&handle->initialInfo;
	EncInfo *info=&handle->pEncInfo;
	Frm_Ctrl *pfrm_ctrl=&ppriv->frm_ctrl;
//	GstBuffer *inbuf = NULL;
//	GstFlowReturn ret = GST_FLOW_OK;
	guint frm_y_size, frm_c_size, frm_mv_size, height;
	guchar i, bufcnt;
	guint frm_addr = VENC_FRAME_BUF_ADDR;

	printf("coda encoder allocate frame buffer\n");
	//GetInfo->stride = lakers_frm_y_size =  (GetInfo->picWidth + 255 )>> 8 <<8;	//y: 256 bytes aligned
	EncGetInfo->stride = frm_y_size =  OpenParam->picWidth;
	height = ( OpenParam->picHeight + 15 ) >> 4 << 4;		// 16 bytes aligned
//	GetInfo->picHeight = height;
	frm_y_size *= height;

	if ( handle->BitstreamFormat != STD_MJPG ) {
		/* 4:2:0 */
		frm_c_size = frm_y_size >> 2;
		if ( handle->BitstreamFormat != STD_MP2)
			frm_mv_size = (MAX_FRAME_MVL_STRIDE * MAX_FRAME_MVL_HEIGHT) >> 2;
		else
			frm_mv_size = 0;
		bufcnt = EncGetInfo->minFrameBufferCount + 3;
	} else {
	#if 0
		/* mjpeg format */
		if ( OpenParam->mjpgChromaFormat == YCBCR420) {
			frm_c_size = lakers_frm_y_size >> 2;
		} else if ( OpenParam->mjpgChromaFormat == YCBCR422H || OpenParam->mjpgChromaFormat == YCBCR422V ) {
			frm_c_size = lakers_frm_y_size >> 1;
		} else if ( OpenParam->mjpgChromaFormat == YCBCR444 ) {
			frm_c_size = lakers_frm_y_size;
		} else if ( OpenParam->mjpgChromaFormat == YCBCR400 ) {
			frm_c_size = 0;
		}
	#endif
		frm_mv_size = 0;
		bufcnt = EncGetInfo->minFrameBufferCount + 1;
	}

	enc->insize = frm_y_size + 2 * frm_c_size;
	for ( i = 0; i < bufcnt; i++){
		pfrm_ctrl->frm_info[i].used = 0;
		pfrm_ctrl->frm_info[i].size[0] = frm_y_size;
		pfrm_ctrl->frm_info[i].size[1] = frm_c_size;
		pfrm_ctrl->frm_info[i].size[2] = frm_c_size;
		pfrm_ctrl->frm_info[i].size[3] = frm_mv_size;
		pfrm_ctrl->frm_info[i].frm_buffer.bufY = frm_addr;
		pfrm_ctrl->frm_info[i].frm_buffer.bufCb = pfrm_ctrl->frm_info[i].frm_buffer.bufY + frm_y_size;
		pfrm_ctrl->frm_info[i].frm_buffer.bufCr = pfrm_ctrl->frm_info[i].frm_buffer.bufCb + frm_c_size;
		pfrm_ctrl->frm_info[i].frm_buffer.bufMvCol = pfrm_ctrl->frm_info[i].frm_buffer.bufCr + frm_c_size;
		info->frameBufPool[i] = pfrm_ctrl->frm_info[i].frm_buffer;
		frm_addr = pfrm_ctrl->frm_info[i].frm_buffer.bufMvCol + frm_mv_size;
	}

	pfrm_ctrl->max_enc_frame_num = bufcnt;
	

    return 1;
}

static void gst_coda_enc_frame(GstCodaEnc *enc)
{
  struct venc_private *ppriv = (struct venc_private *)enc->priv;
  EncInstCtl *handle = ppriv->inst_handle;  
  EncInitialInfo *EncGetInfo=&handle->initialInfo;
  Vbv_Ctrl *pvbv_ctrl = &ppriv->vbv_ctrl;
  Frm_Ctrl *pfrm_ctrl = &ppriv->frm_ctrl;
  struct inlbuf *inframebuf;
  GstClockTime pts, duration;
  PhysicalAddress subSampBaseA, subSampBaseB;
  gint cur_writep;
  guchar frm_idx;//, i; 
//  GstBuffer *outbuf;
//  GstFlowReturn retflow = GST_FLOW_OK;
  CodecRetCode ret;
  
  if ( ppriv->enc_status ==  VPU_STOP_STATE || ppriv->enc_status == IDLE_STATE) {
	return;
  }

  if(ppriv->enc_status == SEQ_START_STATE) {
	/* check the decoder working status */
	if ( vbv_get_valid(pvbv_ctrl) ) {
		//ppriv->start_time = hal_get_cpu_tick();
		pvbv_ctrl->readp = pvbv_ctrl->start;
		pvbv_ctrl->writep = pvbv_ctrl->start;
		pvbv_ctrl->valid_size = pvbv_ctrl->end + 1 - pvbv_ctrl->start;
		//printf("INIT readp %x\n", pvbv->readp);
		if ( pvbv_ctrl->valid_size ){
			VPU_EncInit(handle, ppriv->instIdx);
			if ( handle->BitstreamFormat == STD_MJPG)
				ppriv->enc_status = SET_FRAMEBUF_STATE;
			else{
				ppriv->enc_status = SEQ_CONTINUE_STATE;
				return;
			}
		} 
	} 
	

  }

  if (ppriv->enc_status == SET_FRAMEBUF_STATE) {
	gst_coda_enc_allocate_buffer(enc);
	if(handle->BitstreamFormat != STD_MJPG ) {
	    subSampBaseA = pfrm_ctrl->frm_info[EncGetInfo->minFrameBufferCount+1].frm_buffer.bufY;
	    subSampBaseB = pfrm_ctrl->frm_info[EncGetInfo->minFrameBufferCount+2].frm_buffer.bufY;
	    // Register frame buffers requested by the encoder.
	    ret = VPU_EncRegisterFrameBuffer(handle, &ppriv->VPU_buffer, ppriv->instIdx, subSampBaseA, subSampBaseB);
	    if( ret != RETCODE_SUCCESS ) {
	        printf( "VPU_EncRegisterFrameBuffer failed Error code is 0x%x \n", ret );
	        return;
	    }
		ppriv->enc_status = SET_FRAMEBUF_CONTINUE_STATE;
		return;
	}else{
		ppriv->enc_status = ENC_PUT_HEADER;
	}
	
  }

  if (ppriv->enc_status == ENC_PUT_HEADER) {
	VPU_EncPutHeader(handle, ppriv->instIdx);
	cur_writep = GetNewWritePtr(handle, ppriv->instIdx, 0);
	g_mutex_lock(enc->buflock);
	vbv_update_writep(pvbv_ctrl, cur_writep);
	g_mutex_unlock(enc->buflock);	
	ppriv->enc_status = ENC_HEADER_DONE;
	return;
  }	

  if (ppriv->enc_status == PICFRM_START_STATE) {
	if ( ppriv->encoded_first_frame != 0  ) {
		inframebuf = pvbv_ctrl->databuf;
		if ( inframebuf ){
			pts = GST_BUFFER_TIMESTAMP (inframebuf->inbuf);
			duration = GST_BUFFER_DURATION (inframebuf->inbuf);  

  			frm_idx = EncGetInfo->minFrameBufferCount;
			memcpy((gpointer)(hal_paddr_to_vaddr((gpointer)pfrm_ctrl->frm_info[frm_idx].frm_buffer.bufY)),(gpointer)(GST_BUFFER_DATA(inframebuf->inbuf)), enc->insize);
			pfrm_ctrl->frm_info[frm_idx].pts = pts;
			pfrm_ctrl->frm_info[frm_idx].duration = duration;
			
	    	pvbv_ctrl->databuf = inframebuf->next;
	    	gst_buffer_unref(inframebuf->inbuf);  
	    	g_free(inframebuf);
	    	inframebuf = pvbv_ctrl->databuf;
		}else{
			return;
		}
	  //printf("%x\n", (guint)streambuf);	
	}

	ppriv->enc_status = PICBIT_START_STATE;
	
  }

  if(ppriv->enc_status == PICBIT_START_STATE) {
	if (vbv_get_valid(pvbv_ctrl)) {

		//SetDecodeFrameParam(&dec_handle->FrameCfgParam);
		//ppriv->end_time = hal_get_cpu_tick();
		//ppriv->start_time = hal_get_cpu_tick();
		//starttime = OsclTickCount::TickCount();
//		start = g_thread_gettime();
//	    tfstart = gst_util_get_timestamp ();
		VPU_EncStartOneFrame(handle, ppriv->instIdx);
		//pdec_inst->wait_finish = 1;
		ppriv->enc_status = PICBIT_CONTINUE_STATE;
	} 
	return;
  }
  
}


static gboolean VideoFrameInput(GstCodaEnc * enc, GstBuffer * buf)
{
  struct venc_private *ppriv= (struct venc_private *)enc->priv;
  EncInstCtl *handle= ppriv->inst_handle;  
  Vbv_Ctrl *pvbv= &ppriv->vbv_ctrl;
  struct inlbuf *indatabuf;
  Frm_Ctrl *pfrm_ctrl = &ppriv->frm_ctrl;
  guchar frmidx;
//  guint readp;
  guchar *input_data;
  guint input_size;
//  static guint size = 0;
  GstClockTime bufpts, bufduration;

  input_size = GST_BUFFER_SIZE(buf);
  input_data = GST_BUFFER_DATA(buf);
  bufpts = GST_BUFFER_TIMESTAMP (buf);
  bufduration = GST_BUFFER_DURATION (buf);
  //printf("pts %lld, dur %lld\n",bufpts, bufduration);
  GST_DEBUG("pts %lld, dur %lld\n",bufpts, bufduration);

  if ( ppriv->enc_status == ENC_HEADER_DONE){
  	frmidx = handle->initialInfo.minFrameBufferCount;
	memcpy((gpointer)(hal_paddr_to_vaddr((gpointer)pfrm_ctrl->frm_info[frmidx].frm_buffer.bufY)),(gpointer)(GST_BUFFER_DATA(buf)), enc->insize);

    ppriv->enc_status = PICFRM_START_STATE;
	gst_coda_enc_frame(enc);
  }else{
    if ( pvbv->databuf == NULL ){
  	pvbv->databuf = (struct inlbuf *)g_malloc0(sizeof(struct inlbuf));
	if ( pvbv->databuf ){
	  pvbv->databuf->inbuf = buf;
	  pvbv->databuf->next = NULL;
	 // printf("add buf0 %x\n", input_size);
	}else{
	  printf("enc error to allocate memory0\n");
	}
  } else {
  	indatabuf = pvbv->databuf;
    while ( indatabuf->next ){
		indatabuf = indatabuf->next;
    }
	indatabuf->next = (struct inlbuf *)g_malloc0(sizeof(struct inlbuf));
	if ( indatabuf->next ){
	  indatabuf->next->inbuf = buf;
	  indatabuf->next->next = NULL;
	  //printf("add %x, %x\n", input_size, buf);
	}else{
	  printf("enc error to allocate memory1\n");
	}
  }

  }
  

  return TRUE;

}

static GstFlowReturn gst_coda_enc_chain (GstPad * pad, GstBuffer * buf)
{
  GstCodaEnc *enc;
 // DecInstCtl *handle;  
#ifndef GST_DISABLE_GST_DEBUG
 // guchar *data;
#endif
//  guchar *outdata;
//  guchar *base[3], *last[3];
//  guint img_len, outsize;
//  gint width, height;
//  gint r_h, r_v;
//  guint code, hdr_ok;
//  GstFlowReturn res;

  enc = GST_CODA_ENC (GST_PAD_PARENT (pad));
//  ppriv = (struct vdec_private *)dec->priv;
//  handle = ppriv->inst_handle;

//  printf("chain buf %x, int %x\n", (guint)buf, *(guint *)(0x5f400174));
  g_mutex_lock (enc->buflock);
  VideoFrameInput(enc, buf);
  g_mutex_unlock (enc->buflock);
  
  //gst_pad_start_task(dec->sinkpad, (GstTaskFunction)gst_coda_dec_loop, dec->sinkpad);
  //gst_coda_dec_loop(dec);

//  printf("chain finish\n");
 // frame_free(dec);
  
  return GST_FLOW_OK;  

}


static void enc_frame_output(GstCodaEnc *enc)
{
	struct venc_private *ppriv =(struct venc_private *)enc->priv;
	EncInstCtl *enc_handle = ppriv->inst_handle;
	Frm_Ctrl *pfrm_ctrl = &ppriv->frm_ctrl;
    Vbv_Ctrl *pvbv_ctrl = &ppriv->vbv_ctrl ;
    GstBuffer *outbuf = NULL;
	guchar index;
//	GstClockTime duration;
	GstFlowReturn ret = GST_FLOW_OK;	
	
	outbuf = gst_buffer_new();
	GST_BUFFER_DATA(outbuf) = (gpointer)(hal_paddr_to_vaddr((gpointer)pvbv_ctrl->readp));
	if ( pvbv_ctrl->writep > pvbv_ctrl->readp)
		GST_BUFFER_SIZE(outbuf) = pvbv_ctrl->writep - pvbv_ctrl->readp;
	else
		GST_BUFFER_SIZE(outbuf) = pvbv_ctrl->end + 1 - pvbv_ctrl->start- pvbv_ctrl->writep + pvbv_ctrl->readp;

	index = enc_handle->initialInfo.minFrameBufferCount;
	GST_BUFFER_TIMESTAMP(outbuf) = pfrm_ctrl->frm_info[index].pts;
    GST_BUFFER_DURATION (outbuf) = pfrm_ctrl->frm_info[index].duration;

#if 1
//	test = (guint)GST_MINI_OBJECT_REFCOUNT_VALUE(GST_MINI_OBJECT_CAST(outbuf));

	//pushstart = gst_util_get_timestamp ();

//	printf("outbuf push start\n");

    ret = gst_pad_push (enc->srcpad, outbuf);

	GST_DEBUG("pad push ret %x\n", (gint)ret);
		{
			GstClockTime ppts;
			
			ppts = GST_BUFFER_TIMESTAMP(outbuf);
			GST_DEBUG("ppts %lld\n", ppts);
    	}
      //pushend = gst_util_get_timestamp ();

      //diff = GST_CLOCK_DIFF (pushstart, pushend);

	  //GST_DEBUG ("64bit push time %lld ns.\n", diff);

	  //g_usleep(1000000);

#endif
}


static void VideoEncoder_Init(GstCodaEnc * enc)
{
  struct venc_private *ppriv;
  EncConfigParam *EncConfParam=&ppriv->ConfParam;
  EncInstCtl *handle;
  CodecRetCode ret;
  guint vpu_addr;
  guchar i;

  ppriv = (struct venc_private *)g_malloc0(sizeof(struct venc_private));
  enc->priv = (gpointer)ppriv;

  handle = (EncInstCtl *)g_malloc0(sizeof(EncInstCtl));
  ppriv->inst_handle = handle;

  /* Init the memory and reg */
  bsp_postboot_init();

  handle->RegBase = (guint)hal_paddr_to_vaddr((gpointer)VENC_REG_BASE_ADDR);
  vpu_addr = (guint)hal_paddr_to_vaddr((gpointer)VENC_MEM_BASE_ADDR);
  ppriv->vbv_ctrl.start= VENC_MEM_BASE_ADDR + TOTAL_VPUBUF_SIZE;
  ppriv->vbv_ctrl.end = ppriv->vbv_ctrl.start + VENC_STREAM_BUF_SIZE - 1;
  ppriv->frm_ctrl.base_addr = ppriv->vbv_ctrl.start + VENC_STREAM_BUF_SIZE;
  ppriv->frm_ctrl.size = VENC_FRAME_BUF_ADDR;
  ppriv->frm_ctrl.mv_base_addr = VENC_MV_BUF_ADDR;
  
  ppriv->VPU_buffer.code_buf = vpu_addr;
  ppriv->VPU_buffer.work_buf = ppriv->VPU_buffer.code_buf + CODE_BUF_SIZE;
  ppriv->VPU_buffer.para_buf = ppriv->VPU_buffer.work_buf + WORK_BUF_SIZE + PARA_BUF2_SIZE;
  ppriv->VPU_buffer.slice_buf = ppriv->VPU_buffer.para_buf + PARA_BUF_SIZE;
  ppriv->VPU_buffer.ps_buf[0] = ppriv->VPU_buffer.slice_buf + SLICE_BUF_SIZE;
  for ( i = 1; i < MAX_NUM_INSTANCE; i++) {
	 ppriv->VPU_buffer.ps_buf[i] =  ppriv->VPU_buffer.ps_buf[i-1] + PS_BUF_SIZE;
  }

  VPU_Init(&(ppriv->VPU_buffer), handle, 0);

  EnableVPUInt(handle, 0);
  ppriv->instIdx = 0;
  //ppriv->decoded_first_frame = 0;
  ppriv->frm_ctrl.disp_flag = 0xff;
  ppriv->frm_ctrl.pre_idx = ppriv->frm_ctrl.disp_idx;
  ppriv->frm_ctrl.disp_idx = 0xff;
  ppriv->frm_ctrl.clr_flag = 0;
  ppriv->frm_ctrl.IPframe = 0;
  ppriv->frm_ctrl.output_en = 0;
  ppriv->frm_ctrl.last_enc_frame = 1;
  ppriv->frm_ctrl.outbuf = NULL;
  ppriv->frm_ctrl.get_pts_valid = 0;

  ppriv->vbv_ctrl.readp = ppriv->vbv_ctrl.writep = ppriv->vbv_ctrl.start;
  ppriv->vbv_ctrl.config_writep = ppriv->vbv_ctrl.start;
  ppriv->vbv_ctrl.valid_size = 0;
  ppriv->vbv_ctrl.config_validsize = 0;
  ppriv->vbv_ctrl.vbv_vo_status = 0;
  ppriv->vbv_ctrl.first_data = 1;
  ppriv->vbv_ctrl.databuf = NULL;
  ppriv->enc_status = IDLE_STATE;
  ppriv->stop_status = IDLE_STATE;

  g_atomic_int_set(&(ppriv->vbv_ctrl.vo_status), 0);

  handle->EncSeqOpenParam.bitstreamBuffer= ppriv->vbv_ctrl.start;
  handle->EncSeqOpenParam.bitstreamBufferSize = ppriv->vbv_ctrl.end + 1 - ppriv->vbv_ctrl.start;
  handle->pEncInfo.streamBufEndAddr = handle->EncSeqOpenParam.bitstreamBuffer + handle->EncSeqOpenParam.bitstreamBufferSize;
  //handle->pEncInfo.filePlayEnable = 0;
  handle->pEncInfo.dynamicAllocEnable = 0;
  handle->pEncInfo.ringBufferEnable = 1;

  SetEncOpenParamDefault(handle->BitstreamFormat, &(handle->EncSeqOpenParam), EncConfParam);
  
  ret = VPU_EncOpen(ppriv->instIdx, handle);
  if ( ret != RETCODE_SUCCESS){
  	printf("coda encoder open failure\n");
	return;
  }
  
  ppriv->enc_status = SEQ_START_STATE;
  gst_coda_enc_frame(enc);	
  

  //SetTiledMapType(LINEAR_FRAME_MAP);

  //sync_reset(&ppriv->sync_ctrl);
  //ppriv->sync_ctrl.block[ppriv->sync_ctrl.wr].start_addr = ppriv->vbv_ctrl.start;

  ppriv->wait_finish = 0;

}

static void enc_seq_init_isr(GstCodaEnc *enc)
{
	struct venc_private *ppriv = (struct venc_private *)enc->priv;
	EncInstCtl *handle = ppriv->inst_handle;  
	//struct sync_block* block;
    Vbv_Ctrl *pvbv_ctrl = &ppriv->vbv_ctrl;
//	struct coda_sync_ctrl *sync_ctrl= &ppriv->sync_ctrl;
//    Frm_Ctrl *pfrm_ctrl = &ppriv->frm_ctrl;
	CodecRetCode ret;
//	guchar i;
	guint cur_writep;

	if ( ppriv->enc_status == WAITFOR_STOP_STATE) {
		ppriv->stop_status = VPU_STOP_STATE;
		return;
	}

	if  ( ppriv->enc_status != SEQ_CONTINUE_STATE){
		printf("ENC SEQ IRQ ERR!\n");
		return;
	}

	
	ret = VPU_isr_enc_seq_init(handle);
	if (ret == RETCODE_SUCCESS ){
		g_mutex_lock(enc->buflock);
		ppriv->enc_status = SET_FRAMEBUF_STATE;
		g_mutex_unlock(enc->buflock);		
	}
	else{
		g_mutex_lock(enc->buflock);
		ppriv->enc_status = SEQ_START_STATE;
		g_mutex_unlock(enc->buflock);
		return;
	}

	cur_writep = GetNewWritePtr(handle, ppriv->instIdx, 0);
	g_mutex_lock(enc->buflock);
	vbv_update_writep(pvbv_ctrl, cur_writep);
	if (pvbv_ctrl->vbv_vo_status == 1){
		sem_post(&enc->push_sem);
		pvbv_ctrl->vbv_vo_status = 0;
	}	
	GST_LOG("seq init writep %x\n", (unsigned int)cur_writep);
	g_mutex_unlock(enc->buflock);

	return;
}

static void enc_set_framebuffer_isr(GstCodaEnc *enc)
{
	struct venc_private *ppriv=(struct venc_private *)enc->priv;
	CodecRetCode ret;

	g_mutex_lock(enc->buflock);
	if ( ppriv->enc_status == WAITFOR_STOP_STATE) {
		ppriv->stop_status = VPU_STOP_STATE;
		g_mutex_unlock(enc->buflock);
		return;
	}

	ret = VPU_isr_enc_reg_framebuf(ppriv->inst_handle);
	if ( ret != RETCODE_SUCCESS){
		printf("enc set frame buffer failure\n");
		ppriv->enc_status = SET_FRAMEBUF_STATE;
		g_mutex_unlock(enc->buflock);
		return;
	}
	
	VPURegInit(ppriv->inst_handle, 0x180, 32, 0);
	ppriv->enc_status = ENC_PUT_HEADER;

	g_mutex_unlock(enc->buflock);
	
	return;
}

static void enc_pic_run_isr(GstCodaEnc *enc)
{
	struct venc_private *ppriv =(struct venc_private *)enc->priv;
	EncInstCtl *handle=ppriv->inst_handle;  
    Vbv_Ctrl *pvbv_ctrl;
    Frm_Ctrl *pfrm_ctrl;
	guint cur_writep;
	//guint cur_pts;
	CodecRetCode ret;
	//guchar ii, num;
	//gpointer addr;

	g_mutex_lock(enc->buflock);
	if ( ppriv->enc_status == WAITFOR_STOP_STATE) {
		ppriv->stop_status = VPU_STOP_STATE;
		g_mutex_unlock(enc->buflock);
		GST_LOG("Enc PIC run stop!\n");
		return;
	}

	if ( ppriv->enc_status != PICBIT_CONTINUE_STATE) {
		g_mutex_unlock(enc->buflock);
		GST_LOG("Enc PIC RUN IRQ ERR!\n");
		return;
	}
	g_mutex_unlock(enc->buflock);

	ret = VPU_isr_enc_pic_run(handle);
	if ( ret != RETCODE_SUCCESS) {
		ppriv->enc_status = PICFRM_START_STATE;
		return;
	} 

    pvbv_ctrl = &ppriv->vbv_ctrl;
    pfrm_ctrl = &ppriv->frm_ctrl;	

	cur_writep = GetNewWritePtr(handle, ppriv->instIdx, 0);
	g_mutex_lock(enc->buflock);
	vbv_update_writep(pvbv_ctrl, cur_writep);
	if (pvbv_ctrl->vbv_vo_status == 1){
		sem_post(&enc->push_sem);
		pvbv_ctrl->vbv_vo_status = 0;
	}
	g_mutex_unlock(enc->buflock);

	enc_frame_output(enc);

	pvbv_ctrl->readp = pvbv_ctrl->writep;
	pvbv_ctrl->valid_size = pvbv_ctrl->end + 1 - pvbv_ctrl->start;

	g_mutex_lock(enc->buflock);
	ppriv->enc_status = PICFRM_START_STATE;
	g_mutex_unlock(enc->buflock);


	return;

}

static gboolean gst_coda_enc_isr(GstCodaEnc *enc)
{
  struct venc_private *ppriv=(struct venc_private *)enc->priv;
  EncInstCtl *handle=ppriv->inst_handle;  
  Vbv_Ctrl *pvbv_ctrl=&ppriv->vbv_ctrl;
//  Frm_Ctrl *pfrm_ctrl=&ppriv->frm_ctrl;
  guint status;//, cur_writep;  

  if ( handle->BitstreamFormat != STD_MJPG )
	  status = GetVPUIntStatus(handle, 0);
  else
  	  status = JPU_GetStatus(handle, 0);
	
  if (status == 0)
	return FALSE;

  if ( handle->BitstreamFormat != STD_MJPG ){
    if(status & VPU_ISR_SEQ_INIT) {	
	  ClrVPUIntStatus(handle, status, VPU_ISR_SEQ_INIT, 0);
  	  ClrVPUBitInt(handle, 0);
	  enc_seq_init_isr(enc);
    }
	#if 0
    if(status & VPU_ISR_SEQ_END) {
	  seq_end_isr(enc);
	  ClrVPUIntStatus(handle, status, VPU_ISR_SEQ_END);
	  ClrVPUBitInt(handle, 0);
    }
	#endif
    if(status & VPU_ISR_PIC_RUN) {
//	  double usedtime;
	//  usedtime = OsclTickCount::TicksToMsec(OsclTickCount::TickCount()) - OsclTickCount::TicksToMsec(starttime);
	//  printf("dec time cycle %f msec\n", usedtime);
	//  ppriv->start_time = hal_get_cpu_tick();
	  ClrVPUIntStatus(handle, status, VPU_ISR_PIC_RUN, 1);
	  ClrVPUBitInt(handle, 0);
	  enc_pic_run_isr(enc);	
	  if ( pvbv_ctrl->vbv_vo_status == 1 ){
		  //vdec_lakers_bsbuffer_flush(ppriv);
		  //return TRUE;
	  }		
    }
    if(status & VPU_ISR_SET_FRAME_BUFFER) {
	  ClrVPUIntStatus(handle, status, VPU_ISR_SET_FRAME_BUFFER, 0);
	  ClrVPUBitInt(handle, 0);
	  enc_set_framebuffer_isr(enc);
    }
    #if 0
    if (status & VPU_ISR_PARA_SET) {
	  coda_ctrl->VPU_isr_para_set();
	  coda_ctrl->ClrVPUIntStatus(dec_handle, status, VPU_ISR_PARA_SET);
    }
    #endif
    #if 0
    if (status & VPU_ISR_BUF_FLUSH) {
	  VPU_isr_flush_buffer();
	  pvbv_ctrl->readp = pvbv_ctrl->start;
	  pvbv_ctrl->valid_size = 0;
	  //vdec_lakers_sync_reset (sync_ctrl);

 	  ClrVPUIntStatus(handle, status, VPU_ISR_BUF_FLUSH);
	  ClrVPUBitInt(handle, 0);

	  //pvbv->readp = pvbv->start;
	  pvbv_ctrl->valid_size = pvbv_ctrl->writep - pvbv_ctrl->readp;
	  pvbv_ctrl->config_end = get_config_end(pvbv_ctrl);
	  pvbv_ctrl->vbv_vo_status = 0;

	  VPU_DecUpdateBitstreamBuffer(handle, ppriv->instIdx, pvbv_ctrl->config_end, 1);
	  #if 1
	  ppriv->dec_status = SEQ_START_STATE;
	  ppriv->decoded_first_frame = 0;
	  pfrm_ctrl->disp_flag = 0xff;
	  pfrm_ctrl->pre_idx = pfrm_ctrl->disp_idx;
	  #else
	  if (dec_handle->BitstreamFormat == STD_MJPG)
	    ppriv->dec_status = VDEC_LAKERS_SEQ_START;
	  else {
	    ppriv->dec_status = VDEC_LAKERS_PICFRM_START;
	    ppriv->decoded_first_frame = 0;
	  }
	  #endif

    }
	  #endif
	  #if 0
    if (status & VPU_ISR_BUF_EMPTY) {
	  ClrVPUIntStatus(handle, status, VPU_ISR_BUF_EMPTY);
	  ClrVPUBitInt(handle, 0);
	  bsbuffer_underflow_isr(enc);
    }
	#endif
  }else{
    if ( status & VPU_ISR_JPG_DONE ){
		JPU_ClrStatus(handle, 0, VPU_ISR_JPG_DONE);
		enc_pic_run_isr(enc);	
    }
  }

  return TRUE;
  

}


static gboolean gst_coda_enc_status(EncInstCtl *enc_handle)
{
	if ( enc_handle->BitstreamFormat != STD_MJPG ){
		if (VPU_IsBusy(enc_handle, 0))
			return FALSE;
	}else{
		if (JPU_IsBusy(enc_handle, 0))
			return FALSE;
	}

	/* coda is not busy */
	return TRUE;
}


static void gst_coda_enc_loop(void *param)
{
  GstCodaEnc *enc = (GstCodaEnc *)param;
  struct venc_private *ppriv=(struct venc_private *)enc->priv;
  EncInstCtl *handle=ppriv->inst_handle;  
//  Vbv_Ctrl *pvbv_ctrl=&ppriv->vbv_ctrl;
//  Frm_Ctrl *pfrm_ctrl=&ppriv->frm_ctrl;
  #if 1
  	{

  if (gst_coda_enc_status(handle) == TRUE )
  {
	gst_coda_enc_isr(enc);
  }

  g_mutex_lock(enc->buflock);
  gst_coda_enc_frame(enc);
  g_mutex_unlock(enc->buflock);

  	}
  #endif
}

static GstCaps *
gst_codaenc_vid_caps_new (const char *mimetype, const char *fieldname, ...)
{
  GstStructure *structure = NULL;
  GstCaps *caps = NULL;
  va_list var_args;
  gint i;

  if (!caps) {
    GST_DEBUG ("Creating default caps");
    caps = gst_caps_new_simple (mimetype,
        "width", GST_TYPE_INT_RANGE, 16, 4096,
        "height", GST_TYPE_INT_RANGE, 16, 4096,
        "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT, 1, NULL);
  }

  for (i = 0; i < gst_caps_get_size (caps); i++) {
    va_start (var_args, fieldname);
    structure = gst_caps_get_structure (caps, i);
    gst_structure_set_valist (structure, fieldname, var_args);
    va_end (var_args);
  }

  return caps;
}

static void
gst_coda_enc_base_init (GstCodaEncClass * g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);
  GstPadTemplate *srctempl;//*sinktempl, 
  GstCaps *srccaps;//*sinkcaps, 

  gst_element_class_set_details_simple (element_class, "CODA video encoder",
      "Codec/Encoder/Video",
      "Encode Video based on coda core", 
      "wangqin <qin.wang@huayamicro.com>");

  /* get the caps */
  srccaps = gst_codaenc_vid_caps_new ("video/mpeg",
            "mpegversion", GST_TYPE_INT_RANGE, 1, 4, NULL);

  gst_caps_append (srccaps, gst_codaenc_vid_caps_new ("video/x-h264", NULL));
  gst_caps_append (srccaps, gst_codaenc_vid_caps_new ("video/x-h263", NULL));
  gst_caps_append (srccaps, gst_codaenc_vid_caps_new ("video/x-jpeg", NULL));
  
  
  if (!srccaps) {
    GST_INFO ("Couldn't get src caps for encoder\n" );
    srccaps = gst_caps_from_string ("unknown/unknown");
  }
  #if 0
  if (in_plugin->type == CODEC_TYPE_VIDEO) {
    srccaps = gst_caps_from_string ("video/x-raw-rgb; video/x-raw-yuv");
  } else {
    srccaps = gst_ffmpeg_codectype_to_audio_caps (NULL,
        in_plugin->id, FALSE, in_plugin);
  }
  if (!srccaps) {
    GST_DEBUG ("Couldn't get source caps for decoder '%s'", in_plugin->name);
    srccaps = gst_caps_from_string ("unknown/unknown");
  }
  srctempl = gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS, srccaps);  
  #endif

  /* pad templates */
  srctempl = gst_pad_template_new ("src", GST_PAD_SRC,
      GST_PAD_ALWAYS, srccaps);

  gst_element_class_add_pad_template (element_class, srctempl);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_template));

  g_class->srctempl = srctempl;  
}

static void
gst_coda_enc_finalize (GObject * object)
{
  GstCodaEnc *enc = GST_CODA_ENC (object);

  //g_object_unref (enc->adapter);

  g_static_rec_mutex_free (enc->enc_lock);
  g_free (enc->enc_lock);

  g_mutex_free (enc->buflock);
  enc->buflock = NULL;

  hal_device_detach(enc->fd);

  printf("finalize\n");

  G_OBJECT_CLASS (parent_class)->finalize (object);
}



static void
gst_coda_enc_class_init (GstCodaEncClass * klass)
{
  GstElementClass *gstelement_class;
  GObjectClass *gobject_class;

  gstelement_class = (GstElementClass *) klass;
  gobject_class = (GObjectClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->set_property = gst_coda_enc_set_property;
  gobject_class->get_property = gst_coda_enc_get_property;
  gobject_class->finalize = gst_coda_enc_finalize;

  gstelement_class->change_state = GST_DEBUG_FUNCPTR (gst_coda_enc_change_state);

  GST_DEBUG_CATEGORY_INIT (codaenc_debug, "codaenc", 0, "CODA encoder");

}



static void
gst_coda_enc_init (GstCodaEnc * enc)
{
  GstCodaEncClass *encclass;

  encclass = (GstCodaEncClass *) (G_OBJECT_GET_CLASS (enc));

  GST_DEBUG ("encoder initializing\n");

  /* create the sink and src pads */
  enc->sinkpad = gst_pad_new_from_template (&sink_template,"sink");
  gst_element_add_pad (GST_ELEMENT (enc), enc->sinkpad);
  gst_pad_set_setcaps_function (enc->sinkpad, GST_DEBUG_FUNCPTR (gst_coda_enc_setcaps));
  //gst_pad_set_getcaps_function (enc->sinkpad, GST_DEBUG_FUNCPTR (gst_coda_enc_getcaps));
  gst_pad_set_chain_function (enc->sinkpad, GST_DEBUG_FUNCPTR (gst_coda_enc_chain));
  //gst_pad_set_event_function (enc->sinkpad, GST_DEBUG_FUNCPTR (gst_coda_enc_sink_event));

  enc->srcpad = gst_pad_new_from_static_template (encclass->srctempl, "src");
  //gst_pad_set_event_function (enc->srcpad, GST_DEBUG_FUNCPTR (gst_coda_enc_src_event));
  gst_pad_use_fixed_caps (enc->srcpad);
  //gst_pad_set_query_function (enc->srcpad, GST_DEBUG_FUNCPTR (gst_coda_enc_src_query));  
  gst_element_add_pad (GST_ELEMENT (enc), enc->srcpad);

  
  enc->buflock = g_mutex_new ();
  
  enc->fd = hal_device_attach("venc_coda");

  hal_int_register(enc->fd, gst_coda_enc_loop, (void *)enc);
  
  enc->task = NULL;
  enc->enc_lock= g_new (GStaticRecMutex, 1);
  g_static_rec_mutex_init (enc->enc_lock);

  sem_init(&enc->push_sem, 0, 0); 

 
#ifdef DATA_DEBUG
  fp = g_fopen("data.bin", "wb");
#endif

  GST_DEBUG("encode init finish\n");
  
    //dec->adapter = gst_adapter_new ();

}


static void gst_codaenc_caps_to_codecid (const GstCaps * caps, const gchar *mimetype, struct venc_private *ppriv)
{
  EncInstCtl *handle=ppriv->inst_handle;  
  EncConfigParam *EncConfParam=&ppriv->ConfParam;
  GstStructure *structure = NULL;

  structure = gst_caps_get_structure (caps, 0);

  if (!strcmp (mimetype, "video/x-h263")) {
    const gchar *h263version =
        gst_structure_get_string (structure, "h263version");
    if (h263version && !strcmp (h263version, "h263p"))
      printf("version h263p\n");
	EncConfParam->stdMode = handle->BitstreamFormat = STD_H263;
  } else if (!strcmp (mimetype, "video/mpeg")) {
    gboolean sys_strm;
    gint mpegversion;

    if (gst_structure_get_boolean (structure, "systemstream", &sys_strm) &&
        gst_structure_get_int (structure, "mpegversion", &mpegversion) &&
        !sys_strm) {
      switch (mpegversion) {
        case 1:
        case 2:
          break;
        case 4:
          EncConfParam->stdMode = handle->BitstreamFormat = STD_MP4;
          break;
      }
    }
  } else if (!strcmp (mimetype, "image/jpeg")) {
	  EncConfParam->stdMode = handle->BitstreamFormat = STD_MJPG;
  } else if (!strcmp (mimetype, "video/x-h264")) {
	  EncConfParam->stdMode = handle->BitstreamFormat = STD_AVC;
  }  

}

static void
gst_codaenc_caps_to_pixfmt (const GstCaps * caps, struct venc_private *ppriv)
{
  GstStructure *structure;
  const gchar *mimetype;
  EncConfigParam *EncConfParam=&ppriv->ConfParam;
  const GValue *fps;
  const GValue *par = NULL;

  GST_DEBUG ("converting caps %" GST_PTR_FORMAT, caps);
  g_return_if_fail (gst_caps_get_size (caps) == 1);
  structure = gst_caps_get_structure (caps, 0);

  mimetype = gst_structure_get_name (structure);
  gst_codaenc_caps_to_codecid(caps, mimetype, ppriv);
  	
  gst_structure_get_int (structure, "width", &EncConfParam->picWidth);
  gst_structure_get_int (structure, "height", &EncConfParam->picHeight);
  gst_structure_get_int (structure, "bpp", &EncConfParam->BitsPerPixel);

  fps = gst_structure_get_value (structure, "framerate");
  if (fps != NULL && GST_VALUE_HOLDS_FRACTION (fps)) {

    /* somehow these seem mixed up.. */
    EncConfParam->framerate_numerator = gst_value_get_fraction_numerator (fps);
    EncConfParam->framerate_denominator = gst_value_get_fraction_denominator (fps);

    GST_DEBUG ("setting framerate %d/%d \n",
        EncConfParam->framerate_numerator, EncConfParam->framerate_numerator);
  }

  par = gst_structure_get_value (structure, "pixel-aspect-ratio");
  if (par && GST_VALUE_HOLDS_FRACTION (par)) {

    EncConfParam->aspectratio_numerator = gst_value_get_fraction_numerator (par);
    EncConfParam->aspectratio_denominator = gst_value_get_fraction_denominator (par);

    GST_DEBUG ("setting pixel-aspect-ratio %d/%d n",
        EncConfParam->aspectratio_denominator, EncConfParam->aspectratio_numerator);
  }

  if (strcmp (gst_structure_get_name (structure), "video/x-raw-yuv") == 0) {
    guint32 fourcc;

    if (gst_structure_get_fourcc (structure, "format", &fourcc)) {
      switch (fourcc) {
        case GST_MAKE_FOURCC ('I', '4', '2', '0'):
          EncConfParam->mjpgChromaFormat = YCBCR420;
          break;
        case GST_MAKE_FOURCC ('Y', '4', '2', 'B'):
          EncConfParam->mjpgChromaFormat = YCBCR422H;
          break;
        case GST_MAKE_FOURCC ('Y', '4', '4', '4'):
          EncConfParam->mjpgChromaFormat = YCBCR444;
          break;		  
        case GST_MAKE_FOURCC ('Y', '8', '0', '0'):
          EncConfParam->mjpgChromaFormat = YCBCR400;
          break;
		default:
		  EncConfParam->mjpgChromaFormat = YCBCR420;
		  break;

      }
    }
  } 
}


static void gst_coda_enc_setcaps (GstPad * pad, GstCaps * caps)
{
  GstCodaEnc *enc = GST_CODA_ENC (GST_OBJECT_PARENT (pad));
  struct venc_private *priv = (struct venc_private*)enc->priv;
  EncConfigParam *EncConfParam=&priv->ConfParam;
//  GstVideoFormat format;
//  gint width, height;
//  gint fps_num, fps_den;
//  gint par_num, par_den;
//  gint i;
//  GstCaps *othercaps;
//  gboolean ret;

  gst_codaenc_caps_to_pixfmt (caps, priv);

  if (!EncConfParam->framerate_denominator) {
    EncConfParam->framerate_denominator = 25;
    EncConfParam->framerate_numerator= 1;
  } else if ((EncConfParam->stdMode == STD_MP4)
      && (EncConfParam->framerate_denominator > 65535)) {
    /* MPEG4 Standards do not support time_base denominator greater than
     * (1<<16) - 1 . We therefore scale them down.
     * Agreed, it will not be the exact framerate... but the difference
     * shouldn't be that noticeable */
    EncConfParam->framerate_numerator=
        (gint) gst_util_uint64_scale_int (EncConfParam->framerate_numerator,
        65535, EncConfParam->framerate_denominator);
    EncConfParam->framerate_denominator = 65535;
  }

    /* setup coda encoder lib */
  VideoEncoder_Init(enc);


}


static void
gst_coda_enc_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstCodaEnc *enc;
  struct venc_private *priv = (struct venc_private*)enc->priv;
  EncConfigParam *EncConfParam=&priv->ConfParam;

  /* Get a pointer of the right type. */
  enc = (GstCodaEnc *) (object);

  /* Check the argument id to see which argument we're setting. */
  switch (prop_id) {
    case ARG_BIT_RATE:
      EncConfParam->bitrate = g_value_get_ulong (value);
      break;
    case ARG_GOP_SIZE:
      EncConfParam->gop_size = g_value_get_int (value);
      break;
	case ARG_BUFSIZE:
	  EncConfParam->buf_size = g_value_get_int (value);
	  break;
  }
}

/* The set function is simply the inverse of the get fuction. */
static void
gst_coda_enc_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstCodaEnc *enc;
  struct venc_private *priv = (struct venc_private*)enc->priv;
  EncConfigParam *EncConfParam=&priv->ConfParam;  

  /* It's not null if we got it, but it might not be ours */
  enc = (GstCodaEnc *) (object);

  switch (prop_id) {
    case ARG_BIT_RATE:
      g_value_set_ulong (value, EncConfParam->bitrate);
      break;
    case ARG_GOP_SIZE:
      g_value_set_int (value, EncConfParam->gop_size);
      break;
	case ARG_BUFSIZE:
	  g_value_set_int (value, EncConfParam->buf_size);
	  break;
	  
  }
}


static GstStateChangeReturn
gst_coda_enc_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret;
  GstCodaEnc *enc;

  enc = GST_CODA_ENC (element);
  
  switch (transition) {
  	case GST_STATE_CHANGE_NULL_TO_READY:
	  break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
	  break;
	case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
	  break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  if (ret != GST_STATE_CHANGE_SUCCESS)
    return ret;

  switch (transition) {
  	case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
	  break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      break;
	case GST_STATE_CHANGE_READY_TO_NULL:
      break;
    default:
      break;
  }

  return ret;
}


