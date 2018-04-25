TOPDIR= ../../..
APPDIR= ./..

#SHAREDLIB=libtrio-ril.so
LIB=libtrio-ril_1.a
 
CFLAGS= -I$(APPDIR)/include -I. -DRIL_SHLIB -D_GNU_SOURCE
LDFLAGS= -L$(LIBPATH) -lril_1  -lrilutils_1 -lpthread -lrt -lstdc++

include $(TOPDIR)/Makefile.env
