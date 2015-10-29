
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <gst/gst.h>

#include "gstcodadec.h"
#include "gstcodaenc.h"


static gboolean
plugin_init (GstPlugin * plugin)
{

  if (!gst_element_register (plugin, "codaenc", GST_RANK_PRIMARY,
          GST_TYPE_CODA_ENC))
    return FALSE;

  if (!gst_element_register (plugin, "codadec", GST_RANK_PRIMARY,
          GST_TYPE_CODA_DEC))
    return FALSE;


  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "coda",
    "coda plugin library",
    plugin_init, VERSION, "LGPL", "Gstreamer coda Plug-ins source release", "Coda package origin")


