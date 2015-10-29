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

#include "gstboda.h"
#include "boda_dec.h"

#include <glib.h>
#include <glib/gstdio.h>

GST_DEBUG_CATEGORY_STATIC (bodadec_debug);
#define GST_CAT_DEFAULT bodadec_debug

/* *INDENT-OFF* */
static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw-yuv, "     
    "format = (fourcc) { YVYU, I420, Y444, Y800 }, "   
    //"format = (fourcc) I420, "   
    "width = (int) [ 16, 1920 ], "      
    "height = (int) [ 16, 1088 ], "    
    "framerate = (fraction) [ 0/1, 2147483647/1 ]")  
    );

static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
   // GST_STATIC_CAPS ("{video/mpeg, video/x-h264,video/x-wmv,video/x-h263,video/x-3ivx,video/x-xvid,video/x-divx,video/x-msmpeg,video/x-pn-realvideo,video/x-jpeg},"
    GST_STATIC_CAPS ("video/mpeg,"
        "width = (int) [ 0, MAX ], " 
        "height = (int) [ 0, MAX ], "
        "framerate = (fraction) [ 0/1, MAX ]")
    );

static guint64 start = 0, end = 0;
static GstClockTime tfstart, tfend;
static GstClockTime loopstart, loopend, loopstart1;
static GstClockTime rlsstart, rlsend;
static GstClockTime pushstart, pushend;
static GstClockTime allocstart, allocend;
static GstClockTimeDiff diff;
static guint32 frm_num=1;
static gchar prog_cnt;
static GstElementClass *parent_class;   /* NULL */

static void gst_boda_dec_base_init (GstBodaDecClass * g_class);
static void gst_boda_dec_class_init (GstBodaDecClass * klass);
static void gst_boda_dec_init (GstBodaDec * bodadec);
#if 0
static void gst_boda_dec_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_boda_dec_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
#endif
static GstFlowReturn gst_boda_dec_chain (GstPad * pad, GstBuffer * buffer);
static gboolean gst_boda_dec_setcaps (GstPad * pad, GstCaps * caps);
static gboolean gst_boda_dec_sink_event (GstPad * pad, GstEvent * event);
static gboolean gst_boda_dec_src_event (GstPad * pad, GstEvent * event);
static GstStateChangeReturn gst_boda_dec_change_state (GstElement * element,
    GstStateChange transition);
static gboolean gst_boda_dec_src_query (GstPad * pad, GstQuery * query);
static void gst_boda_dec_negotiate (GstBodaDec * dec, gint width, gint height);
static void gst_boda_dec_loop(void *param);
static void gst_boda_dec_frame(GstBodaDec *dec);
static void copydata2buffer(GstBodaDec * dec, guchar *data, guint size, GstClockTime pts);
static gboolean gst_boda_dec_finish(GstBodaDec *dec);

//#define DATA_DEBUG

#ifdef DATA_DEBUG
FILE *fp, *fq;
#endif

GType
gst_boda_dec_get_type (void)
{
  static GType type = 0;

  if (!type) {
    static const GTypeInfo boda_dec_info = {
      sizeof (GstBodaDecClass),
      (GBaseInitFunc) gst_boda_dec_base_init,
      NULL,
      (GClassInitFunc) gst_boda_dec_class_init,
      NULL,
      NULL,
      sizeof (GstBodaDec),
      0,
      (GInstanceInitFunc) gst_boda_dec_init,
    };

    type = g_type_register_static (GST_TYPE_ELEMENT, "GstBodaDec",
        &boda_dec_info, 0);
  }
  return type;
}

static void
gst_boda_dec_finalize (GObject * object)
{
  GstBodaDec *dec = GST_BODA_DEC (object);

  //g_object_unref (dec->adapter);

  g_static_rec_mutex_free (dec->dec_lock);
  g_free (dec->dec_lock);

  g_mutex_free (dec->buflock);
  dec->buflock = NULL;

  printf("finalize\n");

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GstCaps *
gst_boda_vid_caps_new (const char *mimetype, const int w_min, const int w_max, const int h_min, const int h_max, const char *fieldname, ...)
{
  GstStructure *structure = NULL;
  GstCaps *caps = NULL;
  va_list var_args;
  gint i;

  if (!caps) {
    GST_DEBUG ("Creating default caps");
    caps = gst_caps_new_simple (mimetype,
        "width", GST_TYPE_INT_RANGE, w_min, w_max,
        "height", GST_TYPE_INT_RANGE, h_min, h_max,
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


void
gst_boda_caps_with_codecid (GstBodaDec *dec, enum CodecStd codec_id,const GstCaps * caps)
{
  GstStructure *str;
  const GValue *value;
  const GstBuffer *buf;

  if (!gst_caps_get_size (caps))
    return;

  str = gst_caps_get_structure (caps, 0);

  /* extradata parsing (esds [mpeg4], wma/wmv, msmpeg4v1/2/3, etc.) */
  if ((value = gst_structure_get_value (str, "codec_data"))) {
    guint size;
    guint8 *data;

    buf = GST_BUFFER_CAST (gst_value_get_mini_object (value));
    size = GST_BUFFER_SIZE (buf);
    data = GST_BUFFER_DATA (buf);

    /* free the old one if it is there */
    if (dec->codec_data)
      g_free (dec->codec_data);

#if 0
    if (codec_id == CODEC_ID_H264) {
      guint extrasize;

      GST_DEBUG ("copy, escaping codec_data %d", size);
      /* ffmpeg h264 expects the codec_data to be escaped, there is no real
       * reason for this but let's just escape it for now. Start by allocating
       * enough space, x2 is more than enough.
       *
       * FIXME, we disabled escaping because some file already contain escaped
       * codec_data and then we escape twice and fail. It's better to leave it
       * as is, as that is what most players do. */
      context->extradata =
          av_mallocz (GST_ROUND_UP_16 (size * 2 +
              FF_INPUT_BUFFER_PADDING_SIZE));
      copy_config (context->extradata, data, size, &extrasize);
      GST_DEBUG ("escaped size: %d", extrasize);
      context->extradata_size = extrasize;
    } else
#endif
    {
      /* allocate with enough padding */
      GST_DEBUG ("copy codec_data");
      dec->codec_data =
          g_malloc (GST_ROUND_UP_16 (size + INPUT_BUFFER_PADDING_SIZE));
	  memset(dec->codec_data, 0, size);
      memcpy (dec->codec_data, data, size);
      dec->codec_data_size = size;
    }

    /* Hack for VC1. Sometimes the first (length) byte is 0 for some files */
    if (codec_id == STD_VC1 && size > 0 && data[0] == 0) {
      dec->codec_data[0] = (guint8) size;
    }

    GST_DEBUG ("have codec data of size %d", size);
  } else if (dec->codec_data == NULL) {
    /* no extradata, alloc dummy with 0 sized, some codecs insist on reading
     * extradata anyway which makes then segfault. */
    dec->codec_data =
        g_malloc (GST_ROUND_UP_16 (INPUT_BUFFER_PADDING_SIZE));
    dec->codec_data_size = 0;
    GST_DEBUG ("no codec data");
  }

  if (!gst_caps_is_fixed (caps))
    return;

}

enum CodecStd
gst_boda_caps_to_codecid (GstBodaDec *dec, const GstCaps * caps)
{
  struct vdec_private *ppriv ;
  DecInstCtl *handle;  
  const gchar *mimetype;
  const GstStructure *structure;
  gboolean video = FALSE, audio = FALSE;        /* we want to be sure! */

  structure = gst_caps_get_structure (caps, 0);

  ppriv =(struct vdec_private *)dec->priv;
  handle=ppriv->inst_handle;  
  
  mimetype = gst_structure_get_name (structure);
  printf("video format %s\n", mimetype);

  if (!strcmp (mimetype, "video/x-h263")) {
    const gchar *h263version =
        gst_structure_get_string (structure, "h263version");
    if (h263version && !strcmp (h263version, "h263p"))
      printf("version h263p\n");
    handle->BitstreamFormat = STD_H263;
  } else if (!strcmp (mimetype, "video/mpeg")) {
    gboolean sys_strm;
    gint mpegversion;

    if (gst_structure_get_boolean (structure, "systemstream", &sys_strm) &&
        gst_structure_get_int (structure, "mpegversion", &mpegversion) &&
        !sys_strm) {
      switch (mpegversion) {
        case 1:
        case 2:
          handle->BitstreamFormat = STD_MP2;
          break;
        case 4:
          handle->BitstreamFormat = STD_MP4;
		  dec->priv_data = (struct MP4BSContext*)g_malloc(sizeof(struct MP4BSContext));
		  memset(dec->priv_data, 0, sizeof(struct MP4BSContext));
          break;
      }
    }
  } else if (!strcmp (mimetype, "image/jpeg")) {
		handle->BitstreamFormat = STD_MJPG;
  } else if (!strcmp (mimetype, "video/x-wmv")) {
    gint wmvversion = 0;

    if (gst_structure_get_int (structure, "wmvversion", &wmvversion)) {
	  struct VC1BSContext* VC1Context;	
	  dec->priv_data = (struct VC1BSContext*)g_malloc(sizeof(struct VC1BSContext));
	  memset(dec->priv_data, 0, sizeof(struct VC1BSContext));
	  VC1Context = (struct VC1BSContext*)dec->priv_data;
      switch (wmvversion) {
        case 1:
		  printf("WMV1\n");
		  VC1Context->biCompression = GST_MAKE_FOURCC('W', 'M', 'V', '1');
          break;
        case 2:
		  VC1Context->biCompression = GST_MAKE_FOURCC('W', 'M', 'V', '2');			
		  printf("WMV2\n");
          break;
        case 3:
        {
          guint32 fourcc;

          /* WMV3 unless the fourcc exists and says otherwise */
          if (gst_structure_get_fourcc (structure, "format", &fourcc)) {
            if (fourcc == GST_MAKE_FOURCC ('W', 'V', 'C', '1')) {
			  VC1Context->biCompression = GST_MAKE_FOURCC('W', 'V', 'C', '1');            
			  printf("VC1\n");
            }else if (fourcc == GST_MAKE_FOURCC ('W', 'M', 'V', 'A')){
              VC1Context->biCompression = GST_MAKE_FOURCC('W', 'M', 'V', 'A'); 
	  		  printf("VC1\n");
            }else{
			  VC1Context->biCompression = GST_MAKE_FOURCC('W', 'M', 'V', '3');        
              printf("WMV3\n");
            }
          }else{
			  VC1Context->biCompression = GST_MAKE_FOURCC('W', 'M', 'V', '3');        
              printf("WMV3\n");
          }
        }
		break;
		default:
			printf("other wmv format %d\n", wmvversion);
          break;
      }
    }else{
    	printf("no wmvversion\n");
    }
	handle->BitstreamFormat = STD_VC1;

  } else if (!strcmp (mimetype, "video/x-msmpeg")) {
    gint msmpegversion = 0;

    if (gst_structure_get_int (structure, "msmpegversion", &msmpegversion)) {
      switch (msmpegversion) {
        case 41:
        case 42:
		  handle->BitstreamFormat = STD_MP4;
		  printf("msmpegversion %d\n", msmpegversion);
          break;
        case 43:
		  handle->BitstreamFormat = STD_DIV3;
  		  dec->priv_data = (struct Divx3BSContext*)g_malloc(sizeof(struct Divx3BSContext));
		  memset(dec->priv_data, 0, sizeof(struct Divx3BSContext));
		  printf("msmpegversion %d\n", msmpegversion);
          break;
      }
    }
  } else if (!strcmp (mimetype, "video/x-divx")) {
    gint divxversion = 0;

    if (gst_structure_get_int (structure, "divxversion", &divxversion)) {
      switch (divxversion) {
        case 3:
		  handle->BitstreamFormat = STD_DIV3;
          break;
        case 4:
        case 5:
		  handle->BitstreamFormat = STD_MP4;
          break;
      }
    }
  } else if (!strcmp (mimetype, "video/x-3ivx")) {
	handle->BitstreamFormat = STD_MP4;
  } else if (!strcmp (mimetype, "video/x-xvid")) {
	handle->BitstreamFormat = STD_MP4;
  } else if (!strcmp (mimetype, "video/x-pn-realvideo")) {
    gint rmversion;

    if (gst_structure_get_int (structure, "rmversion", &rmversion)) {
      switch (rmversion) {
        case 1:
		  printf("RV10\n");
          break;
        case 2:
		  printf("RV20\n");
          break;
        case 3:
		  printf("RV30\n");
          break;
        case 4:
		  printf("RV40\n");
          break;
      }
    }
	handle->BitstreamFormat = STD_RV;
  } else if (!strcmp (mimetype, "video/x-h264")) {
	handle->BitstreamFormat = STD_AVC;
	dec->priv_data = (struct H264BSContext*)g_malloc(sizeof(struct H264BSContext));
	memset(dec->priv_data, 0, sizeof(struct H264BSContext));
  }  

  gst_boda_caps_with_codecid (dec, handle->BitstreamFormat, caps);

  GST_DEBUG ("The id=%d belongs to the caps %" GST_PTR_FORMAT, handle->BitstreamFormat, caps);

  return handle->BitstreamFormat;
}


static void
gst_boda_dec_base_init (GstBodaDecClass * g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);
  GstPadTemplate *sinktempl, *srctempl;
  GstCaps *sinkcaps, *srccaps;

  gst_element_class_set_details_simple (element_class, "BODA video decoder",
      "Codec/Decoder/Video",
      "Decode Video based on boda core", 
      "wangqin <qin.wang@huayamicro.com>");

  /* get the caps */
  sinkcaps = gst_boda_vid_caps_new ("video/mpeg", 16, 1920, 16, 1088,
            "mpegversion", GST_TYPE_INT_RANGE, 1, 4, "systemstream", G_TYPE_BOOLEAN, FALSE, NULL);

  gst_caps_append (sinkcaps, gst_boda_vid_caps_new ("video/x-h264", 16, 1920, 16, 1088, NULL));
  gst_caps_append (sinkcaps, gst_boda_vid_caps_new ("video/x-wmv", 16, 1920, 16, 1088, NULL));
  gst_caps_append (sinkcaps, gst_boda_vid_caps_new ("video/x-h263", 16, 1920, 16, 1088, NULL));
  gst_caps_append (sinkcaps, gst_boda_vid_caps_new ("video/x-msmpeg", 16, 1920, 16, 1088,
  	"msmpegversion", GST_TYPE_INT_RANGE, 41, 43, NULL));
  gst_caps_append (sinkcaps, gst_boda_vid_caps_new ("video/x-pn-realvideo", 16, 1920, 16, 1088, NULL));
  gst_caps_append (sinkcaps, gst_boda_vid_caps_new ("video/x-jpeg", 16, 8192, 16, 8192, NULL));
  gst_caps_append (sinkcaps, gst_boda_vid_caps_new ("image/jpeg", 16, 8192, 16, 8192, NULL));
  gst_caps_append (sinkcaps, gst_boda_vid_caps_new ("video/x-divx",16, 1920, 16, 1088,
  	"divxversion", GST_TYPE_INT_RANGE, 4, 6, NULL));
  gst_caps_append (sinkcaps, gst_boda_vid_caps_new ("video/x-xvid", 16, 1920, 16, 1088, NULL));
  gst_caps_append (sinkcaps, gst_boda_vid_caps_new ("video/x-3ivx", 16, 1920, 16, 1088, NULL));
  
  
  if (!sinkcaps) {
    GST_DEBUG ("Couldn't get sink caps for decoder \n" );
    sinkcaps = gst_caps_from_string ("unknown/unknown");
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
  sinktempl = gst_pad_template_new ("sink", GST_PAD_SINK,
      GST_PAD_ALWAYS, sinkcaps);

//  gst_element_class_add_pad_template (element_class, srctempl);
  gst_element_class_add_pad_template (element_class, sinktempl);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_template));

//  g_class->srctempl = srctempl;
  g_class->sinktempl = sinktempl;  
}

static void
gst_boda_dec_class_init (GstBodaDecClass * klass)
{
  GstElementClass *gstelement_class;
  GObjectClass *gobject_class;

  gstelement_class = (GstElementClass *) klass;
  gobject_class = (GObjectClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = gst_boda_dec_finalize;

  gstelement_class->change_state = GST_DEBUG_FUNCPTR (gst_boda_dec_change_state);

  GST_DEBUG_CATEGORY_INIT (bodadec_debug, "bodadec", 0, "BODA decoder");
  //GST_DEBUG_CATEGORY_GET (GST_CAT_PERFORMANCE, "GST_PERFORMANCE");
}


static guchar frm_request(Frm_Ctrl *pfrm_ctrl, DecInstCtl *handle)
{
	DecSeqGetInfo *GetInfo = &handle->SeqInfo;
	guchar i;
	
	for (i=0;i<GetInfo->numFrameBuffers;i++) {
		if (pfrm_ctrl->frm_info[i].used == 0) {
			pfrm_ctrl->frm_info[i].used = 1;
			return i;
		}
    }
	return 0xFF;

}

static void frm_set_state(Frm_Ctrl *pfrm_ctrl, guchar index, guchar used)
{
	if (index >= pfrm_ctrl->max_dec_frame_num) {
		return;
	}
	pfrm_ctrl->frm_info[index].used = used;

	return;
}

#if 0
static void frmbuf_release(struct vdec_lakers_private *ppriv, struct vdec_lakers_frm_ctrl *pfrm_ctrl, guchar tol_frm_num)
{
	guchar i, j;

#if 0
	for ( i = 0; i < frm_num; i++ ){
		printf("%d prog %d idx, display %d\n", prog_cnt, i, pfrm_ctrl->frm_info[i].display);
	}
#endif
	for ( i = 0; i < tol_frm_num; i++ ){
		if ( pfrm_ctrl->frm_info[i].display == 0 ){
			if ( pfrm_ctrl->frm_info[i].tag == NULL ){
				printf("frame %d lbuf NULL\n", i);
			}else{
				printf("%d prog rls %d framebuffer, addr %x\n", prog_cnt, i, (pfrm_ctrl->frm_info[i].tag)->buf_address);
				meml_free(pfrm_ctrl->frm_info[i].tag);
				pfrm_ctrl->frm_info[i].tag = NULL;
			}
		}else{
			pfrm_ctrl->frm_info[i].display = 0;
			pfrm_ctrl->frm_info[i].used = 0;
			pfrm_ctrl->frm_info[i].tag = NULL;
			pfrm_ctrl->frm_info[i].metalbuf = NULL;
			if ( pfrm_ctrl->free_cnt == MAX_NUM_FRAME){
				printf("free buffer full\n");
				g_assert(0);
			}else {
				if ( pfrm_ctrl->frm_info[i].tag ){
					for ( j = 0; j < MAX_NUM_FRAME; j++ ){
						if ( pfrm_ctrl->free_frame[j].tag == NULL){
							pfrm_ctrl->free_frame[j].tag = pfrm_ctrl->frm_info[i].tag;
							pfrm_ctrl->free_frame[j].metalbuf = pfrm_ctrl->frm_info[i].metalbuf;
							pfrm_ctrl->free_cnt++;
							break;
						}
					}
				}			
			}
		}
	}

}
#endif


static void frm_update(Frm_Ctrl *pfrm_ctrl, gshort index, DecInstCtl *dec_handle )
{
	struct frame_info* frm_info = &pfrm_ctrl->frm_info[index];
	//struct video_picture_meta* pmeta = &frm_info->meta;
	//DecFrameOutputInfo *FrameOutInfo = &dec_handle->FrameOutInfo;
	DecSeqGetInfo *SeqInfo = &dec_handle->SeqInfo;

	if (index >= pfrm_ctrl->max_dec_frame_num) {
		GST_WARNING("out of the max frame number\n");
		return;
	}

	frm_info->hsize = SeqInfo->picWidth;
	frm_info->vsize = SeqInfo->picHeight;

	return;

}

static void frm_output_queue(Frm_Ctrl *pfrm_ctrl, guchar frm_index)
{
    struct outlbuf * outdatabuf;
	printf("queue out buffer\n");
	if (pfrm_ctrl->outbuf == NULL){
		pfrm_ctrl->outbuf = (struct outlbuf*)g_malloc0(sizeof(struct outlbuf));
		if ( pfrm_ctrl->outbuf ){
			pfrm_ctrl->outbuf->index = frm_index;
			pfrm_ctrl->outbuf->next = NULL;
		}else{
			printf("error to alloc memory for outbuf queue\n");
		}
	}else{
		outdatabuf = pfrm_ctrl->outbuf;
		while(outdatabuf->next){
			outdatabuf = outdatabuf->next;
		}
		outdatabuf->next = (struct outlbuf*)g_malloc0(sizeof(struct outlbuf));
		if (outdatabuf->next){
			outdatabuf->next->index = frm_index;
			outdatabuf->next->next = NULL;
		}else{
			printf("error to alloc memory for outbuf queue\n");
		}
	}
	printf("queue finish\n");
}

static void frame_output(GstBodaDec *dec, gshort index)
{
	//struct device_link_item *output_link = dev_link_item_find(dev, ppriv->osid);
	//struct lbuf* ptag = &frm_info->tag;
	//struct video_picture_meta* pmeta = &frm_info->meta;
	//enum vdec_lakers_sync_flag flag;
	struct vdec_private *ppriv =(struct vdec_private *)dec->priv;
	DecInstCtl *dec_handle = ppriv->inst_handle;
	Frm_Ctrl *pfrm_ctrl = &ppriv->frm_ctrl;
	struct boda_sync_ctrl	*sync_ctrl=&ppriv->sync_ctrl;  
	struct frame_info* frm_info=&pfrm_ctrl->frm_info[index];	
	gshort disp_idx;
    GstBuffer *outbuf = NULL;
	GstClockTime duration;
	GstFlowReturn ret = GST_FLOW_OK;	
	guint addr, i, srcaddr, destaddr;
	guint test;


	disp_idx = dec_handle->FrameOutInfo.indexFrameDisplay;

	if (pfrm_ctrl->frm_info[index].display== 1){
		printf("redisplay the same index %d\n", index);
		return;
	}
	//g_usleep(1000000);
	//ppriv->output_video_info.hsize = frm_info->hsize;
	//ppriv->output_video_info.vsize = frm_info->vsize;
	//ppriv->output_video_info.frame_rate = frm_info->frm_video_info.frame_rate;
	//ppriv->output_video_info.aspect_ratio = frm_info->frm_video_info.aspect_ratio;
	
    #if 0
	if (frm_info->frm_video_info.frame_rate  )
		sync_ctrl->frame_step = 45000*1000/frm_info->frm_video_info.frame_rate;
	if (pfrm_ctrl->frm_info[index].pts_valid == 0) {
		pfrm_ctrl->frm_info[index].pts_valid = 1;
		pfrm_ctrl->frm_info[index].pts = sync_ctrl->last_pic_pts+sync_ctrl->frame_step;
		pfrm_ctrl->frm_info[index].stc_id = sync_ctrl->last_pic_stcid;
		//PRINTF("b.<%d>PTS<%x>\n",pfrm_ctrl->frm_info[index].picture_coding_type,pfrm_ctrl->frm_info[index].pts);
	} else {
		//PRINTF("a.<%d>PTS<%x>\n",pfrm_ctrl->frm_info[index].picture_coding_type,pfrm_ctrl->frm_info[index].pts);
	}
	sync_ctrl->last_pic_pts =  pfrm_ctrl->frm_info[index].pts;
	sync_ctrl->last_pic_stcid = pfrm_ctrl->frm_info[index].stc_id;
	#endif

	//frm_output_queue(pfrm_ctrl, index);

    //pfrm_ctrl->frm_info[index].display = 1;

#if 1
	outbuf = pfrm_ctrl->frm_info[index].outbuf;
	if ( pfrm_ctrl->frm_info[index].pts_valid == 1 ){
		GST_BUFFER_TIMESTAMP(outbuf) = pfrm_ctrl->frm_info[index].pts;
		pfrm_ctrl->frm_info[index].pts_valid = 0;
	}else{
		pfrm_ctrl->frm_info[index].pts = sync_ctrl->last_pic_pts + sync_ctrl->frame_step;
		GST_BUFFER_TIMESTAMP(outbuf) = pfrm_ctrl->frm_info[index].pts;
	}

	sync_ctrl->last_pic_pts = pfrm_ctrl->frm_info[index].pts;
	
	//gst_buffer_ref(outbuf);
    GST_BUFFER_SIZE(outbuf) = dec->outsize;
	GST_BUFFER_TIMESTAMP(outbuf) = pfrm_ctrl->frm_info[index].pts;
	//GST_BUFFER_TIMESTAMP(outbuf) = GST_CLOCK_TIME_NONE;
	
	duration = GST_CLOCK_TIME_NONE;
    GST_BUFFER_DURATION (outbuf) = duration;

	//GST_BUFFER_DATA(outbuf) = pfrm_ctrl->frm_info[index].frm_buffer.bufY;
	if (pfrm_ctrl->buffromv4l){
	  //fq = g_fopen("yuvdata.bin", "wb");
		for ( i = 0; i < frm_info->vsize; i++ ){
			srcaddr = pfrm_ctrl->frm_info[index].frm_buffer.bufY + i * GST_ROUND_UP_16(frm_info->hsize);
			destaddr = GST_BUFFER_DATA(outbuf) + i * GST_ROUND_UP_8(frm_info->hsize);
			memcpy((gpointer)destaddr, (gpointer)(hal_paddr_to_vaddr(srcaddr)), GST_ROUND_UP_8(frm_info->hsize));
			//fwrite((gpointer)destaddr, 1, GST_ROUND_UP_8(frm_info->hsize), fq);
		}

		for ( i = 0; i < (GST_ROUND_UP_2(frm_info->vsize)>>1); i++ ){
			srcaddr = pfrm_ctrl->frm_info[index].frm_buffer.bufCb + i * (GST_ROUND_UP_16(frm_info->hsize)/2);
			destaddr = GST_BUFFER_DATA(outbuf) + GST_ROUND_UP_8(frm_info->hsize)*GST_ROUND_UP_2(frm_info->vsize) + i * (GST_ROUND_UP_8(frm_info->hsize)/2);
			memcpy((gpointer)destaddr, (gpointer)(hal_paddr_to_vaddr(srcaddr)), (GST_ROUND_UP_8(frm_info->hsize)/2));
			//fwrite((gpointer)destaddr, 1, GST_ROUND_UP_8(frm_info->hsize)/2, fq);
		}

		for ( i = 0; i < (GST_ROUND_UP_2(frm_info->vsize)>>1); i++ ){
			srcaddr = pfrm_ctrl->frm_info[index].frm_buffer.bufCr + i * (GST_ROUND_UP_16(frm_info->hsize)/2);
			destaddr = GST_BUFFER_DATA(outbuf) + GST_ROUND_UP_8(frm_info->hsize)*GST_ROUND_UP_2(frm_info->vsize);
			destaddr += (GST_ROUND_UP_8(frm_info->hsize)/2) * (GST_ROUND_UP_2(frm_info->vsize)/2);
			destaddr += i * (GST_ROUND_UP_8(frm_info->hsize)/2);
			memcpy((gpointer)destaddr, (gpointer)(hal_paddr_to_vaddr(srcaddr)), (GST_ROUND_UP_8(frm_info->hsize)/2));	
			//fwrite((gpointer)destaddr, 1, GST_ROUND_UP_8(frm_info->hsize)/2, fq);

		}

		
		//fclose(fq);
	}


#else
	outbuf = gst_buffer_new();

	if ( pfrm_ctrl->frm_info[index].pts_valid == 1 ){
		GST_BUFFER_TIMESTAMP(outbuf) = pfrm_ctrl->frm_info[index].pts;
		pfrm_ctrl->frm_info[index].pts_valid = 0;
	}else{
		pfrm_ctrl->frm_info[index].pts = sync_ctrl->last_pic_pts + sync_ctrl->frame_step;
		GST_BUFFER_TIMESTAMP(outbuf) = pfrm_ctrl->frm_info[index].pts;
	}

	sync_ctrl->last_pic_pts = pfrm_ctrl->frm_info[index].pts;
	
    printf("allocate finish %x\n", (guint)outbuf);
	addr = pfrm_ctrl->frm_info[index].frm_buffer.bufY;

	printf("outbuf data %x, malloc data %x\n", GST_BUFFER_DATA(outbuf), GST_BUFFER_MALLOCDATA (outbuf));
	#if 0
    //GST_BUFFER_DATA(outbuf) = GST_BUFFER_MALLOCDATA (outbuf) = (guint8 *)g_malloc(dec->outsize) ;
	memcpy(outbuf->data, (guint8 *)(hal_paddr_to_vaddr((gpointer)addr)), dec->outsize);
    printf("buf addr %x, %x\n", (guint)GST_BUFFER_DATA(outbuf), *(guchar *)(outbuf->data+276480));
	#else
	//g_free(GST_BUFFER_DATA(outbuf));
	//GST_BUFFER_DATA(outbuf) = GST_BUFFER_MALLOCDATA (outbuf) = 0;
    GST_BUFFER_DATA(outbuf) = (guint8 *)(hal_paddr_to_vaddr((gpointer)addr)) ;
	#endif
	//gst_buffer_ref(outbuf);
	ppriv->framebuf[index] = outbuf;
	printf("allocate address %x\n", (guint)outbuf->data);
    GST_BUFFER_SIZE(outbuf) = dec->outsize;
	GST_BUFFER_TIMESTAMP(outbuf) = pfrm_ctrl->frm_info[index].pts;
	GST_BUFFER_FREE_FUNC (outbuf) = g_free;
	
	duration = GST_CLOCK_TIME_NONE;
    GST_BUFFER_DURATION (outbuf) = duration;

	gst_buffer_set_caps (outbuf, GST_PAD_CAPS (dec->srcpad));
#endif

#if 1
//	test = (guint)GST_MINI_OBJECT_REFCOUNT_VALUE(GST_MINI_OBJECT_CAST(outbuf));

	pushstart = gst_util_get_timestamp ();

//	printf("outbuf push start\n");

    ret = gst_pad_push (dec->srcpad, outbuf);

	GST_DEBUG("pad push ret %x\n", (gint)ret);
		{
			GstClockTime ppts;
			
			ppts = GST_BUFFER_TIMESTAMP(outbuf);
			GST_DEBUG("ppts %lld\n", ppts);
    	}
      pushend = gst_util_get_timestamp ();

      diff = GST_CLOCK_DIFF (pushstart, pushend);

	  GST_DEBUG ("64bit push time %lld ns.\n", diff);

	 // g_usleep(1000000);

#endif


}
static void alloc_and_copy(GstBodaDec *dec,const guchar *sps_pps, guint sps_pps_size,
							const guchar *in, guint in_size) 
{
    //gint offset = *poutbuf_size;
    guchar nal_header_size = 4;//offset ? 3 : 4;
    const guchar nalu_h[4] = {0, 0, 0, 1};

    if (sps_pps)
		copydata2buffer(dec, sps_pps, sps_pps_size, GST_CLOCK_TIME_NONE);
	copydata2buffer(dec, nalu_h, 4, GST_CLOCK_TIME_NONE);
	copydata2buffer(dec, in, in_size, GST_CLOCK_TIME_NONE);
}



static gint avc_mp4toannexb_filter(GstBodaDec *dec, guchar *buf, guint buf_size)
{
    struct H264BSContext *ctx = (struct H264BSContext *)dec->priv_data;
    guchar unit_type;
    gint nal_size;
    guint cumul_size = 0;
    const guchar *buf_end = buf + buf_size;

    /* nothing to filter */
    if (!dec->codec_data || dec->codec_data_size < 6) {
        //*data = (guchar*) buf;
        //*size = buf_size;		
        return 0;
    }

	if (dec->codec_data[0] != 1)      //configurationVersion==1
		return 0;

    /* retrieve sps and pps NAL units from extradata */
    if (!ctx->sps_pps_data) {
        gushort unit_size;
        guint total_size = 0;
        guchar *out = NULL, unit_nb, sps_done = 0;
        const guchar *extradata = dec->codec_data+4;
        static const guchar nalu_header[4] = {0, 0, 0, 1};
		guint i;

		for ( i = 0; i < dec->codec_data_size; i++ ){
			printf("%x ", dec->codec_data[i]);
			if (i % 16 == 15)
				printf("\n");
		}
        /* retrieve length coded size */
        ctx->length_size = (*extradata++ & 0x3) + 1;
        if (ctx->length_size == 3)
            return -1;

        /* retrieve sps and pps unit(s) */
        unit_nb = *extradata++ & 0x1f; /* number of sps unit(s) */
        if (!unit_nb) {
            unit_nb = *extradata++; /* number of pps unit(s) */
            sps_done++;
        }
        while (unit_nb--) {
            unit_size = (extradata[0] << 8) | extradata[1];
            total_size += unit_size+4;
            if (extradata+2+unit_size > dec->codec_data+dec->codec_data_size) {
				printf("AVC extradata size %d less than requirement %d\n", dec->codec_data_size, unit_size);
				if (out)
					g_free(out);
                return -1;
            }
            out = realloc(out, total_size);
            if (!out)
                return -1;

            memcpy(out+total_size-unit_size-4, nalu_header, 4);
            memcpy(out+total_size-unit_size,   extradata+2, unit_size);
            extradata += 2+unit_size;

            if (!unit_nb && !sps_done++)
                unit_nb = *extradata++; /* number of pps unit(s) */
        }

        ctx->sps_pps_data = out;
        ctx->size = total_size;
        ctx->first_idr = 1;
    }


    do {
        if (buf + ctx->length_size > buf_end)
            return -1;;

        if (ctx->length_size == 1)
            nal_size = buf[0];
        else if (ctx->length_size == 2)
            nal_size = (buf[0] << 8) | buf[1];
        else
            nal_size = (buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];

        buf += ctx->length_size;
        unit_type = *buf & 0x1f;

        if (buf + nal_size > buf_end || nal_size < 0)
            return -1;

        /* prepend only to the first type 5 NAL unit of an IDR picture */
        if (ctx->first_idr && unit_type == 5) {
            alloc_and_copy(dec,ctx->sps_pps_data, ctx->size,buf, nal_size);
            ctx->first_idr = 0;
        }
        else {
            alloc_and_copy(dec,NULL, 0,buf, nal_size);
            if (!ctx->first_idr && unit_type == 1)
                ctx->first_idr = 1;
        }

        buf += nal_size;
        cumul_size += nal_size + ctx->length_size;
    } while (cumul_size < buf_size);


    return 1;

}

static void mp4filter(GstBodaDec *dec, guchar *buf, guint buf_size)
{
	copydata2buffer(dec, buf, buf_size, GST_CLOCK_TIME_NONE);
	printf("size %x, data %x, %x, %x, %x\n", buf_size, buf[0], buf[1], buf[2], buf[3]);
}

static void wmvfilter(GstBodaDec *dec, guchar *buf, guint buf_size)
{
    struct VC1BSContext *ctx = (struct VC1BSContext *)dec->priv_data;

	if (ctx->biCompression == GST_MAKE_FOURCC('W', 'M', 'V', '3') ||
		ctx->biCompression == GST_MAKE_FOURCC('W', 'M', 'V', 'A')){
		if ( !ctx->first_frame ){
		    /*not rcv_v2*/
			ctx->first_frame = 1;
			ctx->SeqData[0]=(0x85<<24) | 0xFFFFFF;
			ctx->SeqData[1]=0x4;
			if (buf_size<4){
				printf("WMV3 struct C error\n");
			}
			ctx->SeqData[2]=buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24);
			ctx->SeqData[3]=dec->caps_height;
			ctx->SeqData[4]=dec->caps_width;
			copydata2buffer(dec, &(ctx->SeqData), 20, GST_CLOCK_TIME_NONE);
		}
		copydata2buffer(dec, &(ctx->frame_size), 4, GST_CLOCK_TIME_NONE);
	}else if (ctx->biCompression == GST_MAKE_FOURCC('W', 'V', 'C', '1')){
		guchar size;
		guchar frameheader[4]={0x00, 0x00, 0x01, 0x0D};
		if ( !ctx->first_frame ){
			ctx->first_frame = 1;
			size = buf[0];
			if ( size > buf_size )
				size = buf_size;
			copydata2buffer(dec, &buf[1], (size-1), GST_CLOCK_TIME_NONE);
		}
		copydata2buffer(dec, &frameheader, 4, GST_CLOCK_TIME_NONE);
	}

}

static void divx3filter(GstBodaDec *dec, struct Divx3BSContext *ctx, guint size)
{
	struct vdec_private *ppriv = (struct vdec_private *)dec->priv;
	struct boda_sync_ctrl *sync_ctrl=&ppriv->sync_ctrl;
	MainAVIHeader *mheader=&(ctx->main_header);
	AVIStreamHeader *sheader=&(ctx->avi_header);
	BitmapInfoHeader *bheader=&(ctx->bitmap_header);
	guchar *isft_data;
	guchar *junk_data;
	guint i;

	static const guchar vid_header[4] = {0x30, 0x30, 0x64, 0x63};
	static const guchar riff_header[4] = { 0x52, 0x49, 0x46, 0x46 };
	static const guchar avi_header[4] = { 0x41, 0x56, 0x49, 0x20 };
	static const guchar list_header[4] = { 0x4c, 0x49, 0x53, 0x54 };
	static const guchar hdrl_header[4] = { 0x68, 0x64, 0x72, 0x6c };
	static const guchar avih_header[4] = { 0x61, 0x76, 0x69, 0x68 };
	static const guchar strl_header[4] = { 0x73, 0x74, 0x72, 0x6c };
	static const guchar strh_header[4] = { 0x73, 0x74, 0x72, 0x68 };
	static const guchar vids_header[4] = { 0x76, 0x69, 0x64, 0x63 };
	static const guchar div3_header[4] = { 0x44, 0x49, 0x56, 0x33 };
	static const guchar strf_header[4] = { 0x73, 0x74, 0x72, 0x66 };
	static const guchar info_header[4] = { 0x49, 0x4e, 0x46, 0x4f };
	static const guchar isft_header[4] = { 0x49, 0x53, 0x46, 0x54 };
	static const guchar junk_header[4] = { 0x4a, 0x55, 0x4e, 0x4b };
	static const guchar movi_header[4] = { 0x6D, 0x6F, 0x76, 0x69 };
	static const guchar idx1_header[4] = { 0x69, 0x64, 0x78, 0x31 };
	guint len = 0;
	guint frame_time;
	

#define AVIF_HASINDEX			0x00000010
#define AVIF_MUSTUSEINDEX		0x00000020
#define AVIF_ISINTERLEAVED		0x00000100
#define AVIF_WASCAPTUREFILE		0x00010000
#define AVIF_COPYRIGHTED		0x00020000
#define AVIF_TRUSTCKTYPE		0x00000800

#define AVISF_DISABLED			0x00000001
#define AVISF_VIDEO_PALCHANGES  0x00010000

	if ( !ctx->first_idr ){
		/* create the AVI file */
		ctx->file_len = 0;
		copydata2buffer(dec, riff_header, 4, GST_CLOCK_TIME_NONE);
		copydata2buffer(dec, &(ctx->file_len), 4, GST_CLOCK_TIME_NONE);				//file len
		copydata2buffer(dec, avi_header, 4, GST_CLOCK_TIME_NONE);
		copydata2buffer(dec, list_header, 4, GST_CLOCK_TIME_NONE);
		ctx->list_len = 0xc0;
		copydata2buffer(dec, &(ctx->list_len) , 4, GST_CLOCK_TIME_NONE);			//AVI list len

		copydata2buffer(dec, hdrl_header, 4, GST_CLOCK_TIME_NONE);
		copydata2buffer(dec, avih_header, 4, GST_CLOCK_TIME_NONE);
		ctx->avih_len = 0x38;
		copydata2buffer(dec, &(ctx->avih_len), 4, GST_CLOCK_TIME_NONE);				//avih len
		mheader->dwMicroSecPerFrame = (dec->framerate_denominator*1e6)/ dec->framerate_numerator;
		mheader->dwMaxBytesPerSec = 0;
		mheader->dwReserved1 = 0;
		mheader->dwFlags = AVIF_HASINDEX | AVIF_ISINTERLEAVED | AVIF_TRUSTCKTYPE;
		mheader->dwTotalFrames = 0;
		mheader->dwInitialFrames = 0;
		mheader->dwStreams = 1;
		mheader->dwSuggestedBufferSize = 0;
		mheader->dwWidth = dec->caps_width;
		mheader->dwHeight = dec->caps_height;
		mheader->dwScale = 0;
		mheader->dwRate = 0;
		mheader->dwStart = 0;
		mheader->dwLength = 0;
		copydata2buffer(dec, mheader, sizeof(MainAVIHeader), GST_CLOCK_TIME_NONE);

		copydata2buffer(dec, list_header, 4, GST_CLOCK_TIME_NONE);
		ctx->list_len= 0x74;
		copydata2buffer(dec, &(ctx->list_len) , 4, GST_CLOCK_TIME_NONE);	
		
		copydata2buffer(dec, strl_header, 4, GST_CLOCK_TIME_NONE);
		copydata2buffer(dec, strh_header, 4, GST_CLOCK_TIME_NONE);
		ctx->strl_len = 0x38;
		copydata2buffer(dec, &(ctx->strl_len), 4, GST_CLOCK_TIME_NONE);
		copydata2buffer(dec, vids_header, 4, GST_CLOCK_TIME_NONE);
		copydata2buffer(dec, div3_header, 4, GST_CLOCK_TIME_NONE);
		memset(sheader, 0, sizeof(AVIStreamHeader));
		copydata2buffer(dec, sheader, sizeof(AVIStreamHeader), GST_CLOCK_TIME_NONE);

		copydata2buffer(dec, strf_header, 4, GST_CLOCK_TIME_NONE);
		ctx->strf_len = 0x28;
		copydata2buffer(dec, &(ctx->strf_len), 4, GST_CLOCK_TIME_NONE);
		bheader->biSize = 0x28;
		bheader->biWidth = dec->caps_width;
		bheader->biHeight = dec->caps_height;
		bheader->biPlanes = 1;
		bheader->biCompression = 24;
		bheader->biSizeImage = 0x33564944;    // ASCII CODE for "DIV3"
		bheader->biSizeImage = bheader->biWidth * bheader->biHeight * 3;
		bheader->biXPersPerMeter = 0;
		bheader->biYPersPerMeter = 0;
		bheader->biClrUsed = 0;
		bheader->biClrImportant = 0;
		copydata2buffer(dec, bheader, sizeof(BitmapInfoHeader), GST_CLOCK_TIME_NONE);

		copydata2buffer(dec, list_header, 4, GST_CLOCK_TIME_NONE);		
		ctx->list_len = 0x2a;
		copydata2buffer(dec, &(ctx->list_len) , 4, GST_CLOCK_TIME_NONE);
		copydata2buffer(dec, info_header, 4, GST_CLOCK_TIME_NONE);
		
		copydata2buffer(dec, isft_header, 4, GST_CLOCK_TIME_NONE);
		ctx->isft_len = 0x1e;
		copydata2buffer(dec, &(ctx->isft_len), 4, GST_CLOCK_TIME_NONE);
		isft_data = g_malloc(0x1e);
		strcpy(isft_data, "MEncoder dev-SVN-r24815-3.4.4.");
		copydata2buffer(dec, isft_data, 0x1e, GST_CLOCK_TIME_NONE);

		copydata2buffer(dec, junk_header, 4, GST_CLOCK_TIME_NONE);
		ctx->junk_len = 0xef2;
		copydata2buffer(dec, &(ctx->junk_len), 4, GST_CLOCK_TIME_NONE);
		junk_data = g_malloc(24);
		strcpy(junk_data, "[= Mplayer junk data! =]");
		for ( i = 0; i < (ctx->junk_len/24); i++ ){
			copydata2buffer(dec, junk_data, 24, GST_CLOCK_TIME_NONE);
		}
		copydata2buffer(dec, junk_data, (ctx->junk_len%24), GST_CLOCK_TIME_NONE);

		copydata2buffer(dec, list_header, 4, GST_CLOCK_TIME_NONE);
		ctx->list_len = 0;
		copydata2buffer(dec, &(ctx->list_len), 4, GST_CLOCK_TIME_NONE);
		copydata2buffer(dec, movi_header, 4, GST_CLOCK_TIME_NONE);
	}

	copydata2buffer(dec, vid_header, 4, GST_CLOCK_TIME_NONE);
	copydata2buffer(dec, &size, 4, GST_CLOCK_TIME_NONE);
	
}

static void frm_release(Frm_Ctrl *pfrm_ctrl, guchar frm_index)
{
	if (frm_index >= pfrm_ctrl->max_dec_frame_num) {
		return;
	}

	pfrm_ctrl->frm_info[frm_index].used = 0;
	pfrm_ctrl->frm_info[frm_index].display = 0;

	return;
}

static void frmbuf_release(struct vdec_private *ppriv, Frm_Ctrl *pfrm_ctrl)
{
	guchar i;

	for ( i = 0; i < pfrm_ctrl->max_dec_frame_num; i++){
		gst_buffer_unref(pfrm_ctrl->frm_info[i].outbuf);
	}
}


void frame_free(GstBodaDec *dec, guchar index)
{
	struct vdec_private *ppriv =(struct vdec_private *)dec->priv;
	DecInstCtl *dec_handle = ppriv->inst_handle;
	Frm_Ctrl *pfrm_ctrl = &ppriv->frm_ctrl;
	guchar frm_idx, ii, num;
	guint test;

#if 0
	for ( ii = 0; ii < dec_handle->SeqInfo.numFrameBuffers; ii++ ){
		if ( ppriv->framebuf[ii]){
			test = (guint)GST_MINI_OBJECT_REFCOUNT_VALUE(GST_MINI_OBJECT_CAST(ppriv->framebuf[ii]));
			//printf("test %d ref is %d\n", ii, test);
			//if ( gst_buffer_is_writable(ppriv->framebuf[ii]) ){
			if ( test == 1 ){
				gst_buffer_unref (ppriv->framebuf[ii]);
				printf("freed buf %x\n", ppriv->framebuf[ii]);
				printf("freed addr %x, %x\n", GST_BUFFER_DATA(ppriv->framebuf[ii]),GST_BUFFER_MALLOCDATA(ppriv->framebuf[ii]) );
				//gst_buffer_unref(ppriv->framebuf[ii]);
				ppriv->framebuf[ii] = NULL;
				frm_idx = ii;
				if ( dec_handle->BitstreamFormat == STD_MJPG ){
	    			frm_release(pfrm_ctrl, frm_idx);
	    			return;
				}
				#if 1
				printf("rls %d frame\n", ii);
				if (pfrm_ctrl->frm_info[frm_idx].display == 1) {
					pfrm_ctrl->clr_flag |= (1 << frm_idx);
	    	
				}
				#endif
				
			}
		}
	}
#endif

	{

			if ( dec_handle->BitstreamFormat == STD_MJPG ){
    			frm_release(pfrm_ctrl, index);
    			return;
			}		
	}

	VPU_DecClrDispFlag( dec_handle, ppriv->instIdx, index);
	frm_release(pfrm_ctrl, index);
}


void frm_output_dequeue(GstBodaDec *dec)
{
	printf("in deque\n");
	struct vdec_private *ppriv =(struct vdec_private *)dec->priv;
	Frm_Ctrl *pfrm_ctrl = &(ppriv->frm_ctrl);
	struct outlbuf * outdatabuf;

	printf("dequeue function\n");
	if ( pfrm_ctrl->outbuf == NULL ){
		printf("error to free frame\n");
		return;
	}

	outdatabuf = pfrm_ctrl->outbuf;
	frame_free(dec, outdatabuf->index);

	pfrm_ctrl->outbuf = outdatabuf->next;

	printf("rls outbuf start\n");
	g_free(outdatabuf);
	printf("rls outbuf end\n");
	
}

static void sync_reset(struct boda_sync_ctrl* sync_ctrl)
{
	sync_ctrl->rd = sync_ctrl->wr = sync_ctrl->cnt = 0;
	sync_ctrl->cur_block = NULL;
	sync_ctrl->head_block = NULL;
	sync_ctrl->sync_state = 0;
	sync_ctrl->sync_data = 0;
	sync_ctrl->sync_block_list = (struct boda_sync_block*)g_malloc0(sizeof(struct boda_sync_block));
	if ( sync_ctrl->sync_block_list == NULL ){
		GST_LOG("can not allocate the sync block\n");
	}
}

static struct sync_block* sync_request(struct boda_sync_ctrl* sync_ctrl)
{
	guint j;
	struct boda_sync_block* sync_list, *old_sync_list;
	struct sync_block* sync_block, *new_block;

	sync_list = sync_ctrl->sync_block_list;
	while( sync_list ){
		for ( j = 0; j < BODA_META_NUM; j++ ){
			if ( sync_list->block[j].used == 0 )  
				break;
		}
		if ( j !=  BODA_META_NUM )
			break;
		old_sync_list = sync_list;
		sync_list = sync_list->next;
	}
	if ( j ==  BODA_META_NUM ){
		sync_list = old_sync_list;
        GST_LOG("sync_block runout, allocate new sync_block!\n");
		sync_list->next = (struct boda_sync_block*)g_malloc0(sizeof(struct boda_sync_block));
		if (sync_list->next == NULL){
			GST_LOG("can not re allocate the sync block\n");
			return NULL;
		}
		j = 0;
		sync_list = sync_list->next;
	}
	
	sync_block = &(sync_list->block[j]);
	sync_block->used = 1;

	if ( sync_ctrl->head_block == NULL){
		sync_ctrl->head_block = sync_block;
	}else{
		new_block = sync_ctrl->head_block;
		while( new_block->next ){
			new_block = new_block->next;
		}
		new_block->next = sync_block;
	}
	#if 0
	sync_ctrl->wr++;
	if (sync_ctrl->wr == BODA_META_NUM) {
		sync_ctrl->wr = 0;
	}
	#endif
	sync_ctrl->cnt ++;
	//printf("<cnt+:%d>\n", sync_ctrl->cnt);
	return sync_block;
}

static struct sync_block* sync_search_addr(struct boda_sync_ctrl* sync_ctrl, guint addr)
{
	struct sync_block* sync_block;

	gushort rd_idx = sync_ctrl->rd;
	//struct sync_block* sync_block;
	//printf("searchaddr %x, start %x, end %x\n", addr, sync_ctrl->start_addr, sync_ctrl->end_addr);
	if ((sync_ctrl->cnt == 0)
		||((sync_ctrl->start_addr<=sync_ctrl->end_addr) && (addr<sync_ctrl->start_addr ||addr>sync_ctrl->end_addr))
		||((sync_ctrl->start_addr>sync_ctrl->end_addr) && (addr<sync_ctrl->start_addr && addr>sync_ctrl->end_addr))) {
		return NULL;
	}
	sync_block = sync_ctrl->head_block;
	while(sync_block){
		if ( sync_block->end_addr > sync_block->start_addr ){
			if ( (addr > sync_block->end_addr)  || (addr < sync_block->start_addr)){
				sync_ctrl->head_block = sync_block->next;
				sync_block->next = NULL;
				sync_block->used = 0;
				sync_ctrl->cnt --;
				//printf("<cnt-:%d, %x>\n", sync_ctrl->cnt, sync_ctrl->head_block);
				sync_block = sync_ctrl->head_block;
			}else{
				break;
			}
		}else{
			if ( (addr > sync_block->end_addr)  && (addr < sync_block->start_addr)){
				sync_ctrl->head_block = sync_block->next;
				sync_block->next = NULL;
				sync_block->used = 0;
				sync_ctrl->cnt --;
				//printf("<cnt-:%d, %x>\n", sync_ctrl->cnt, sync_ctrl->head_block);
				sync_block = sync_ctrl->head_block;
			}else{
				break;
			}			
		}
	}	
	if( sync_block ){
		if ((addr >= sync_block->start_addr && addr <= sync_block->end_addr)
			||((sync_block->start_addr > sync_block->end_addr)
				&& (addr >= sync_block->start_addr || addr <= sync_block->end_addr))){
			sync_ctrl->head_block = sync_block->next;
			sync_block->next = NULL;
			sync_block->used = 0;
			sync_ctrl->cnt --;
			//printf("<cnt-:%d, %x>\n", sync_ctrl->cnt, sync_ctrl->head_block);
		    if (sync_ctrl->cnt >0) {
        		sync_ctrl->start_addr = sync_ctrl->head_block->start_addr;
    		}				
		}else{
			sync_block = NULL;
		}
	}
	return sync_block;
}
#if 0
static struct sync_block* cur_block(struct boda_sync_ctrl* sync_ctrl)
{
	if (sync_ctrl->cnt == 0) {
		return NULL;
	}
	if (sync_ctrl->wr == 0) {
	    return &(sync_ctrl->block[BODA_META_NUM-1]);
	} else {
	    return &(sync_ctrl->block[sync_ctrl->wr-1]);
	}
}
#endif
static guint get_config_end(Vbv_Ctrl *pvbv)
{
	guint config_end;

	if ( pvbv->valid_size > VBV_RESERVE_DATA )
		config_end = (pvbv->writep-1-VBV_RESERVE_DATA);
	else
		return 0xFFFFFFFF; 

	if(config_end < pvbv->start)
		config_end = pvbv->end + 1 - (pvbv->start - config_end);
	if(config_end > pvbv->end) {
		config_end = config_end - pvbv->end - 1 + pvbv->start;
	}
	return config_end;
}

static gboolean vbv_get_valid(Vbv_Ctrl *pvbv)
{
	if(pvbv->valid_size > VBV_RESERVE_DATA )
		return TRUE;
	else
		return FALSE;
}

static void vbv_release_data(Vbv_Ctrl *pvbv)
{
	struct inlbuf *streambuf;

	streambuf = pvbv->databuf;
	while(streambuf) {
		pvbv->databuf = streambuf->next;
		gst_buffer_unref(streambuf->inbuf); 
		g_free(streambuf);
		streambuf = pvbv->databuf;
		
	}

	/*reset vbv buffer*/
	pvbv->readp = pvbv->writep = pvbv->start;
	pvbv->valid_size = 0;
	
}

static gint gst_boda_allocate_buffer(GstBodaDec *dec)
{
	//struct video_picture_meta* pmeta;	
	struct vdec_private *ppriv = (struct vdec_private *)dec->priv;
	DecInstCtl *handle;
	DecSeqGetInfo *GetInfo;
	Frm_Ctrl *pfrm_ctrl;
	GstBuffer *outbuf = NULL;
	GstFlowReturn ret = GST_FLOW_OK;
	guchar frame_num = 0;
	guint lakers_frm_y_size, lakers_frm_c_size, lakers_frm_mv_size, height;
	guchar i;
	guint frm_addr = VDEC_FRAME_BUF_ADDR;

	handle = ppriv->inst_handle;
	GetInfo = &handle->SeqInfo;
	pfrm_ctrl = &ppriv->frm_ctrl;

	printf("allocate frame buffer\n");
	//GetInfo->stride = lakers_frm_y_size =  (GetInfo->picWidth + 255 )>> 8 <<8;	//y: 256 bytes aligned
	GetInfo->stride = lakers_frm_y_size = GST_ROUND_UP_16(GetInfo->picWidth);
	height = GST_ROUND_UP_16(GetInfo->picHeight);		// 32 bytes aligned
	//height = ( GetInfo->picHeight + 1 ) & 0xFFFFFFFE;
	GetInfo->picHeight = height;
	lakers_frm_y_size *= height;
	printf("allocate width %d, height %d\n", GetInfo->picWidth, height);

	if ( handle->BitstreamFormat != STD_MJPG ) {
		/* 4:2:0 */
		lakers_frm_c_size = lakers_frm_y_size >> 2;
		if ( handle->BitstreamFormat != STD_MP2)
			lakers_frm_mv_size = (MAX_FRAME_MVL_STRIDE * MAX_FRAME_MVL_HEIGHT) >> 2;
		else
			lakers_frm_mv_size = 0;
	} else {
	#if 0
		/* mjpeg format */
		if ( GetInfo->mjpg_sourceFormat == YCBCR420) {
			lakers_frm_c_size = lakers_frm_y_size >> 2;
		} else if ( GetInfo->mjpg_sourceFormat == YCBCR422H || GetInfo->mjpg_sourceFormat == YCBCR422V ) {
			lakers_frm_c_size = lakers_frm_y_size >> 1;
		} else if ( GetInfo->mjpg_sourceFormat == YCBCR444 ) {
			lakers_frm_c_size = lakers_frm_y_size;
		} else if ( GetInfo->mjpg_sourceFormat == YCBCR400 ) {
			lakers_frm_c_size = 0;
		}
	#endif
		lakers_frm_c_size = lakers_frm_y_size;
		lakers_frm_mv_size = 0;
	}

	dec->outsize = (GST_ROUND_UP_8(GetInfo->picWidth) * GST_ROUND_UP_2(GetInfo->picHeight)) * 3 / 2;;
	pfrm_ctrl->frm_info[0].size[0] = lakers_frm_y_size;
	pfrm_ctrl->frm_info[0].size[1] = lakers_frm_c_size;
	pfrm_ctrl->frm_info[0].size[2] = lakers_frm_c_size;
	pfrm_ctrl->frm_info[0].size[3] = lakers_frm_mv_size;	
	gst_boda_dec_negotiate (dec, GetInfo->picWidth, GetInfo->picHeight);
	do{
		pfrm_ctrl->frm_info[frame_num].used = 0;
		pfrm_ctrl->frm_info[frame_num].size[0] = lakers_frm_y_size;
		pfrm_ctrl->frm_info[frame_num].size[1] = lakers_frm_c_size;
		pfrm_ctrl->frm_info[frame_num].size[2] = lakers_frm_c_size;
		pfrm_ctrl->frm_info[frame_num].size[3] = lakers_frm_mv_size;
		
		outbuf = NULL;
        //gst_boda_dec_negotiate (dec, GetInfo->picWidth, GetInfo->picHeight);
	    ret = gst_pad_alloc_buffer_and_set_caps (dec->srcpad, GST_BUFFER_OFFSET_NONE,
    	    dec->outsize, GST_PAD_CAPS (dec->srcpad), &outbuf);
	    if (G_UNLIKELY (ret != GST_FLOW_OK)){
			printf("can not allocate frame buffer before register framebuffer\n");
			break;
		}
		
		pfrm_ctrl->frm_info[frame_num].outbuf = outbuf;
		//pfrm_ctrl->frm_info[frame_num].frm_buffer.bufY = GST_BUFFER_DATA(outbuf);
		if ( outbuf->_gst_reserved[0]){
			pfrm_ctrl->frm_info[frame_num].frm_buffer.bufY = (guint)(outbuf->_gst_reserved[0]);
			pfrm_ctrl->buffromv4l = 0;
		}else{
			pfrm_ctrl->frm_info[frame_num].frm_buffer.bufY = frm_addr;
			pfrm_ctrl->buffromv4l = 1;
			if (frame_num >= (GetInfo->numFrameBuffers+4) )
				break;
		}
		pfrm_ctrl->frm_info[frame_num].frm_buffer.bufCb = pfrm_ctrl->frm_info[frame_num].frm_buffer.bufY + lakers_frm_y_size;
		pfrm_ctrl->frm_info[frame_num].frm_buffer.bufCr = pfrm_ctrl->frm_info[frame_num].frm_buffer.bufCb + lakers_frm_c_size;
		//pfrm_ctrl->frm_info[frame_num].frm_buffer.bufMvCol = pfrm_ctrl->frm_info[frame_num].frm_buffer.bufCr + lakers_frm_c_size;
		pfrm_ctrl->frm_info[frame_num].frm_buffer.bufMvCol = pfrm_ctrl->mv_base_addr + frame_num * lakers_frm_mv_size;

		handle->SeqInfo.frameBufPool[frame_num] = pfrm_ctrl->frm_info[frame_num].frm_buffer;
		frm_addr += (lakers_frm_y_size + 2 * lakers_frm_c_size);

		printf("\nFRM(%d):Y <%x>",frame_num,pfrm_ctrl->frm_info[frame_num].frm_buffer.bufY);
		printf("\nFRM(%d):Cb<%x>",frame_num,pfrm_ctrl->frm_info[frame_num].frm_buffer.bufCb);
		printf("\nFRM(%d):Cr<%x>",frame_num,pfrm_ctrl->frm_info[frame_num].frm_buffer.bufCr);
		printf("\nFRM(%d):MV<%x>",frame_num,pfrm_ctrl->frm_info[frame_num].frm_buffer.bufMvCol);
		
		frame_num++;
	}while(1);

	if ( GetInfo->numFrameBuffers > frame_num ){
		printf("allocate buffer from v4l2 %d is less than the decoder request %d\n", frame_num, GetInfo->numFrameBuffers);
		g_assert(0);
	}else{
		printf("allocate buffer from v4l2 %d\n", frame_num);
		GetInfo->numFrameBuffers = frame_num;
	}

	GetInfo->allUsedBuffers = 0;
	for (i = 0; i < GetInfo->numFrameBuffers; i++){
		GetInfo->allUsedBuffers |= (1 << i);
	}	

	pfrm_ctrl->max_dec_frame_num = GetInfo->numFrameBuffers;
	for (i = 0; i < pfrm_ctrl->max_dec_frame_num; i++ )
	{	
		frm_set_state(pfrm_ctrl, i, 0);
	}	

#ifdef ADD_ON_CONF
	for ( i = GetInfo->numFrameBuffers; i < (GetInfo->numFrameBuffers+ADD_ON_FRAME); i++ ){
		outbuf = NULL;
		gst_boda_dec_negotiate (dec, GetInfo->picWidth, GetInfo->picHeight);
	    ret = gst_pad_alloc_buffer_and_set_caps (dec->srcpad, GST_BUFFER_OFFSET_NONE,
    	    dec->outsize, GST_PAD_CAPS (dec->srcpad), &outbuf);
	    if (G_UNLIKELY (ret != GST_FLOW_OK)){
			GST_LOG("can not allocate frame buffer before register framebuffer\n");
			g_assert(0);
		}		
		pfrm_ctrl.frm_info[i].used = 0;
		pfrm_ctrl.frm_info[i].size[0] = lakers_frm_y_size;
		pfrm_ctrl.frm_info[i].size[1] = lakers_frm_c_size;
		pfrm_ctrl.frm_info[i].size[2] = lakers_frm_c_size;
		pfrm_ctrl.frm_info[i].size[3] = lakers_frm_mv_size;
		pfrm_ctrl.frm_info[i].frm_buffer.bufY = GST_BUFFER_DATA(outbuf);
		pfrm_ctrl.frm_info[i].frm_buffer.bufCb = pfrm_ctrl.frm_info[i].frm_buffer.bufY + lakers_frm_y_size;
		pfrm_ctrl.frm_info[i].frm_buffer.bufCr = pfrm_ctrl.frm_info[i].frm_buffer.bufCb + lakers_frm_c_size;
		
	}
#endif

    return 1;
}


static void vbv_update_readp(Vbv_Ctrl *pvbv, guint new_readp)
{
	pvbv->readp = new_readp;
	//pvbv->valid_size = (pvbv->writep >= pvbv->readp)?(pvbv->writep - pvbv->readp):(pvbv->writep - pvbv->start + pvbv->end + 1 - pvbv->readp);
	if (pvbv->writep == pvbv->readp)
		pvbv->valid_size = pvbv->end + 1- pvbv->start;
	else
		pvbv->valid_size = (pvbv->writep >= pvbv->readp)?(pvbv->writep - pvbv->readp):(pvbv->writep - pvbv->start + pvbv->end + 1 - pvbv->readp);
	
}


static void VideoDecoder_Init(GstBodaDec * dec)
{
  struct vdec_private *ppriv;
  DecInstCtl *handle;
  guint vpu_addr;
  guchar i;

  ppriv = (struct vdec_private *)g_malloc0(sizeof(struct vdec_private));
  dec->priv = (gpointer)ppriv;

  handle = (DecInstCtl *)g_malloc0(sizeof(DecInstCtl));
  ppriv->inst_handle = handle;

  /* Init the memory and reg */
  bsp_postboot_init();

  handle->RegBase = (guint)hal_paddr_to_vaddr((gpointer)VDEC_REG_BASE_ADDR);
  vpu_addr = (guint)hal_paddr_to_vaddr((gpointer)VDEC_MEM_BASE_ADDR);
  ppriv->vbv_ctrl.start= VDEC_MEM_BASE_ADDR + TOTAL_VPUBUF_SIZE;
  ppriv->vbv_ctrl.end = ppriv->vbv_ctrl.start + VDEC_STREAM_BUF_SIZE - 1;
  ppriv->frm_ctrl.base_addr = ppriv->vbv_ctrl.start + VDEC_STREAM_BUF_SIZE;
  ppriv->frm_ctrl.size = VDEC_FRAME_BUF_SIZE;
  ppriv->frm_ctrl.mv_base_addr = VDEC_MV_BUF_ADDR;
  
  ppriv->VPU_buffer.code_buf = vpu_addr;
  ppriv->VPU_buffer.work_buf = ppriv->VPU_buffer.code_buf + CODE_BUF_SIZE;
  ppriv->VPU_buffer.para_buf = ppriv->VPU_buffer.work_buf + WORK_BUF_SIZE + PARA_BUF2_SIZE;
  ppriv->VPU_buffer.slice_buf = ppriv->VPU_buffer.para_buf + PARA_BUF_SIZE;
  ppriv->VPU_buffer.ps_buf[0] = ppriv->VPU_buffer.slice_buf + SLICE_BUF_SIZE;
  for ( i = 1; i < MAX_NUM_INSTANCE; i++) {
	 ppriv->VPU_buffer.ps_buf[i] =  ppriv->VPU_buffer.ps_buf[i-1] + PS_BUF_SIZE;
  }

  VPU_Init(&(ppriv->VPU_buffer), handle);

  EnableVPUInt(handle);
  ppriv->instIdx = 0;
  ppriv->decoded_first_frame = 0;
  ppriv->frm_ctrl.disp_flag = 0xff;
  ppriv->frm_ctrl.pre_idx = ppriv->frm_ctrl.disp_idx;
  ppriv->frm_ctrl.disp_idx = 0xff;
  ppriv->frm_ctrl.clr_flag = 0;
  ppriv->frm_ctrl.IPframe = 0;
  ppriv->frm_ctrl.output_en = 0;
  ppriv->frm_ctrl.last_dec_frame = 1;
  ppriv->frm_ctrl.outbuf = NULL;
  ppriv->frm_ctrl.get_pts_valid = 0;

  ppriv->vbv_ctrl.readp = ppriv->vbv_ctrl.writep = ppriv->vbv_ctrl.start;
  ppriv->vbv_ctrl.config_writep = ppriv->vbv_ctrl.start;
  ppriv->vbv_ctrl.valid_size = 0;
  ppriv->vbv_ctrl.config_validsize = 0;
  ppriv->vbv_ctrl.vbv_vo_status = 0;
  ppriv->vbv_ctrl.first_data = 1;
  ppriv->vbv_ctrl.databuf = NULL;
  ppriv->vbv_ctrl.threshold = 0x80000;
  ppriv->dec_status = IDLE_STATE;
  ppriv->stop_status = IDLE_STATE;

  g_atomic_int_set(&(ppriv->vbv_ctrl.vo_status), 0);

  handle->SeqOpenParam.streamBufStartAddr = ppriv->vbv_ctrl.start;
  handle->SeqOpenParam.streamBufSize = ppriv->vbv_ctrl.end + 1 - ppriv->vbv_ctrl.start;
  handle->SeqOpenParam.filePlayEnable = 0;
  handle->SeqOpenParam.picWidth = 720;
  handle->SeqOpenParam.picHeight = 576;
  handle->SeqOpenParam.dynamicAllocEnable = 0;
  
  VPU_DecOpen(ppriv->instIdx, handle);

  sync_reset(&ppriv->sync_ctrl);
 // ppriv->sync_ctrl.block[ppriv->sync_ctrl.wr].start_addr = ppriv->vbv_ctrl.start;
  (ppriv->sync_ctrl.sync_block_list)->block[0].start_addr = ppriv->vbv_ctrl.start;
  ppriv->wait_finish = 0;

}

static void copydata2buffer(GstBodaDec * dec, guchar *data, guint size, GstClockTime pts)
{
    struct vdec_private *ppriv;
	Vbv_Ctrl *pvbv;
	struct boda_sync_ctrl	*sync_ctrl;  

    ppriv = (struct vdec_private *)dec->priv;
    pvbv = &ppriv->vbv_ctrl;
    sync_ctrl = &ppriv->sync_ctrl;

	while ((pvbv->end + 1- pvbv->start - pvbv->valid_size) < (size+pvbv->threshold)) {
	    //vdec_lakers_decode_frame(dev);
		//osal_mutex_unlock(dev->mutex_id);	
		GST_DEBUG("input buffer overflow, %x, valid_size %x\n", (guint)pvbv->writep, (guint)pvbv->valid_size);
		pvbv->vbv_vo_status = 1;
		g_mutex_unlock(dec->buflock);	  
		sem_wait(&dec->push_sem);
		g_mutex_lock(dec->buflock);
  		//g_atomic_int_add(&pvbv->vo_status, -1);
	} 

	if (size){
    	if ((pvbv->writep + size) < (pvbv->end + 1)) {
			memcpy((gpointer)(hal_paddr_to_vaddr((gpointer)pvbv->writep)), (gpointer)(data), size);
#ifdef DATA_DEBUG		
			fwrite(data, 1, size, fp);
#endif
		} else {
			memcpy((gpointer)(hal_paddr_to_vaddr((gpointer)pvbv->writep)), (gpointer)(data), pvbv->end + 1 - pvbv->writep);
#ifdef DATA_DEBUG
			fwrite(data, 1, (pvbv->end + 1 - pvbv->writep), fp);
#endif
			memcpy((gpointer)(hal_paddr_to_vaddr((gpointer)pvbv->start)), (gpointer)(data + pvbv->end + 1 - pvbv->writep), size - (pvbv->end + 1 - pvbv->writep));
#ifdef DATA_DEBUG
			fwrite((data+(pvbv->end + 1 - pvbv->writep)), 1, (size - (pvbv->end + 1 - pvbv->writep)), fp );
#endif
		}	

		if ( pts != GST_CLOCK_TIME_NONE ){
  			sync_ctrl->cur_block = sync_request(sync_ctrl);
			if (sync_ctrl->cur_block) {
    			sync_ctrl->cur_block->pts = pts;
				sync_ctrl->cur_block->start_addr = pvbv->writep;
    			if (sync_ctrl->cnt == 1) {//first block
    				sync_ctrl->start_addr = sync_ctrl->cur_block->start_addr;
	    		}	
			} 
		}

	
		if((pvbv->writep + size) < (pvbv->end + 1)) {
			pvbv->writep = pvbv->writep + size;
		} else {
			pvbv->writep = pvbv->start + (pvbv->writep + size - (pvbv->end + 1));
		}
		if (pvbv->writep == pvbv->readp)
			pvbv->valid_size = pvbv->end + 1- pvbv->start;
		else
			pvbv->valid_size = (pvbv->writep >= pvbv->readp)?(pvbv->writep - pvbv->readp):(pvbv->writep - pvbv->start + pvbv->end + 1 - pvbv->readp);

		if (sync_ctrl->cur_block) {
			sync_ctrl->cur_block->end_addr = pvbv->writep;
			if (sync_ctrl->cur_block->end_addr == pvbv->start) {
				sync_ctrl->cur_block->end_addr = pvbv->end;
			} else {
				sync_ctrl->cur_block->end_addr -= 1;
			}
			sync_ctrl->end_addr = sync_ctrl->cur_block->end_addr;
			//printf("st %x,end %x\n", sync_ctrl->cur_block->start_addr, sync_ctrl->cur_block->end_addr);
		}
	}

}

static gboolean StreamInBuf(GstBodaDec * dec)
{
  struct vdec_private *ppriv;
  DecInstCtl *handle;  
  Vbv_Ctrl *pvbv;
  guchar *in_data;
  guint in_size;
  GstClockTime pts, duration;
  struct inlbuf *streambuf;
  gint ret;
  gboolean result;

  ppriv = (struct vdec_private *)dec->priv;
  handle = ppriv->inst_handle;
  pvbv = &ppriv->vbv_ctrl;

  if ( pvbv->databuf == NULL ){
  	return TRUE;
  }

  streambuf = pvbv->databuf;

  while ( streambuf ){

	
    in_size = GST_BUFFER_SIZE(streambuf->inbuf);
    in_data = GST_BUFFER_DATA(streambuf->inbuf);
#if 0
  printf("in_data %x\n", (guint)in_data);
  if ( in_data ){
  	int i;
  	for (i = 0; i < 256; i++){
		printf("%x ", *(in_data+i));
  	}
	printf("\n");
  }
  #endif
    pts = GST_BUFFER_TIMESTAMP (streambuf->inbuf);
    duration = GST_BUFFER_DURATION (streambuf->inbuf);  
	
    //printf("input size %x\n", in_size);
	#if 0
	if ( dec->codec_data_size && dec->codec_data ){
		printf("extra size %x data[%x], [%x], [%x], [%x]\n", dec->codec_data_size, dec->codec_data[0], dec->codec_data[1], dec->codec_data[2], dec->codec_data[3]);
	}
	#endif

    if (pvbv->vbv_share == 0){
	  ret = 0;
      if ( handle->BitstreamFormat == STD_AVC){
	  	ret = avc_mp4toannexb_filter(dec, in_data, in_size);
      }else if (  handle->BitstreamFormat == STD_MP4 ){
        struct MP4BSContext *ctx = (struct MP4BSContext *)dec->priv_data;
		if ( ctx ){
			if ( ctx && !ctx->first_idr ){
	        	mp4filter(dec, dec->codec_data, dec->codec_data_size);
				ctx->first_idr = 1;
    	    }
		}
      }else if ( handle->BitstreamFormat == STD_VC1){
        struct VC1BSContext *ctx = (struct VC1BSContext *)dec->priv_data;
		if (ctx){
			ctx->frame_size = in_size;
			if ( dec->codec_data_size )
	      		wmvfilter(dec, dec->codec_data, dec->codec_data_size);
		}
      }else if ( handle->BitstreamFormat == STD_DIV3){
        struct Divx3BSContext *ctx = (struct Divx3BSContext *)dec->priv_data;
      	divx3filter(dec, ctx, in_size);
      }

	  if ( !ret ){
	  	copydata2buffer(dec, in_data, in_size, pts);
	  }
	  	
	  pvbv->databuf = streambuf->next;
	  gst_buffer_unref(streambuf->inbuf);  
	  g_free(streambuf);
	  streambuf = pvbv->databuf;
	  //printf("%x\n", (guint)streambuf);
	}
		
    
#if 0
    pvbv->config_end = get_config_end(pvbv);
//printf("config end %x, writep %x\n", pvbv->config_end, pvbv->writep);
    if ( pvbv->config_end == 0xFFFFFFFF ){
	  //return TRUE;
    } else {

      VPU_DecUpdateBitstreamBuffer(handle, ppriv->instIdx, pvbv->config_end, 1);
    }
  
 #else
	 VPU_DecUpdateBitstreamBuffer(handle, ppriv->instIdx, pvbv->writep, 1);
 #endif
 }

  if ( ppriv->dec_status == IDLE_STATE ) {
	ppriv->dec_status = SEQ_START_STATE;
  }
	//gst_task_start (dec->task);
//	result = gst_pad_start_task (dec->sinkpad, (GstTaskFunction) gst_boda_dec_loop, dec->sinkpad);
	//pthread_create(&dec_thread, NULL, &gst_boda_dec_loop, dec);
	//printf("dec_loop: PPID: 0x%08x -> PID: %d\n", pthread_self(), dec_thread)
	//uio_init_interrupt();
	//hal_exception_set_mask(dec->fd);
  if ( ppriv->dec_status == SEQ_START_STATE || ppriv->dec_status == PICBIT_START_STATE){
	gst_boda_dec_frame(dec);
  }
  
  return TRUE;
}

static gboolean VideoStreamInput(GstBodaDec * dec, GstBuffer * buf)
{
  struct vdec_private *ppriv;
  DecInstCtl *handle;  
  Vbv_Ctrl *pvbv;
  struct inlbuf *indatabuf;
  guint readp;
  guchar *input_data;
  guint input_size;
  static guint size = 0;
  GstClockTime bufpts, bufduration;


  ppriv = (struct vdec_private *)dec->priv;
  handle = ppriv->inst_handle;
  pvbv = &ppriv->vbv_ctrl;

  input_size = GST_BUFFER_SIZE(buf);
  input_data = GST_BUFFER_DATA(buf);
  size += input_size;
  if ( size >= 0x100000 )
  	size -= 0x100000;
  //printf("in %x, %x, %x\n", input_size, size, (guint)input_data);
  bufpts = GST_BUFFER_TIMESTAMP (buf);
  bufduration = GST_BUFFER_DURATION (buf);
  //printf("pts %lld, dur %lld\n",bufpts, bufduration);
  GST_DEBUG("pts %lld, dur %lld\n",bufpts, bufduration);

#if 0
  if (ppriv->dec_status == IDLE_STATE || ppriv->dec_status == SEQ_START_STATE || ppriv->dec_status == SEQ_CONTINUE_STATE){
	readp = GetNewNormalReadPtr(handle, ppriv->instIdx);
  }else{
	readp = GetNewReadPtr(handle, ppriv->instIdx);
  }
#endif
  readp = GetNewNormalReadPtr(handle, ppriv->instIdx);

  if ( readp != 1 )
  	pvbv->readp = readp;

  if (pvbv->first_data == 1) {
	if ((guint)(hal_vaddr_to_paddr((gpointer)input_data)) >= pvbv->start && (guint)(hal_vaddr_to_paddr((gpointer)input_data)) <= pvbv->end ) {
		pvbv->vbv_share = 1;
	} else {
		pvbv->vbv_share = 0;
	}
	pvbv->first_data = 0;
  } else if (pvbv->vbv_share == 0) {
	if ((guint)(hal_vaddr_to_paddr((gpointer)input_data)) >= pvbv->start && (guint)(hal_vaddr_to_paddr((gpointer)input_data)) <= pvbv->end ) {
		GST_ERROR("plbuf->buf_address invalid 1\n");			
		g_assert(0);
	}
  } else if (pvbv->vbv_share == 1) {
	if ((guint)(hal_vaddr_to_paddr((gpointer)input_data)) < pvbv->start || (guint)(hal_vaddr_to_paddr((gpointer)input_data)) > pvbv->end ) {
		GST_ERROR("plbuf->buf_address invalid 2\n");			
		g_assert(0);			
	}
  }

  if ( pvbv->databuf == NULL ){
  	pvbv->databuf = (struct inlbuf *)g_malloc0(sizeof(struct inlbuf));
	if ( pvbv->databuf ){
	  pvbv->databuf->inbuf = buf;
	  pvbv->databuf->next = NULL;
	 // printf("add buf0 %x\n", input_size);
	}else{
	  printf("error to allocate memory0\n");
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
	  printf("error to allocate memory1\n");
	}
  }

  if (pvbv->vbv_share == 1) {
	if ((guint)(hal_vaddr_to_paddr((gpointer)input_data)) != pvbv->writep) {
		GST_LOG("plbuf->buf_address invalid 3 %x\n", (guint)(input_data));	
		return FALSE;			
	}
	if ((pvbv->end + 1- pvbv->start - pvbv->valid_size) < input_size) {
		GST_WARNING("pvbv overflow, dmx control failed!\n");			
		g_assert(0);
	}
		
	if((pvbv->writep + input_size) < (pvbv->end + 1)) {
		pvbv->writep = pvbv->writep + input_size;
	} else {
		pvbv->writep = pvbv->start + (pvbv->writep + input_size - (pvbv->end + 1));
	}
	pvbv->valid_size = (pvbv->writep >= pvbv->readp)?(pvbv->writep - pvbv->readp):(pvbv->writep - pvbv->start + pvbv->end + 1 - pvbv->readp);
	//PRINTF("VBV+<%d>\n",pvbv->valid_size);
		
  } else {
	StreamInBuf(dec);
//printf("inbuf out\n");
  }  

#if 0
  sync_ctrl->cur_block = sync_request(sync_ctrl);
  if (sync_ctrl->cur_block) {
    sync_ctrl->cur_block->pts = pts;
	if ( pvbv->writep != pvbv->start )
	{
      sync_ctrl->cur_block->end_addr = pvbv->writep-1;
	}else
	{
	  sync_ctrl->cur_block->end_addr = pvbv->end;
	}
	sync_ctrl->block[sync_ctrl->wr].start_addr = pvbv->writep;
    if (sync_ctrl->cnt == 1) {//first block
      sync_ctrl->start_addr = sync_ctrl->cur_block->start_addr;
    }	
  } else {
//  	GST_WARNING("get sync block failure\n");
    sync_ctrl->cur_block = cur_block(sync_ctrl);
  }

  pvbv->config_end = get_config_end(pvbv);
  if ( pvbv->config_end == 0xFFFFFFFF ){
	return TRUE;
  }

  VPU_DecUpdateBitstreamBuffer(handle, ppriv->instIdx, pvbv->config_end, 1);

  if ( ppriv->dec_status == IDLE_STATE ) {
	ppriv->dec_status = SEQ_START_STATE;
  }
#endif

  return TRUE;

}

static void gst_boda_dec_frame(GstBodaDec *dec)
{
  struct vdec_private *ppriv;
  DecInstCtl *handle;  
  Vbv_Ctrl *pvbv_ctrl;
  Frm_Ctrl *pfrm_ctrl;
  struct boda_sync_ctrl	*sync_ctrl;  
  struct sync_block* sync_block;
  guchar frm_idx, i; 
  GstBuffer *outbuf;
  GstFlowReturn retflow = GST_FLOW_OK;
  CodecRetCode ret;
  
  ppriv = (struct vdec_private *)dec->priv;
  handle = ppriv->inst_handle;
  pvbv_ctrl = &ppriv->vbv_ctrl;
  pfrm_ctrl = &ppriv->frm_ctrl;
  sync_ctrl=&ppriv->sync_ctrl;  

  if ( ppriv->dec_status ==  VPU_STOP_STATE || ppriv->dec_status == IDLE_STATE) {
	return;
  }

  if(ppriv->dec_status == SEQ_START_STATE) {
	/* check the decoder working status */
	if ( vbv_get_valid(pvbv_ctrl) ) {
		/* set escape mode: 0, if not sufficient data fed in, the VPU will wait until there is enough data,  for just one instance  */
		VPU_DecSetEscSeqInit(handle, 0);
		//ppriv->start_time = hal_get_cpu_tick();
		if( handle->BitstreamFormat == STD_AVC ) {
			handle->SeqOpenParam.psSaveBuffer = (guint)hal_vaddr_to_paddr((gpointer)(ppriv->VPU_buffer.ps_buf[ppriv->instIdx]));
			handle->SeqOpenParam.psSaveBufferSize = PS_BUF_SIZE;
		}
		handle->SeqOpenParam.reorderEnable = 1;
		pvbv_ctrl->readp = pvbv_ctrl->start;
		pvbv_ctrl->valid_size = pvbv_ctrl->writep - pvbv_ctrl->readp;
		//printf("INIT readp %x\n", pvbv->readp);
		if ( pvbv_ctrl->valid_size ){
			if ( VPU_DecInit(handle, ppriv->instIdx) == RETCODE_WRONG_CALL_SEQUENCE ){
				guint new_readp;
			
				//new_readp = GetNewReadPtr(handle, ppriv->instIdx);
				new_readp = GetNewNormalReadPtr(handle, ppriv->instIdx);
				vbv_update_readp(pvbv_ctrl, new_readp);
				//vbv_update_inputbuf(pInputQueue, new_readp);
				return;
			}
			ppriv->dec_status = SEQ_CONTINUE_STATE;
		} 
	} 
	return;

  }

  if (ppriv->dec_status == SET_FRAMEBUF_STATE) {
	VPU_DecSetEscSeqInit(handle, 0);
	gst_boda_allocate_buffer(dec);
	ret = VPU_DecRegisterFrameBuffer(handle, &ppriv->VPU_buffer, ppriv->instIdx);
	if ( ret != RETCODE_SUCCESS ) {
		GST_LOG("initialize the frame buffer error %x!\n", (guint)ret);
	}
	ppriv->dec_status = SET_FRAMEBUF_CONTINUE_STATE;
	return;
  }

  if (ppriv->dec_status == PICFRM_START_STATE) {
	#if 0
	if ( pvbv_ctrl.vbv_vo_status == 1 ){
		vdec_lakers_bsbuffer_flush();
		dec_status = PICBIT_CONTINUE_STATE;
		printf("flush\n");
		return;
	}	
	#endif
	if ( ppriv->decoded_first_frame != 0  ) {
		if ( handle->BitstreamFormat == STD_MJPG ){
			frm_idx = frm_request(pfrm_ctrl, handle);
			if ( frm_idx == 0xff ){
				return;
			}
			VPU_DecGiveCommand(handle, ppriv->instIdx, SET_ROTATION_OUTPUT, &(pfrm_ctrl->frm_info[frm_idx].frm_buffer) );
			VPU_DecGiveCommand(handle, ppriv->instIdx, SET_ROTATION_STRIDE, &(handle->SeqInfo.stride) );
			pfrm_ctrl->cur_idx = frm_idx;
			ppriv->dec_status = PICBIT_START_STATE;
		}else{
			outbuf = NULL;
			if (!pfrm_ctrl->buffromv4l){
				//if ( (handle->SeqInfo.allUsedBuffers == VPU_GetDispFlag(handle, ppriv->instIdx)) || handle->FrameOutInfo.indexFrameDecoded == -1){	
					//no enough buffer for decode, get buffer from v4l2 sink
					do{
						//gst_boda_dec_negotiate (dec, handle->SeqInfo.picWidth, handle->SeqInfo.picHeight);
						retflow = gst_pad_alloc_buffer_and_set_caps (dec->srcpad, GST_BUFFER_OFFSET_NONE,
    			    		dec->outsize, GST_PAD_CAPS (dec->srcpad), &outbuf);
						if ( outbuf ){
							for ( i = 0; i < pfrm_ctrl->max_dec_frame_num; i++){
								/* free frame buffer */
								//if ( pfrm_ctrl->frm_info[i].frm_buffer.bufY == GST_BUFFER_DATA(outbuf)){
								if ( pfrm_ctrl->frm_info[i].frm_buffer.bufY == (guint)outbuf->_gst_reserved[0]){
									frame_free(dec, i);
									ppriv->dec_status = PICBIT_START_STATE;
									break;
								}
							}
						}
						if ( (retflow == GST_FLOW_OK) && (i == pfrm_ctrl->max_dec_frame_num) ){
							printf("buf from src pad error %x\n", (guint)GST_BUFFER_DATA(outbuf));
							g_assert(0);
						}
					}while(retflow == GST_FLOW_OK);
				//}else{
				//	ppriv->dec_status = PICBIT_START_STATE;
				//}
			}else{
				// get buffer from the other sink except the v4l2 sink
				guchar ii, test;
			
				for ( ii = 0; ii < pfrm_ctrl->max_dec_frame_num; ii++ ){
					if ( pfrm_ctrl->frm_info[ii].outbuf){
						test = (guint)GST_MINI_OBJECT_REFCOUNT_VALUE(GST_MINI_OBJECT_CAST(pfrm_ctrl->frm_info[ii].outbuf));
						if ( test == 0 ){
							pfrm_ctrl->frm_info[ii].outbuf = NULL;
							frm_idx = ii;
							frame_free(dec, frm_idx);
							retflow = gst_pad_alloc_buffer_and_set_caps (dec->srcpad, GST_BUFFER_OFFSET_NONE,
    			    			dec->outsize, GST_PAD_CAPS (dec->srcpad), &outbuf);
							pfrm_ctrl->frm_info[ii].outbuf = outbuf;
							ppriv->dec_status = PICBIT_START_STATE;
						}
					}
				}

			}
			
			
			if ( ppriv->dec_status != PICBIT_START_STATE ){
				if (handle->FrameOutInfo.indexFrameDecoded == -1 && handle->FrameOutInfo.indexFrameDisplay == -3){
					return;
				}else{
					ppriv->dec_status = PICBIT_START_STATE;
				}
			}
			//printf("dec status %d\n",ppriv->dec_status);
		}
	}else {
		if ( handle->BitstreamFormat == STD_MJPG ){
			pfrm_ctrl->cur_idx = 0;
			frm_set_state(pfrm_ctrl, 0, 1);
			VPU_DecGiveCommand(handle, ppriv->instIdx, SET_ROTATION_OUTPUT, &(pfrm_ctrl->frm_info[0].frm_buffer) );
			VPU_DecGiveCommand(handle, ppriv->instIdx, SET_ROTATION_STRIDE, &(handle->SeqInfo.stride) );
		}
		ppriv->dec_status = PICBIT_START_STATE;
	}
	
  }

  if(ppriv->dec_status == PICBIT_START_STATE) {
	handle->FrameCfgParam.prescanEnable = 0;
	//if (vbv_get_valid(pvbv_ctrl)) {
	if (1){
	#ifdef ADD_ON_CONF
		VPU_DecGiveCommand(handle, pvbv_ctrl, ENABLE_ROTATION, 0);
	#endif
	
		//SetDecodeFrameParam(&dec_handle->FrameCfgParam);
		//ppriv->end_time = hal_get_cpu_tick();
		//ppriv->start_time = hal_get_cpu_tick();
		//starttime = OsclTickCount::TickCount();
//		start = g_thread_gettime();
//	    tfstart = gst_util_get_timestamp ();
		VPU_DecStartOneFrame(handle, ppriv->instIdx);
		//pdec_inst->wait_finish = 1;
		ppriv->dec_status = PICBIT_CONTINUE_STATE;
	} 
	return;
  }

  
}

static void gst_boda_dec_stop(struct vdec_private *ppriv)
{
	DecInstCtl *handle=ppriv->inst_handle;  
	Frm_Ctrl *pfrm_ctrl=&ppriv->frm_ctrl;	
    Vbv_Ctrl *pvbv_ctrl = &ppriv->vbv_ctrl;
	guchar i;

	frmbuf_release(ppriv, pfrm_ctrl);

	for ( i = 0; i < pfrm_ctrl->max_dec_frame_num; i++ ) {
		frm_release(pfrm_ctrl, i);
	}
	
	VPU_DecClose(handle, ppriv->instIdx);

	vbv_release_data(pvbv_ctrl);	
	
	ppriv->dec_status = IDLE_STATE;

	bsp_close();
}

static void seq_init_isr(GstBodaDec *dec)
{
	struct vdec_private *ppriv;
	DecInstCtl *handle;  
	struct sync_block* block;
    Vbv_Ctrl *pvbv_ctrl;
	struct boda_sync_ctrl *sync_ctrl;
    Frm_Ctrl *pfrm_ctrl;
	CodecRetCode ret;
	guchar i;
	guint cur_readp;

    ppriv = (struct vdec_private *)dec->priv;
	handle = ppriv->inst_handle;
    pvbv_ctrl = &ppriv->vbv_ctrl;
	sync_ctrl = &ppriv->sync_ctrl;
    pfrm_ctrl = &ppriv->frm_ctrl;

	if ( ppriv->dec_status == WAITFOR_STOP_STATE) {
		ppriv->stop_status = VPU_STOP_STATE;
		return;
	}

	if  ( ppriv->dec_status != SEQ_CONTINUE_STATE){
		printf("SEQ IRQ ERR!\n");
		return;
	}

	
	ret = VPU_isr_seq_init(handle);

	cur_readp = GetNewReadPtr(handle, ppriv->instIdx);
//	cur_readp = GetNewNormalReadPtr(handle, ppriv->instIdx);
	g_mutex_lock(dec->buflock);
	vbv_update_readp(pvbv_ctrl, cur_readp);
	if (pvbv_ctrl->vbv_vo_status == 1){
		sem_post(&dec->push_sem);
		pvbv_ctrl->vbv_vo_status = 0;
	}	
	GST_LOG("seq init readp %x\n", (unsigned int)cur_readp); 
	if ( cur_readp == (pvbv_ctrl->start + 1)){
		cur_readp = pvbv_ctrl->start + dec->codec_data_size;
	}
	block = sync_search_addr(sync_ctrl, cur_readp);
	g_mutex_unlock(dec->buflock);
	if (block) {
		pfrm_ctrl->get_pts = block->pts;
		pfrm_ctrl->get_pts_valid = 1;
		GST_DEBUG("gst valid pts %lld\n", block->pts);				
	} else {
		pfrm_ctrl->get_pts = 0;
		pfrm_ctrl->get_pts_valid = 0;
		GST_DEBUG("gst no valid pts\n");				
	}
	//vdec_lakers_vbv_update_lbuf(pvbv, new_read_ptr);
	#if 0
	if ( vdec_lakers_vbv_check_readp(pvbv,  new_read_ptr)  == FALSE ) {
		printf("Seq Read Point %x error!\n", new_read_ptr);
		//VPU_DecBitBufferFlush(pdec->VPU_operation);
		vdec_lakers_vbv_check_validsize(pvbv, pvbv->writep, 0);
	}
	#endif

	if ( ret == RETCODE_SUCCESS) {
		guchar frate;
		printf("frameratediv %d, framerateres %d\n", handle->SeqInfo.frameRateDiv, handle->SeqInfo.frameRateRes);
		if ( handle->SeqInfo.frameRateRes && handle->SeqInfo.frameRateDiv ){
			dec->framerate_numerator = handle->SeqInfo.frameRateRes;
			dec->framerate_denominator = handle->SeqInfo.frameRateDiv;
			frate= 0;
		}else{
			if ( dec->framerate_numerator && dec->framerate_denominator ){
				handle->SeqInfo.frameRateRes = dec->framerate_numerator;
				handle->SeqInfo.frameRateDiv = dec->framerate_denominator;
				frate = 1;
			}
		}
		
		if (handle->SeqInfo.frameRateDiv ){
			if ((handle->BitstreamFormat == STD_AVC) && (frate == 0) ){
				sync_ctrl->frame_step = (handle->SeqInfo.frameRateDiv * 2 * 1e9) /  handle->SeqInfo.frameRateRes  ;
			}else{
				sync_ctrl->frame_step = (handle->SeqInfo.frameRateDiv * 1e9) / handle->SeqInfo.frameRateRes  ;
			}
		}
		printf("frame step %lld\n", sync_ctrl->frame_step);
		g_mutex_lock(dec->buflock);
		ppriv->dec_status = SET_FRAMEBUF_STATE;
		g_mutex_unlock(dec->buflock);
		//ppriv->dec_status = VDEC_LAKERS_PICFRM_START;
	}
	else{
		g_mutex_lock(dec->buflock);
		ppriv->dec_status = SEQ_START_STATE;
		g_mutex_unlock(dec->buflock);
	}


	return;
}


static void seq_end_isr(GstBodaDec *dec)
{
	struct vdec_private *ppriv =(struct vdec_private *)dec->priv;

	VPU_isr_seq_end(ppriv->inst_handle);
	return;
}


static void pic_run_isr(GstBodaDec *dec)
{
	struct vdec_private *ppriv =(struct vdec_private *)dec->priv;
	DecInstCtl *handle=ppriv->inst_handle;  
	DecFrameOutputInfo *FrmOutInfo=&handle->FrameOutInfo;	
	struct boda_sync_ctrl *sync_ctrl;
	struct sync_block* sync_block;
    Vbv_Ctrl *pvbv_ctrl;
    Frm_Ctrl *pfrm_ctrl;
	guint cur_readp;
	guint cur_pts;
	CodecRetCode ret;
	guchar ii, num;
	gpointer addr;

	g_mutex_lock(dec->buflock);
	if ( ppriv->dec_status == WAITFOR_STOP_STATE) {
		ppriv->stop_status = VPU_STOP_STATE;
		g_mutex_unlock(dec->buflock);
		GST_LOG("PIC run stop!\n");
		return;
	}

	if ( ppriv->dec_status != PICBIT_CONTINUE_STATE) {
		g_mutex_unlock(dec->buflock);
		GST_LOG("PIC RUN IRQ ERR!\n");
		return;
	}
	g_mutex_unlock(dec->buflock);

	ret = VPU_isr_pic_run(handle);
	if ( ret == RETCODE_FRAME_NOT_COMPLETE) {
		ppriv->dec_status = PICFRM_START_STATE;
		return;
	} 

    pvbv_ctrl = &ppriv->vbv_ctrl;
	sync_ctrl = &ppriv->sync_ctrl;
    pfrm_ctrl = &ppriv->frm_ctrl;	

	//vdec_lakers_vbv_update_lbuf(pvbv, new_read_ptr);
	#if 0
	g_mutex_lock (dec->buflock);
	StreamInBuf(dec);
    g_mutex_unlock (dec->buflock);
	#endif

	if ( FrmOutInfo->indexFrameDecoded >= 0 ){
		if ( pfrm_ctrl->get_pts_valid == 1 ){
			pfrm_ctrl->frm_info[FrmOutInfo->indexFrameDecoded].pts = pfrm_ctrl->get_pts;
			pfrm_ctrl->frm_info[FrmOutInfo->indexFrameDecoded].pts_valid = 1;
			//PRINTF("indx %d PTS %x\n", FrmOutInfo->indexFrameDecoded, pfrm_ctrl->get_pts);
		}else{
			pfrm_ctrl->frm_info[FrmOutInfo->indexFrameDecoded].pts_valid = 0;
			//PRINTF("PTS not valid\n");
		}		
		cur_readp = GetNewReadPtr(handle, ppriv->instIdx);
		if (cur_readp == pvbv_ctrl->end + 1){
			cur_readp = pvbv_ctrl->start;
		}	

		
//		printf("begin:s%x, e%x,n%d\n", sync_ctrl->start_addr, sync_ctrl->end_addr, sync_ctrl->cnt);
		//printf("readp: %x, writep: %x\n", pvbv_ctrl->readp, pvbv_ctrl->writep);
		sync_block = sync_search_addr(sync_ctrl, cur_readp);
		if (sync_block) {
			pfrm_ctrl->get_pts = sync_block->pts;
			pfrm_ctrl->get_pts_valid = 1;
			GST_DEBUG("gst valid pts %lld\n", sync_block->pts);				
		} else {
			pfrm_ctrl->get_pts = 0;
			pfrm_ctrl->get_pts_valid = 0;
			GST_DEBUG("gst no valid pts\n");				
		}
//		printf("get_pts %lld, %x, %d ", pfrm_ctrl->get_pts, pvbv_ctrl->readp, sync_ctrl->rd);
//		printf("s%x, e%x,n%d\n", sync_ctrl->start_addr, sync_ctrl->end_addr, sync_ctrl->cnt);
	}

	//cur_readp = GetNewReadPtr(handle, ppriv->instIdx);
	cur_readp = GetNewNormalReadPtr(handle, ppriv->instIdx);
	if ( FrmOutInfo->indexFrameDecoded >= 0 ){
		//printf("read pointer %x\n", GetNewReadPtr(handle, ppriv->instIdx));
	}
	addr = hal_paddr_to_vaddr(cur_readp);
	g_mutex_lock(dec->buflock);
	vbv_update_readp(pvbv_ctrl, cur_readp);
	if (pvbv_ctrl->vbv_vo_status == 1 ){
		sem_post(&dec->push_sem);
		pvbv_ctrl->vbv_vo_status = 0;
	}
	g_mutex_unlock(dec->buflock);
	GST_DEBUG("updata readp %x, writep %x, valid_size %x\n", (guint)pvbv_ctrl->readp, (guint)pvbv_ctrl->writep, (guint)pvbv_ctrl->valid_size);
	GST_DEBUG("decoded index %x, display index %x\n", (guint)FrmOutInfo->indexFrameDecoded, (guint)FrmOutInfo->indexFrameDisplay);

	#if 0
	if ( vdec_lakers_vbv_check_readp(pvbv,  new_read_ptr)  == FALSE ) {
		printf("Pic Read Point %x error!\n", new_read_ptr);
		//VPU_DecBitBufferFlush(pdec->VPU_operation);
		vdec_lakers_vbv_check_validsize(pvbv, pvbv->writep, 0);

	}
	#endif

	if ( handle->BitstreamFormat != STD_MJPG ){
#if 1
		if ( FrmOutInfo->indexFrameDecoded >= 0 ){
			guchar top_type, bot_type;
			top_type = FrmOutInfo->picType & 7;
			bot_type = ( FrmOutInfo->picType >> 3 ) & 7;
			if ( top_type == 2 || bot_type == 2){
				if (pfrm_ctrl->IPframe < 2){
					VPU_DecClrDispFlag( handle, ppriv->instIdx, (1<<FrmOutInfo->indexFrameDecoded) );
					ppriv->dec_status = PICFRM_START_STATE;
					return;
				}
			}else {
				if ( pfrm_ctrl->IPframe < 2 )
					pfrm_ctrl->IPframe++;
			}			
			
					
			}			
#endif		
		if ( FrmOutInfo->indexFrameDecoded == -1 ) {
			//printf("Not enough frame buffer!\n");
			ppriv->dec_status = PICFRM_START_STATE;
			pfrm_ctrl->last_dec_frame = 0;
			//vdec_lakers_decode_frame(ppriv);
			//return;
		} else {
			if ( FrmOutInfo->indexFrameDecoded != -2 ) {
				frm_update(pfrm_ctrl, FrmOutInfo->indexFrameDecoded, handle);
				frm_set_state(pfrm_ctrl, FrmOutInfo->indexFrameDecoded, 1);
				pfrm_ctrl->last_dec_frame = 1;
			}
		}

	  frm_num++;
//	  rlsend = gst_util_get_timestamp ();

 //     diff = GST_CLOCK_DIFF (rlsstart, rlsend);

//	  printf ("total Execution %d frame ended after %lld ns.\n", frm_num, diff);
		

		if ( FrmOutInfo->indexFrameDisplay == -1 ) {
			/* stream end, no decode, no display */
			g_mutex_lock(dec->buflock);
			ppriv->dec_status = IDLE_STATE;
			g_mutex_unlock(dec->buflock);
			//vdec_lakers_decode_frame(ppriv);
			return;
		}

		if ( FrmOutInfo->indexFrameDisplay >= 0   ){
			pfrm_ctrl->disp_flag = FrmOutInfo->indexFrameDisplay;
		}		


		if ( ppriv->decoded_first_frame == 0 ){
			/* output the first frame because of the async request */
			frame_output(dec, FrmOutInfo->indexFrameDecoded);
			pfrm_ctrl->frm_info[FrmOutInfo->indexFrameDecoded].display = 1;
			pfrm_ctrl->output_en = 1;
			pfrm_ctrl->disp_idx = FrmOutInfo->indexFrameDecoded;	
		}else{
			if ( FrmOutInfo->indexFrameDisplay != -2 && FrmOutInfo->indexFrameDisplay != -3 ) {		
				frame_output(dec, FrmOutInfo->indexFrameDisplay);
				pfrm_ctrl->frm_info[FrmOutInfo->indexFrameDisplay].display = 1;
				//printf("out %d\n", FrmOutInfo->indexFrameDisplay);
				pfrm_ctrl->output_en = 1;
				pfrm_ctrl->disp_idx = FrmOutInfo->indexFrameDisplay;	
			}
		}
		//output_idx = FrmOutInfo->indexFrameDisplay;
//		printf("output_idx %x\n", (unsigned int)output_idx);


		if ( ppriv->decoded_first_frame == 0) {
//				rlsstart = gst_util_get_timestamp ();
			if ( FrmOutInfo->indexFrameDecoded >= 0 ){
				ppriv->decoded_first_frame = 1;
			}
		}
		
		//printf("out %x, decoded %x\n", FrmOutInfo->indexFrameDisplay, FrmOutInfo->indexFrameDecoded);
		//printf("type %d\n", FrmOutInfo->picType);
	
	}else{
		frm_update(pfrm_ctrl, pfrm_ctrl->cur_idx, handle);
		frame_output(dec, pfrm_ctrl->cur_idx);
		pfrm_ctrl->frm_info[pfrm_ctrl->cur_idx].display = 1;
		pfrm_ctrl->disp_idx = pfrm_ctrl->cur_idx;
		pfrm_ctrl->output_en = 1;
		if ( ppriv->decoded_first_frame == 0) {
			ppriv->decoded_first_frame = 1;
		}	
		//output_idx = pfrm_ctrl->cur_idx;
	}

	g_mutex_lock(dec->buflock);
	ppriv->dec_status = PICFRM_START_STATE;
	g_mutex_unlock(dec->buflock);


	return;

}


static void set_framebuffer_isr(GstBodaDec *dec)
{
	struct vdec_private *ppriv=(struct vdec_private *)dec->priv;

	g_mutex_lock(dec->buflock);
	if ( ppriv->dec_status == WAITFOR_STOP_STATE) {
		ppriv->stop_status = VPU_STOP_STATE;
		g_mutex_unlock(dec->buflock);
		return;
	}

	VPURegInit(ppriv->inst_handle, 0x180, 32);
	ppriv->dec_status = PICFRM_START_STATE;

	g_mutex_unlock(dec->buflock);
	
	return;
}


static void bsbuffer_underflow_isr(GstBodaDec *dec)
{
	struct vdec_private *ppriv=(struct vdec_private *)dec->priv;
	DecInstCtl *handle=ppriv->inst_handle;  

	if ( ppriv->dec_status != SEQ_CONTINUE_STATE&& ppriv->dec_status != PICBIT_CONTINUE_STATE) {
		GST_ERROR("BUF EMPTY IRQ ERR %x !\n", ppriv->dec_status);
		return;
	}

    gst_boda_dec_finish(dec);

	return;

}


static void
gst_boda_dec_negotiate (GstBodaDec * dec, gint width, gint height)
{
  GstCaps *caps;
  struct vdec_private *ppriv ;
  Frm_Ctrl *pfrm_ctrl;
  DecInstCtl *handle;  
  DecSeqGetInfo *GetInfo;
  GstVideoFormat format;

  /* calculate or assume an average frame duration for QoS purposes */
  #if 0
  GST_OBJECT_LOCK (dec);
  if (dec->framerate_numerator != 0) {
    dec->qos_duration = gst_util_uint64_scale (GST_SECOND,
        dec->framerate_denominator, dec->framerate_numerator);
  } else {
    /* if not set just use 25fps */
    dec->qos_duration = gst_util_uint64_scale (GST_SECOND, 1, 25);
  }
  GST_OBJECT_UNLOCK (dec);
  #endif
#if 0
  {
    gint i;
    GstCaps *allowed_caps;

    GST_DEBUG_OBJECT (dec, "selecting RGB format");
    /* retrieve allowed caps, and find the first one that reasonably maps
     * to the parameters of the colourspace */
    caps = gst_pad_get_allowed_caps (dec->srcpad);
    if (!caps) {
      GST_DEBUG_OBJECT (dec, "... but no peer, using template caps");
      /* need to copy because get_allowed_caps returns a ref,
       * and get_pad_template_caps doesn't */
      caps = gst_caps_copy (gst_pad_get_pad_template_caps (dec->srcpad));
    }
    /* avoid lists of fourcc, etc */
    allowed_caps = gst_caps_normalize (caps);
    gst_caps_unref (caps);
    caps = NULL;
    GST_LOG_OBJECT (dec, "allowed source caps %" GST_PTR_FORMAT, allowed_caps);

	printf("allowed %d caps\n", gst_caps_get_size (allowed_caps));
    for (i = 0; i < gst_caps_get_size (allowed_caps); i++) {
      if (caps)
        gst_caps_unref (caps);
      caps = gst_caps_copy_nth (allowed_caps, i);
      /* sigh, ds and _parse_caps need fixed caps for parsing, fixate */
      gst_pad_fixate_caps (dec->srcpad, caps);
      GST_LOG_OBJECT (dec, "checking caps %" GST_PTR_FORMAT, caps);
      if (!gst_video_format_parse_caps (caps, &format, NULL, NULL))
        continue;
      /* we'll settle for the first (preferred) downstream rgb format */
      if (gst_video_format_is_yuv (format))
        break;
      /* default fall-back */
      format = GST_VIDEO_FORMAT_I420;
    }
    if (caps)
      gst_caps_unref (caps);
    gst_caps_unref (allowed_caps);
    caps = gst_video_format_new_caps (format, width, height,
        dec->framerate_numerator, dec->framerate_denominator, 1, 1);
    dec->outsize = gst_video_format_get_size (format, width, height);
    /* some format info */
    dec->offset[0] =
        gst_video_format_get_component_offset (format, 0, width, height);
    dec->offset[1] =
        gst_video_format_get_component_offset (format, 1, width, height);
    dec->offset[2] =
        gst_video_format_get_component_offset (format, 2, width, height);
    /* equal for all components */
    dec->stride = gst_video_format_get_row_stride (format, 0, width);
    dec->inc = gst_video_format_get_pixel_stride (format, 0);
  } 
#endif

  ppriv =(struct vdec_private *)dec->priv;
  pfrm_ctrl = &ppriv->frm_ctrl;
  handle=ppriv->inst_handle;  
  GetInfo = &handle->SeqInfo;

  if ( handle->BitstreamFormat != STD_MJPG ){
    format = GST_VIDEO_FORMAT_I420;
  }else{
    if ( GetInfo->mjpg_sourceFormat == YCBCR420 )
		format = GST_VIDEO_FORMAT_I420;
	else if ( GetInfo->mjpg_sourceFormat == YCBCR422H )
		format = GST_VIDEO_FORMAT_YVYU;
	else if ( GetInfo->mjpg_sourceFormat == YCBCR422V )
		format = GST_VIDEO_FORMAT_YVYU;
	else if ( GetInfo->mjpg_sourceFormat == YCBCR444 )
		format = GST_VIDEO_FORMAT_Y444;
	else if ( GetInfo->mjpg_sourceFormat == YCBCR400 )
		format = GST_VIDEO_FORMAT_Y800;
  }
  //caps = gst_pad_get_allowed_caps (dec->srcpad);
  //if (caps)
  //  gst_caps_unref (caps);
  //caps = NULL;
  //caps = gst_video_format_new_caps (format, width, height,
  //    dec->framerate_numerator, dec->framerate_denominator, 1, 1);
  #if 0
  dec->outsize = gst_video_format_get_size (format, width, height);
  dec->offset[0] =
      gst_video_format_get_component_offset (format, 0, width, height);
  dec->offset[1] =
      gst_video_format_get_component_offset (format, 1, width, height);
  dec->offset[2] =
      gst_video_format_get_component_offset (format, 2, width, height);
  dec->stride = gst_video_format_get_row_stride (format, 0, width);
  #endif
  dec->offset[0] = 0;
  dec->offset[1] = pfrm_ctrl->frm_info[0].size[0];
  dec->offset[2] = pfrm_ctrl->frm_info[0].size[0] + pfrm_ctrl->frm_info[0].size[1];
  dec->stride = GetInfo->stride;
  /* equal for all components */
  dec->inc = gst_video_format_get_pixel_stride (format, 0);

//  dec->framerate_numerator = 15;
//  dec->framerate_denominator = 1;

  if ( format == GST_VIDEO_FORMAT_I420 ){
    caps = gst_caps_new_simple ("video/x-raw-yuv",
        "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('I', '4', '2', '0'),
        "width", G_TYPE_INT, width, "height", G_TYPE_INT, height,
        "framerate", GST_TYPE_FRACTION, dec->framerate_numerator,
        dec->framerate_denominator, NULL);
  }else if ( format == GST_VIDEO_FORMAT_YVYU ){
    caps = gst_caps_new_simple ("video/x-raw-yuv",
        "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('Y', 'V', 'Y', 'U'),
        "width", G_TYPE_INT, width, "height", G_TYPE_INT, height,
        "framerate", GST_TYPE_FRACTION, dec->framerate_numerator,
        dec->framerate_denominator, NULL);  	
  }else if ( format == GST_VIDEO_FORMAT_Y444 ){
    caps = gst_caps_new_simple ("video/x-raw-yuv",
        "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('Y', '4', '4', '4'),
        "width", G_TYPE_INT, width, "height", G_TYPE_INT, height,
        "framerate", GST_TYPE_FRACTION, dec->framerate_numerator,
        dec->framerate_denominator, NULL);  	
  }else if ( format == GST_VIDEO_FORMAT_Y800 ){
    caps = gst_caps_new_simple ("video/x-raw-yuv",
        "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('Y', '8', '0', '0'),
        "width", G_TYPE_INT, width, "height", G_TYPE_INT, height,
        "framerate", GST_TYPE_FRACTION, dec->framerate_numerator,
        dec->framerate_denominator, NULL);  	
  }

  GST_DEBUG_OBJECT (dec, "setting caps %" GST_PTR_FORMAT, caps);

  gst_pad_set_caps (dec->srcpad, caps);
  gst_caps_unref (caps);

  dec->caps_width = width;
  dec->caps_height = height;
  dec->caps_framerate_numerator = dec->framerate_numerator;
  dec->caps_framerate_denominator = dec->framerate_denominator;
}


static gboolean gst_boda_dec_isr(GstBodaDec *dec)
{
  struct vdec_private *ppriv;
  DecInstCtl *handle;  
  Vbv_Ctrl *pvbv_ctrl;
  Frm_Ctrl *pfrm_ctrl;
  guint status;  
  
  ppriv = (struct vdec_private *)dec->priv;
  handle = ppriv->inst_handle;
  pvbv_ctrl = &ppriv->vbv_ctrl;
  pfrm_ctrl = &ppriv->frm_ctrl;

  //printf("another thread, %x\n", *(guint*)(0x5f400174));

  status = GetVPUIntStatus(handle);
	
  if (status == 0)
	return FALSE;

//  printf("dec isr status %x\n", status);
 //  end = g_thread_gettime();
  //  printf("loop time %x%x\n", (guint32)((end-start)>>32)&0xFFFFFFFF, (guint32)(end-start)&0xFFFFFFFF);

//      tfend = gst_util_get_timestamp ();

 //     diff = GST_CLOCK_DIFF (tfstart, tfend);

   //   printf ("Execution ended after %x%x ns.\n", (guint32)(diff>>32)&0xFFFFFFFF, (guint32)diff&0xffffffff);
//	  printf ("64bit Execution ended after %lld ns.\n", diff);



  if(status & VPU_ISR_SEQ_INIT) {	
	ClrVPUIntStatus(handle, status, VPU_ISR_SEQ_INIT);
	ClrVPUBitInt(handle);
	seq_init_isr(dec);
	if ( pvbv_ctrl->vbv_vo_status == 1 ){
		//vdec_lakers_bsbuffer_flush(ppriv);
		//return TRUE;
	}	
  }
  if(status & VPU_ISR_SEQ_END) {
	seq_end_isr(dec);
	ClrVPUIntStatus(handle, status, VPU_ISR_SEQ_END);
	ClrVPUBitInt(handle);
  }
  if(status & VPU_ISR_PIC_RUN) {
//	double usedtime;
	//usedtime = OsclTickCount::TicksToMsec(OsclTickCount::TickCount()) - OsclTickCount::TicksToMsec(starttime);
	//printf("dec time cycle %f msec\n", usedtime);
	//ppriv->start_time = hal_get_cpu_tick();
	ClrVPUIntStatus(handle, status, VPU_ISR_PIC_RUN);
	ClrVPUBitInt(handle);
	pic_run_isr(dec);	
	if ( pvbv_ctrl->vbv_vo_status == 1 ){
		//vdec_lakers_bsbuffer_flush(ppriv);
		//return TRUE;
	}		
  }
  if(status & VPU_ISR_SET_FRAME_BUFFER) {
	ClrVPUIntStatus(handle, status, VPU_ISR_SET_FRAME_BUFFER);
	ClrVPUBitInt(handle);
	set_framebuffer_isr(dec);
  }
  #if 0
  if (status & VPU_ISR_PARA_SET) {
	boda_ctrl->VPU_isr_para_set();
	boda_ctrl->ClrVPUIntStatus(dec_handle, status, VPU_ISR_PARA_SET);
  }
  #endif
  #if 1
  if (status & VPU_ISR_BUF_FLUSH) {
	VPU_isr_flush_buffer();
	pvbv_ctrl->readp = pvbv_ctrl->start;
	pvbv_ctrl->valid_size = 0;
	//vdec_lakers_sync_reset (sync_ctrl);

	ClrVPUIntStatus(handle, status, VPU_ISR_BUF_FLUSH);
	ClrVPUBitInt(handle);

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
  if (status & VPU_ISR_BUF_EMPTY) {
	ClrVPUIntStatus(handle, status, VPU_ISR_BUF_EMPTY);
	ClrVPUBitInt(handle);
	bsbuffer_underflow_isr(dec);
  }

  return TRUE;
  

}

static gboolean gst_boda_dec_status(DecInstCtl *dec_handle)
{
	if (VPU_IsBusy(dec_handle))
		return FALSE;

	/* boda is not busy */
	return TRUE;
}

static void gst_boda_dec_loop(void *param)
{
  GstBodaDec *dec = (GstBodaDec *)param;
  struct vdec_private *ppriv;
  DecInstCtl *handle;  
  Vbv_Ctrl *pvbv_ctrl;
  Frm_Ctrl *pfrm_ctrl;
/*
   end = g_thread_gettime();
  printf("loop time %x%x\n", (guint32)((end-start)>>32)&0xFFFFFFFF, (guint32)(end-start)&0xFFFFFFFF);
  */
  //dec = GST_BODA_DEC (GST_OBJECT_PARENT (pad));

  	{
  //printf("dec loop start\n");
  #if 1
  	{
      //loopend = gst_util_get_timestamp ();

      //diff = GST_CLOCK_DIFF (loopstart, loopend);

	  //printf ("loop  %lld ns.\n", diff);

	  
        //diff = GST_CLOCK_DIFF (loopstart1, loopend);

	 // printf ("loop111  %lld ns.\n", diff);

  ppriv = (struct vdec_private *)dec->priv;
  handle = ppriv->inst_handle;
  pvbv_ctrl = &ppriv->vbv_ctrl;
  pfrm_ctrl = &ppriv->frm_ctrl;

 // printf("dec loop 1\n");

	//loopstart = gst_util_get_timestamp ();


  if (gst_boda_dec_status(handle) == TRUE )
  {
	gst_boda_dec_isr(dec);
  }

  g_mutex_lock(dec->buflock);
  gst_boda_dec_frame(dec);
  g_mutex_unlock(dec->buflock);

//  start = g_thread_gettime();

  //g_usleep(1000);
  // end = g_thread_gettime();
//  printf("loop time %x%x\n", (guint32)((end-start)>>32)&0xFFFFFFFF, (guint32)(end-start)&0xFFFFFFFF);
 // printf("loop finish\n");
  	}
  #endif
}

}

static gboolean gst_boda_dec_finish(GstBodaDec *dec)
{
	struct vdec_private * ppriv = (struct vdec_private *)dec->priv;
    DecInstCtl * handle = ppriv->inst_handle;
    Vbv_Ctrl * pvbv = &ppriv->vbv_ctrl;	
	struct boda_sync_ctrl*	sync_ctrl = &ppriv->sync_ctrl;
	struct boda_sync_block *sync_block, *next_sync_block;

	if ( ppriv->dec_status == PICBIT_CONTINUE_STATE || ppriv->dec_status == SEQ_CONTINUE_STATE || ppriv->dec_status == SET_FRAMEBUF_CONTINUE_STATE ) {
		g_mutex_lock(dec->buflock);
		ppriv->stop_status = WAITFOR_STOP_STATE;
		if (ppriv->dec_status == PICBIT_CONTINUE_STATE){
			ppriv->dec_status = WAITFOR_STOP_STATE;
			VPU_DecUpdateBitstreamBuffer(handle, ppriv->instIdx, pvbv->writep, 0);   
		}
		else if ( ppriv->dec_status == SEQ_CONTINUE_STATE ){
			ppriv->dec_status = WAITFOR_STOP_STATE;
			VPU_DecSetEscSeqInit(handle, 1);
		}else if ( ppriv->dec_status == SET_FRAMEBUF_CONTINUE_STATE ){
			ppriv->dec_status = WAITFOR_STOP_STATE;
		}
		g_mutex_unlock(dec->buflock);
		while (ppriv->dec_status== WAITFOR_STOP_STATE){
			ppriv->dec_status = ppriv->stop_status;
			g_usleep(1000);
		}		
		
	}

	g_mutex_lock(dec->buflock);
	ppriv->dec_status = VPU_STOP_STATE;
	g_mutex_unlock(dec->buflock);

	if ( handle->BitstreamFormat == STD_AVC){
		struct H264BSContext *ctx = dec->priv_data;

		if (ctx->sps_pps_data)
			g_free(ctx->sps_pps_data);
	}
	
	gst_boda_dec_stop(ppriv);	

	sync_block = sync_ctrl->sync_block_list;
	while(sync_block){
		next_sync_block = sync_block->next;
		g_free(sync_block);
		sync_block = next_sync_block;
	}
	

	
#ifdef DATA_DEBUG	
	fclose(fp);
#endif
	sem_destroy(&dec->push_sem);
	
}


static gboolean
gst_boda_dec_src_query (GstPad * pad, GstQuery * query)
{
  gboolean res = TRUE;
  GstPad *peer;
  GstBodaDec *dec;

  dec = GST_BODA_DEC (GST_PAD_PARENT (pad));

  peer = gst_pad_get_peer (dec->sinkpad);

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_FORMATS:
      gst_query_set_formats (query, 3, GST_FORMAT_DEFAULT, GST_FORMAT_TIME,
          GST_FORMAT_BYTES);
      break;
    case GST_QUERY_POSITION:
    {
      GstFormat format;
      gint64 cur;

      /* save requested format */
      gst_query_parse_position (query, &format, NULL);

      /* try any demuxer before us first */
      if (format == GST_FORMAT_TIME && peer && gst_pad_query (peer, query)) {
        gst_query_parse_position (query, NULL, &cur);
        GST_LOG_OBJECT (dec, "peer returned position %" GST_TIME_FORMAT,
            GST_TIME_ARGS (cur));
        break;
      }

      /* and convert to the requested format */
	  #if 0
      if (format != GST_FORMAT_DEFAULT) {
        if (!gst_mad_convert_src (pad, GST_FORMAT_DEFAULT, mad->total_samples,
                &format, &cur))
          goto error;
      } else {
        cur = mad->total_samples;
      }
	  #endif
      gst_query_set_position (query, format, cur);

      if (format == GST_FORMAT_TIME) {
        GST_LOG ("position=%" GST_TIME_FORMAT, GST_TIME_ARGS (cur));
      } else {
        GST_LOG ("position=%" G_GINT64_FORMAT ", format=%u", cur, format);
      }
      break;
    }
    case GST_QUERY_DURATION:
    {
      GstFormat bytes_format = GST_FORMAT_BYTES;
      GstFormat time_format = GST_FORMAT_TIME;
      GstFormat req_format;
      gint64 total, total_bytes;

      /* save requested format */
      gst_query_parse_duration (query, &req_format, NULL);

      if (peer == NULL)
        goto error;

      /* try any demuxer before us first */
      if (req_format == GST_FORMAT_TIME && gst_pad_query (peer, query)) {
        gst_query_parse_duration (query, NULL, &total);
        GST_LOG_OBJECT (dec, "peer returned duration %" GST_TIME_FORMAT,
            GST_TIME_ARGS (total));
        break;
      }

      /* query peer for total length in bytes */
      if (!gst_pad_query_peer_duration (dec->sinkpad, &bytes_format,
              &total_bytes) || total_bytes <= 0) {
        GST_LOG_OBJECT (dec, "duration query on peer pad failed");
        goto error;
      }

      GST_LOG_OBJECT (dec, "peer pad returned total=%" G_GINT64_FORMAT
          " bytes", total_bytes);
#if 0
      if (!gst_mad_convert_sink (pad, GST_FORMAT_BYTES, total_bytes,
              &time_format, &total)) {
        GST_DEBUG_OBJECT (mad, "conversion BYTE => TIME failed");
        goto error;
      }

      if (!gst_mad_convert_src (pad, GST_FORMAT_TIME, total,
              &req_format, &total)) {
        GST_DEBUG_OBJECT (mad, "conversion TIME => %s failed",
            gst_format_get_name (req_format));
        goto error;
      }
#endif
      gst_query_set_duration (query, req_format, total);

      if (req_format == GST_FORMAT_TIME) {
        GST_LOG_OBJECT (dec, "duration=%" GST_TIME_FORMAT,
            GST_TIME_ARGS (total));
      } else {
        GST_LOG_OBJECT (dec, "duration=%" G_GINT64_FORMAT " (%s)",
            total, gst_format_get_name (req_format));
      }
      break;
    }
    case GST_QUERY_CONVERT:
    {
      GstFormat src_fmt, dest_fmt;
      gint64 src_val, dest_val;

      gst_query_parse_convert (query, &src_fmt, &src_val, &dest_fmt, &dest_val);
	  #if 0
      if (!(res =
              gst_mad_convert_src (pad, src_fmt, src_val, &dest_fmt,
                  &dest_val)))
        goto error;
	  #endif
      gst_query_set_convert (query, src_fmt, src_val, dest_fmt, dest_val);
      break;
    }
    default:
      res = gst_pad_query_default (pad, query);
      break;
  }

  if (peer)
    gst_object_unref (peer);

  return res;

error:

  GST_DEBUG ("error handling query");

  if (peer)
    gst_object_unref (peer);

  return FALSE;
}


static void
gst_boda_dec_init (GstBodaDec * dec)
{
  GstBodaDecClass *decclass;

  decclass = (GstBodaDecClass *) (G_OBJECT_GET_CLASS (dec));

  GST_DEBUG ("initializing");

  /* create the sink and src pads */
  dec->sinkpad = gst_pad_new_from_template (decclass->sinktempl,"sink");
  gst_element_add_pad (GST_ELEMENT (dec), dec->sinkpad);
  gst_pad_set_setcaps_function (dec->sinkpad, GST_DEBUG_FUNCPTR (gst_boda_dec_setcaps));
  gst_pad_set_chain_function (dec->sinkpad, GST_DEBUG_FUNCPTR (gst_boda_dec_chain));
  gst_pad_set_event_function (dec->sinkpad, GST_DEBUG_FUNCPTR (gst_boda_dec_sink_event));

  dec->srcpad = gst_pad_new_from_static_template (&src_template, "src");
  gst_pad_set_event_function (dec->srcpad, GST_DEBUG_FUNCPTR (gst_boda_dec_src_event));
  gst_pad_use_fixed_caps (dec->srcpad);
  gst_pad_set_query_function (dec->srcpad, GST_DEBUG_FUNCPTR (gst_boda_dec_src_query));  
  gst_element_add_pad (GST_ELEMENT (dec), dec->srcpad);

  
  dec->buflock = g_mutex_new ();
  
  /* setup boda decoder lib */
  VideoDecoder_Init(dec);

  dec->fd = hal_device_attach("vdec_boda");

  hal_int_register(dec->fd, gst_boda_dec_loop, (void *)dec);
  
  dec->task = NULL;
  dec->dec_lock= g_new (GStaticRecMutex, 1);
  g_static_rec_mutex_init (dec->dec_lock);

  sem_init(&dec->push_sem, 0, 0); 

  dec->codec_data = NULL;
  dec->codec_data_size = 0;
  dec->priv_data = NULL;
  
#ifdef DATA_DEBUG
  fp = g_fopen("data.bin", "wb");
#endif

  GST_DEBUG("decode init finish\n");
  
    //dec->adapter = gst_adapter_new ();

}

static gboolean
gst_boda_dec_setcaps (GstPad * pad, GstCaps * caps)
{
  GstStructure *s;
  GstBodaDec *dec;
  const GValue *framerate;
  struct vdec_private *ppriv ;
  DecInstCtl *handle;  

  dec = GST_BODA_DEC (GST_OBJECT_PARENT (pad));
  s = gst_caps_get_structure (caps, 0);

  if ((framerate = gst_structure_get_value (s, "framerate")) != NULL) {
    dec->framerate_numerator = gst_value_get_fraction_numerator (framerate);
    dec->framerate_denominator = gst_value_get_fraction_denominator (framerate);
    dec->packetized = TRUE;
    printf ("got framerate of %d/%d fps => packetized mode\n",
        dec->framerate_numerator, dec->framerate_denominator);
  }else{
  	printf("framerate is NULL\n");
  }

  gst_boda_caps_to_codecid(dec, caps);

  gst_structure_get_int(s, "width", &dec->caps_width);
  gst_structure_get_int(s, "height", &dec->caps_height);
  printf("dec width%d, height %d\n", dec->caps_width, dec->caps_height);

  /*
  "interlaced"
  "pixel-aspect-ratio"
  */
  /* But we can take the framerate values and set them on the src pad */

  return TRUE;
}

static GstFlowReturn
gst_boda_dec_chain (GstPad * pad, GstBuffer * buf)
{
//  GstFlowReturn ret = GST_FLOW_OK;
  GstBodaDec *dec;
//  struct vdec_private *ppriv;
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
  GstFlowReturn res;

  dec = GST_BODA_DEC (GST_PAD_PARENT (pad));
//  ppriv = (struct vdec_private *)dec->priv;
//  handle = ppriv->inst_handle;

//  printf("chain buf %x, int %x\n", (guint)buf, *(guint *)(0x5f400174));
  g_mutex_lock (dec->buflock);
  VideoStreamInput(dec, buf);
  g_mutex_unlock (dec->buflock);
  
  //gst_pad_start_task(dec->sinkpad, (GstTaskFunction)gst_boda_dec_loop, dec->sinkpad);
  //gst_boda_dec_loop(dec);

//  printf("chain finish\n");
 // frame_free(dec);
  
  return GST_FLOW_OK;  


}

static gboolean
gst_boda_dec_src_event (GstPad * pad, GstEvent * event)
{
  GstBodaDec *dec;
  gboolean res;

  //printf("src event %s\n", GST_EVENT_TYPE_NAME (event));

  dec = GST_BODA_DEC (gst_pad_get_parent (pad));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_QOS:{
      GstClockTimeDiff diff;
      GstClockTime timestamp;
      gdouble proportion;

      gst_event_parse_qos (event, &proportion, &diff, &timestamp);
	  //printf("Updating QoS: proportion %lf, diff %s%" GST_TIME_FORMAT ", timestamp %"
      //GST_TIME_FORMAT, proportion, (diff < 0) ? "-" : "",
      //GST_TIME_ARGS (ABS (diff)), GST_TIME_ARGS (timestamp));
//      gst_boda_dec_update_qos (dec, proportion, diff, timestamp);
//	  frm_output_dequeue(dec);
      break;
    }
    default:
      break;
  }

  res = gst_pad_push_event (dec->sinkpad, event);

  gst_object_unref (dec);
  return res;
}


static gboolean
gst_boda_dec_sink_event (GstPad * pad, GstEvent * event)
{
  gboolean ret = TRUE;
  GstBodaDec *dec = GST_BODA_DEC (GST_OBJECT_PARENT (pad));
  struct vdec_private *ppriv = (struct vdec_private *)dec->priv;  
  DecInstCtl *handle;  
  Vbv_Ctrl *pvbv;  
  guint intmask;

  GST_DEBUG_OBJECT (dec, "event : %s", GST_EVENT_TYPE_NAME (event));
	printf("sink event %s\n", GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_FLUSH_STOP:
      gst_segment_init (&dec->segment, GST_FORMAT_UNDEFINED);
      //gst_adapter_clear (dec->adapter);
      g_free (dec->cur_buf);
      dec->cur_buf = NULL;
      dec->parse_offset = 0;
      dec->parse_entropy_len = 0;
      dec->parse_resync = FALSE;
//      gst_boda_dec_reset_qos (dec);
      break;
    case GST_EVENT_NEWSEGMENT:
		#if 0
      gboolean update;
      gdouble rate, applied_rate;
      GstFormat format;
      gint64 start, stop, position;

      gst_event_parse_new_segment_full (event, &update, &rate, &applied_rate,
          &format, &start, &stop, &position);

      GST_DEBUG_OBJECT (dec, "Got NEWSEGMENT [%" GST_TIME_FORMAT
          " - %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "]",
          GST_TIME_ARGS (start), GST_TIME_ARGS (stop),
          GST_TIME_ARGS (position));

      gst_segment_set_newsegment_full (&dec->segment, update, rate,
          applied_rate, format, start, stop, position);
        #endif
      break;
	 case GST_EVENT_EOS:
	  handle = ppriv->inst_handle;
	  ppriv->wait_finish = 1;
	  #if 1
	  gst_boda_dec_finish(dec);
	  #else
	  //enable the underflow interrupt, decode the frame until the stream finish
	  intmask = GetVPUIntMask(handle);
	  SetVPUIntMask(handle, (intmask|VPU_ISR_BUF_EMPTY));
	  #endif
	  break;    
    default:
      break;
  }

  ret = gst_pad_push_event (dec->srcpad, event);

  return ret;
}

static gboolean gst_boda_thread(GstBodaDec *dec)
{
  GThread *thread;
  GError *error = NULL;

  thread =
     g_thread_create ((GThreadFunc) gst_boda_dec_loop, dec, TRUE, &error);
  if (error != NULL) {
    g_warning ("could not create thread reason: %s", error->message);
    return NULL;
  }
  //g_thread_join (thread);

}

static GstStateChangeReturn
gst_boda_dec_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret;
  GstBodaDec *dec;

  dec = GST_BODA_DEC (element);
  

  switch (transition) {
  	case GST_STATE_CHANGE_NULL_TO_READY:
	  break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      dec->framerate_numerator = 0;
      dec->framerate_denominator = 1;
      dec->caps_framerate_numerator = dec->caps_framerate_denominator = 0;
      dec->caps_width = -1;
      dec->caps_height = -1;
      dec->clrspc = -1;
      dec->packetized = FALSE;
      dec->next_ts = 0;
      dec->discont = TRUE;
      dec->parse_offset = 0;
      dec->parse_entropy_len = 0;
      dec->parse_resync = FALSE;
      dec->cur_buf = NULL;
      gst_segment_init (&dec->segment, GST_FORMAT_UNDEFINED);
//      gst_boda_dec_reset_qos (dec);
#if 0
	  if ( dec->task == NULL ){
	    dec->task = gst_task_create ((GstTaskFunction) gst_boda_dec_loop, dec);
		gst_task_set_priority (dec->task, G_THREAD_PRIORITY_URGENT);
	    gst_task_set_lock (dec->task, GST_BODA_GET_LOCK(dec));
	  }
	 #endif
	  //gst_task_start (dec->task);
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
      //gst_adapter_clear (dec->adapter);
      g_free (dec->cur_buf);
      dec->cur_buf = NULL;
      //gst_boda_dec_free_buffers (dec);
	  if ( dec->task ){
	  	gst_task_stop (dec->task);
		gst_object_unref (GST_OBJECT (dec->task));
    	dec->task = NULL;		
	  }
      break;
	case GST_STATE_CHANGE_READY_TO_NULL:
      break;
    default:
      break;
  }

  return ret;
}


static gboolean
plugin_init (GstPlugin * plugin)
{

  if (!gst_element_register (plugin, "bodadec", GST_RANK_PRIMARY,
          GST_TYPE_BODA_DEC))
    return FALSE;

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "boda",
    "Boda plugin library",
    plugin_init, VERSION, "LGPL", "Gstreamer boda Plug-ins source release", "Boda package origin")

