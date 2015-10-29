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


#ifndef __GST_BODA_H__
#define __GST_BODA_H__

#include <glib-object.h>
#include <semaphore.h> 

G_BEGIN_DECLS

#define GST_TYPE_BODA_DEC \
  (gst_boda_dec_get_type())
#define GST_BODA_DEC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_BODA_DEC,GstBodaDec))
#define GST_BODA_DEC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_BODA_DEC,GstBodaDecClass))
#define GST_IS_BODA_DEC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_BODA_DEC))
#define GST_IS_BODA_DEC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_BODA_DEC))
#define GST_BODADEC_CAST(obj) \
  ((GstBodaDec *)(obj))  

#define GST_BODA_GET_LOCK(dec)   (GST_BODADEC_CAST(dec)->dec_lock)
#define GST_BODA_LOCK(dec)       (g_static_rec_mutex_lock (GST_BODA_GET_LOCK(dec)))
#define GST_BODAUNLOCK(dec)      (g_static_rec_mutex_unlock (GST_BODA_GET_LOCK(dec)))


typedef struct _GstBodaDec           GstBodaDec;
typedef struct _GstBodaDecClass      GstBodaDecClass;

struct _GstBodaDec {
  GstElement element;

  /* pads */
  GstPad  *sinkpad;
  GstPad  *srcpad;

  //GstAdapter *adapter;

  guint8     *cur_buf;

  /* TRUE if each input buffer contains a whole jpeg image */
  gboolean packetized;

  /* the (expected) timestamp of the next frame */
  guint64  next_ts;

  GstSegment segment;

  /* TRUE if the next output buffer should have the DISCONT flag set */
  gboolean discont;

  /* QoS stuff *//* with LOCK */
  gdouble proportion;
  GstClockTime earliest_time;
  GstClockTime qos_duration;

  /* video state */
  gint framerate_numerator;
  gint framerate_denominator;

  GMutex *buflock;
  
  GstTask  *task;
  GStaticRecMutex *dec_lock;

  sem_t push_sem;

  guint   codec_data_size;
  guchar* codec_data;

  /* negotiated state */
  gint     caps_framerate_numerator;
  gint     caps_framerate_denominator;
  gint     caps_width;
  gint     caps_height;
  gint     outsize;
  gint     clrspc;

  gint     offset[3];
  gint     stride;
  gint     inc;

  /* parse state */
  gint     parse_offset;
  gint     parse_entropy_len;
  gint     parse_resync;

  /* properties */
  gpointer priv;
  int      fd;
  void *priv_data;

  /* arrays for indirect decoding */
  gboolean idr_width_allocated;
  guchar *idr_y[16],*idr_u[16],*idr_v[16];
  /* current (parsed) image size */
  guint    rem_img_len;
};

struct _GstBodaDecClass {
  GstElementClass  parent_class;
  GstPadTemplate *srctempl, *sinktempl;
};

GType gst_boda_dec_get_type(void);

G_END_DECLS

#endif /* __GST_BODA_H__ */

