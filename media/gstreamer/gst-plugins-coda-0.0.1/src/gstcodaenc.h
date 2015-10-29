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


#ifndef __GST_CODA_ENC_H__
#define __GST_CODA_ENC_H__

#include <glib-object.h>
#include <semaphore.h> 

G_BEGIN_DECLS

#define GST_TYPE_CODA_ENC \
  (gst_coda_enc_get_type())
#define GST_CODA_ENC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_CODA_ENC,GstCodaEnc))
#define GST_CODA_ENC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_CODA_ENC,GstCodaEncClass))
#define GST_IS_CODA_ENC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_CODA_ENC))
#define GST_IS_CODA_ENC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_CODA_ENC))
#define GST_CODAENC_CAST(obj) \
  ((GstCodaEnc *)(obj))  

#define GST_CODA_ENC_GET_LOCK(enc)   (GST_CODAENC_CAST(enc)->enc_lock)
#define GST_CODA_ENC_LOCK(enc)       (g_static_rec_mutex_lock (GST_CODA_ENC_GET_LOCK(enc)))
#define GST_CODA_ENC_UNLOCK(enc)     (g_static_rec_mutex_unlock (GST_CODA_ENC_GET_LOCK(enc)))


typedef struct _GstCodaEnc           GstCodaEnc;
typedef struct _GstCodaEncClass      GstCodaEncClass;

struct _GstCodaEnc {
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
  GStaticRecMutex *enc_lock;

  sem_t push_sem;

  guint   codec_data_size;
  guchar* codec_data;

  /* negotiated state */
  gint     caps_framerate_numerator;
  gint     caps_framerate_denominator;
  gint     caps_width;
  gint     caps_height;
  gint     insize;
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

struct _GstCodaEncClass {
  GstElementClass  parent_class;
  GstPadTemplate *srctempl, *sinktempl;
};

GType gst_coda_enc_get_type(void);

G_END_DECLS

#endif /* __GST_CODA_ENC_H__ */


