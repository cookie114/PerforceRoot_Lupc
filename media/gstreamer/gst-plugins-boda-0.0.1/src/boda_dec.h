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

#ifndef BODA_DEC_H
#define BODA_DEC_H

#include "boda_vpu.h"

#define INPUT_BUFFER_PADDING_SIZE   8

#define VDEC_REG_BASE_ADDR      0x1F400000
#define VDEC_MEM_BASE_ADDR      0x03200000
#define VDEC_STREAM_BUF_SIZE    0x400000
#define VDEC_FRAME_BUF_SIZE     0x3000000
#define VDEC_MV_BUF_ADDR        0x13600000
#define VDEC_FRAME_BUF_ADDR		0X10000000

#define VBV_RESERVE_DATA		0x200

#define MAX_NUM_FRAME					32
#define LAKERS_FRAME_FLAG_NUM 			2
#define TOP_FIELD							0
#define BOTTOM_FIELD						1

/*vdec thread event*/
#define VDEC_FIRST_PIC	        	0x01


enum boda_dec_status {
	IDLE_STATE,
	SEQ_START_STATE,
	SEQ_CONTINUE_STATE,
	SET_FRAMEBUF_STATE,
	SET_FRAMEBUF_DONE_STATE,
	REGISTER_FRAMEBUF_STATE,
	SET_FRAMEBUF_CONTINUE_STATE,
	PICFRM_START_STATE,
	PICBIT_START_STATE,
	PICBIT_CONTINUE_STATE,
	//WAITFOR_FLUSH_BSBUF_STATE, 
	//FLUSH_BSBUF_STATE,
	WAITFOR_STOP_STATE,
	VPU_STOP_STATE
};

struct H264BSContext {
    guchar  length_size;
    guchar  first_idr;
    guchar *sps_pps_data;
    guint size;
};

struct MP4BSContext{
	guchar first_idr;
};

typedef struct {
	guint dwMicroSecPerFrame;
	guint dwMaxBytesPerSec;
	guint dwReserved1;
	guint dwFlags;
	guint dwTotalFrames;
	guint dwInitialFrames;
	guint dwStreams;
	guint dwSuggestedBufferSize;
	guint dwWidth;
	guint dwHeight;
	guint dwScale;
	guint dwRate;
	guint dwStart;
	guint dwLength;
} MainAVIHeader;

typedef struct {
	guint dwFlags;
	guint dwReserved1;
	guint dwInitialFrames;
	guint dwScale;
	guint dwRate;
	guint dwStart;
	guint dwLength;
	guint dwSuggestedBufferSize;
	guint dwQuality;
	guint dwSampleSize;
 	struct {
         gushort left;
         gushort top;
         gushort right;
         gushort bottom;
 	};
} AVIStreamHeader;

typedef struct {
	guint biSize;
	guint biWidth;
	guint biHeight;
	gushort biPlanes;
	gushort biBitCount;
	guint biCompression;
	guint biSizeImage;
	guint biXPersPerMeter;
	guint biYPersPerMeter;
	guint biClrUsed;
	guint biClrImportant;
} BitmapInfoHeader;

struct Divx3BSContext{
	guchar first_idr;
	guint file_len;
	guint list_len;
	guint avih_len;
	guint strl_len;
	guint strf_len;
	guint isft_len;
	guint junk_len;
	MainAVIHeader main_header;
	AVIStreamHeader avi_header;
	BitmapInfoHeader bitmap_header;
};


struct VC1BSContext{
	guint biCompression;
	guint SeqData[9];
	guchar first_frame;
	guint pts;
	guint frame_size;
};

struct frame_info {
	
	FrameBuffer	frm_buffer;			
	guint   	size[4];			/*[0] y, [1] cb, [2] cr, [3] MvCol*/
	Rect        frm_rect;
	
	GstClockTime   	pts;
	guint   	pts_valid	: 1;
	guint   	stc_id		: 2;  /* 0,1 to sel id, 2 means not check stc */
	guint   	used		: 1;	
	guint   	display		: 1;	

    guint       picture_coding_type : 2;

	gint        hsize;
	gint        vsize;

	GstBuffer	*outbuf;


	//struct video_info 			frm_video_info;
	
	//struct lbuf				tag;
	//struct video_picture_meta	meta;    
    
};

struct sync_block {
	GstClockTime   pts;					/* PTS value */
	guint   start_addr;			/* start of data field  */
	guint   end_addr;			/* end of data field */
	guint   num;
	struct sync_block *next;
	guchar used;
};
#define BODA_META_NUM	5000

struct boda_sync_block{
	struct sync_block block[BODA_META_NUM];
	struct boda_sync_block *next;
};

struct boda_sync_ctrl {
	struct boda_sync_block *sync_block_list;
	gushort rd;
	gushort wr;
	gushort cnt;
	struct sync_block* cur_block;
	guint   start_addr;
	guint   end_addr;
	struct sync_block* head_block;
	GstClockTime   frame_step;
	GstClockTime   last_pic_pts;
	guchar 	sync_state		: 1; /*0: free, 1:sync*/
	guchar 	sync_data		: 1; /*0: no, 1:yes*/
};


struct frame_flag{
	guint   frame_flag_buf;
	guint   size;
	guchar  used;
	guchar  index;
};

struct outlbuf{
	struct outlbuf     *next;
	guchar             index;
};

typedef struct boda_frm_ctrl {
	struct frame_info	frm_info[MAX_NUM_FRAME];
	//struct vdec_lakers_frm_flag flag_info[LAKERS_FRAME_FLAG_NUM];
	//struct vdec_lakers_frm	frm_info[8];
	struct frame_flag flag_info[2];
	//gshort display_index;
	//gshort current_decode_index;
	//guchar next_decode_index[3];
	//guchar header_decoding_success;
	guint   base_addr;
	guint   size;
	guint   mv_base_addr;
	guchar  max_dec_frame_num;
	guchar 	cur_idx;
	guchar  clr_flag;	
	guchar  disp_idx;
	guchar	pre_idx;		// the last diplay idx
	guint   disp_flag;
	guchar	IPframe;
	guchar  output_en;
	GstClockTime get_pts;
	guchar  get_pts_valid;
	guchar  last_dec_frame;
	struct outlbuf      *outbuf;
	guchar buffromv4l;
	guint  start_addr;
}Frm_Ctrl;

struct inlbuf {
	struct inlbuf		*next;						/* Next INLBUF point */
    GstBuffer			*inbuf;
} ;


typedef struct boda_vbv_ctrl
{
	//struct lbuf	*head;
	guint	start;
	guint	end;
	guint	readp;
	guint	writep;
	guint	config_writep;					// used for overflow
	guint	valid_size;
	guint	config_end;
	guchar 	vbv_share	: 1;
	guint 	first_data  : 1;
	volatile guchar	vbv_vo_status;
	guint	config_validsize;					// used for overflow
	struct inlbuf *databuf;
	gint   vo_status;
	gint   vo_test_status;
	guint  threshold;
}Vbv_Ctrl;



struct vdec_start_param {
	guchar  dec_std;
};

struct vdec_private{
	guchar					instIdx;
	DecInstCtl *			inst_handle;

	enum boda_dec_status	dec_status;
	enum boda_dec_status    stop_status;
	struct boda_frm_ctrl	frm_ctrl;
	struct boda_vbv_ctrl	vbv_ctrl;
	struct boda_sync_ctrl	sync_ctrl;
	struct frame_info		output_video_info;
	guchar					stream_update;

	guint					disp_cnt;
	guchar                  decoded_first_frame;
	
	guint					first_pic_param;

	guchar					wait_finish;
	guint					start_time;
	guint                   end_time;

	guchar					is_stillpic;
	guint 					err_cnt;
	guint                   vo_times;

	guchar					VPU_code_download;
	BitPrcBuffer 			VPU_buffer;
	
};

#endif

