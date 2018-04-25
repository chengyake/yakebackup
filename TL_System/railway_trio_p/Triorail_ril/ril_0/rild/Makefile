TOPDIR= ../../..
APPDIR= ./..

TARGET=rild-0  

CFLAGS= -I$(APPDIR)/include  -L$(LIBPATH)  -DRIL_SHLIB
LDFLAGS= -L$(LIBPATH) -lril  -ltrio-ril -lrilutils  -lpthread -ldl -lrt -lstdc++


include $(TOPDIR)/Makefile.env
