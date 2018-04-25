TOPDIR= ../../..
APPDIR= ./..

#SHAREDLIB=libtrio-ril.so
LIB=libtrio-ril.a
 
CFLAGS= -I$(APPDIR)/include -I. -DRIL_SHLIB -D_GNU_SOURCE
LDFLAGS= -L$(LIBPATH) -lril  -lrilutils -lpthread -lrt -lstdc++

include $(TOPDIR)/Makefile.env
