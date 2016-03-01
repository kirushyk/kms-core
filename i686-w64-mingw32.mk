CC=i686-w64-mingw32-gcc
CXX=i686-w64-mingw32-g++
TARGET_DIR=./build/

CXXFLAGS=--std=gnu++17 -fpermissive

CFLAGS= \
-I/usr/i686-w64-mingw32/sys-root/mingw/include/gstreamer-1.0 \
-I/usr/i686-w64-mingw32/sys-root/mingw/lib/gstreamer-1.0/include \
-I/usr/i686-w64-mingw32/sys-root/mingw/include/glib-2.0 \
-I/usr/i686-w64-mingw32/sys-root/mingw/lib/glib-2.0/include \
-I./src/gst-plugins/commons/ \
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
\
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
-lglib-2.0 \
-lrpcrt4 \
-lole32

KMSCOMMONS_TARGET=libkmsgstcommons.dll

KMSCOMMONS_SRC= \
./src/gst-plugins/commons/kmsrtcp.c \
./src/gst-plugins/commons/kmsremb.c \
./src/gst-plugins/commons/kmssdpsession.c \
./src/gst-plugins/commons/kmsbasertpsession.c \
./src/gst-plugins/commons/kmsirtpsessionmanager.c \
./src/gst-plugins/commons/kmsirtpconnection.c \
./src/gst-plugins/commons/kmsbasertpendpoint.c \
./src/gst-plugins/commons/kmsbasesdpendpoint.c \
./src/gst-plugins/commons/kmselement.c \
./src/gst-plugins/commons/kmsloop.c \
./src/gst-plugins/commons/kmsrecordingprofile.c \
./src/gst-plugins/commons/kmshubport.c \
./src/gst-plugins/commons/kmsbasehub.c \
./src/gst-plugins/commons/kmsuriendpoint.c \
./src/gst-plugins/commons/kmsbufferlacentymeta.c \
./src/gst-plugins/commons/kmsserializablemeta.c \
./src/gst-plugins/commons/kmsstats.c \
./src/gst-plugins/commons/kmstreebin.c \
./src/gst-plugins/commons/kmsdectreebin.c \
./src/gst-plugins/commons/kmsenctreebin.c \
./src/gst-plugins/commons/kmsparsetreebin.c \
./src/gst-plugins/commons/kmslist.c \
\
./src/gst-plugins/commons/kmsrefstruct.c \
./src/gst-plugins/commons/sdp_utils.c \
./src/gst-plugins/commons/kmsutils.c \
./win32/kms-core-enumtypes.c \
./win32/kms-core-marshal.c

KMSCOMMONS_LIBS= \
-L/usr/i686-w64-mingw32/sys-root/mingw/lib \
-L/usr/lib/gcc/i686-w64-mingw32/5.2.0 \
-L/usr/i686-w64-mingw32/lib/ \
-L./build/ \
-lkmssdpagent \
-lgstpbutils-1.0.dll \
-lgstvideo-1.0.dll \
-lgstrtp-1.0.dll \
-lgstsdp-1.0.dll \
-lgstreamer-1.0 \
-lgobject-2.0 \
-lglib-2.0 \
-lrpcrt4 \
-lole32

KMSCOREIMPL_TARGET=libkmscoreimpl.dll

KMSCOREIMPL_LIBS= \
-L/usr/i686-w64-mingw32/sys-root/mingw/lib \
-L/usr/lib/gcc/i686-w64-mingw32/5.2.0 \
-L/usr/i686-w64-mingw32/lib/ \
-L./build/ \
-lkmssdpagent \
-lkmsgstcommons

KMSCOREIMPL_SRC= \
./src/server/implementation/DotGraph.cpp \
./src/server/implementation/RegisterParent.cpp \
./src/server/implementation/objects/ServerManagerImpl.cpp \
./src/server/implementation/objects/PassThroughImpl.cpp \
./src/server/implementation/objects/HubImpl.cpp \
./src/server/implementation/objects/MediaPipelineImpl.cpp \
./src/server/implementation/objects/MediaPadImpl.cpp \
./src/server/implementation/objects/SessionEndpointImpl.cpp \
./src/server/implementation/objects/SdpEndpointImpl.cpp \
./src/server/implementation/objects/HubPortImpl.cpp \
./src/server/implementation/objects/MediaElementImpl.cpp \
./src/server/implementation/objects/FilterImpl.cpp \
./src/server/implementation/objects/UriEndpointImpl.cpp \
./src/server/implementation/objects/MediaObjectImpl.cpp \
./src/server/implementation/objects/BaseRtpEndpointImpl.cpp \
./src/server/implementation/objects/EndpointImpl.cpp \
./src/server/implementation/MediaSet.cpp \
./src/server/implementation/Factory.cpp \
./src/server/implementation/ModuleManager.cpp \
./src/server/implementation/WorkerPool.cpp \
./src/server/implementation/UUIDGenerator.cpp \
./src/server/implementation/EventHandler.cpp

SDPAGENT_OBJS=$(SDPAGENT_SRC:.c=.o)
KMSCOMMONS_OBJS=$(KMSCOMMONS_SRC:.c=.o)
KMSCOREIMPL_OBJS=$(KMSCOREIMPL_SRC:.c=.o)

all: $(TARGET_DIR)/$(SDPAGENT_TARGET) $(TARGET_DIR)/$(KMSCOMMONS_TARGET) $(TARGET_DIR)/$(KMSCOREIMPL_TARGET)

$(TARGET_DIR)/$(SDPAGENT_TARGET): $(SDPAGENT_OBJS)
	mkdir -p $(TARGET_DIR)
	$(CC) -shared -o $(TARGET_DIR)/$(SDPAGENT_TARGET) $(CFLAGS) $(SDPAGENT_OBJS) $(SDPAGENT_LIBS) -Wl,--out-implib,$(TARGET_DIR)/$(SDPAGENT_TARGET).a

$(TARGET_DIR)/$(KMSCOMMONS_TARGET): $(KMSCOMMONS_OBJS)
	mkdir -p $(TARGET_DIR)
	$(CC) -shared -o $(TARGET_DIR)/$(KMSCOMMONS_TARGET) $(CFLAGS) $(KMSCOMMONS_OBJS) $(KMSCOMMONS_LIBS) -Wl,--out-implib,$(TARGET_DIR)/$(KMSCOMMONS_TARGET).a

$(TARGET_DIR)/$(KMSCOREIMPL_TARGET): $(KMSCOREIMPL_OBJS)
	mkdir -p $(TARGET_DIR)
	$(CC) -shared -o $(TARGET_DIR)/$(KMSCOREIMPL_TARGET) $(CFLAGS) $(KMSCOREIMPL_OBJS) $(KMSCOREIMPL_LIBS) -Wl,--out-implib,$(TARGET_DIR)/$(KMSCOREIMPL_TARGET).a

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm $(TARGET_DIR)/$(SDPAGENT_TARGET)
	rm $(TARGET_DIR)/$(SDPAGENT_TARGET).a

