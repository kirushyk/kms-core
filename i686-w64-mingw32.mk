CC=i686-w64-mingw32-gcc
CXX=i686-w64-mingw32-g++
TARGET_DIR=./build/

CXXFLAGS=--std=gnu++17 -fpermissive

CFLAGS= \
-I/usr/i686-w64-mingw32/sys-root/mingw/include/gstreamer-1.0 \
-I/usr/i686-w64-mingw32/sys-root/mingw/lib/gstreamer-1.0/include \
-I/usr/i686-w64-mingw32/sys-root/mingw/include/glib-2.0 \
-I/usr/i686-w64-mingw32/sys-root/mingw/lib/glib-2.0/include \
-I./win32

SDPAGENT_TARGET=libkmssdpagent.dll

SDPAGENT_SRC= \
./src/gst-plugins/commons/sdpagent/kmssdpmidext.c \
./src/gst-plugins/commons/sdpagent/kmssdpconnectionext.c \
./src/gst-plugins/commons/sdpagent/kmssdpgroupmanager.c \
./src/gst-plugins/commons/sdpagent/kmssdpmediahandler.c \
./src/gst-plugins/commons/sdpagent/kmssdpagent.c \
./src/gst-plugins/commons/sdpagent/kmssdpcontext.c \
./src/gst-plugins/commons/sdpagent/kmsisdpmediaextension.c \
./src/gst-plugins/commons/sdpagent/kmssdpagentcommon.c \
./src/gst-plugins/commons/sdpagent/kmssdpbundlegroup.c \
./src/gst-plugins/commons/sdpagent/kmssdprtpsavpmediahandler.c \
./src/gst-plugins/commons/sdpagent/kmssdprtpmediahandler.c \
./src/gst-plugins/commons/sdpagent/kmssdpbasegroup.c \
./src/gst-plugins/commons/sdpagent/kmssdprtpsavpfmediahandler.c \
./src/gst-plugins/commons/sdpagent/kmsisdpsessionextension.c \
./src/gst-plugins/commons/sdpagent/kmssdprtpavpfmediahandler.c \
./src/gst-plugins/commons/sdpagent/kmssdppayloadmanager.c \
./src/gst-plugins/commons/sdpagent/kmssdpsctpmediahandler.c \
./src/gst-plugins/commons/sdpagent/kmssdprtpavpmediahandler.c \
./src/gst-plugins/commons/sdpagent/kmssdpsdesext.c \
./src/gst-plugins/commons/sdpagent/kmsisdppayloadmanager.c \
./src/gst-plugins/commons/sdpagent/kmssdprejectmediahandler.c \
./src/gst-plugins/commons/kmsrefstruct.c \
./src/gst-plugins/commons/sdp_utils.c \
./src/gst-plugins/commons/kmsutils.c \
./win32/kms-sdp-agent-marshal.c

SDPAGENT_LIBS= \
-L/usr/i686-w64-mingw32/sys-root/mingw/lib \
-L/usr/lib/gcc/i686-w64-mingw32/5.2.0 \
-L/usr/i686-w64-mingw32/lib/ \
-lgstvideo-1.0.dll \
-lgstsdp-1.0.dll \
-lgstreamer-1.0 \
-lgobject-2.0 \
-lrpcrt4 \
-lole32 \
-lglib-2.0

SDPAGENT_OBJS=$(SDPAGENT_SRC:.c=.o)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

all: $(TARGET_DIR)/$(SDPAGENT_TARGET)

$(TARGET_DIR)/$(SDPAGENT_TARGET): $(SDPAGENT_OBJS)
	mkdir -p $(TARGET_DIR)
	$(CC) -shared -o $(TARGET_DIR)/$(SDPAGENT_TARGET) $(CFLAGS) $(SDPAGENT_OBJS) $(SDPAGENT_LIBS) -Wl,--out-implib,$(TARGET_DIR)/$(SDPAGENT_TARGET).a

.PHONY: clean
clean:
	rm $(TARGET_DIR)/$(SDPAGENT_TARGET)
	rm $(TARGET_DIR)/$(SDPAGENT_TARGET).a

