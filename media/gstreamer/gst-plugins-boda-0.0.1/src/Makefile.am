plugin_LTLIBRARIES = libgstboda.la

libgstboda_la_SOURCES = gstboda.c boda_vpu.c hal_dev.c

noinst_HEADERS = gstboda.h BODA7503.h boda_dec.h boda_reg.h boda_vpu.h hal_arch.h 

libgstboda_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) $(GST_BASE_CFLAGS) $(GST_CFLAGS)
libgstboda_la_LIBADD = $(GST_PLUGINS_BASE_LIBS) $(GST_BASE_LIBS) $(GST_LIBS) -lgstvideo-0.10 \
                        -lgsttag-0.10
libgstboda_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstboda_la_LIBTOOLFLAGS = --tag=disable-static

