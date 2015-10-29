plugin_LTLIBRARIES = libgstcoda.la

libgstcoda_la_SOURCES = gstcoda.c gstcodadec.c gstcodaenc.c coda_vpu.c hal_dev.c

noinst_HEADERS = gstcodadec.h gstcodaenc.h blackbird.h coda_dec.h coda_enc.h coda_reg.h coda_vpu.h hal_arch.h VpuJpegTable.h

libgstcoda_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) $(GST_BASE_CFLAGS) $(GST_CFLAGS)
libgstcoda_la_LIBADD = $(GST_PLUGINS_BASE_LIBS) $(GST_BASE_LIBS) $(GST_LIBS) -lgstvideo-0.10 \
                        -lgsttag-0.10
libgstcoda_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstcoda_la_LIBTOOLFLAGS = --tag=disable-static

