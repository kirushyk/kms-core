AR=i686-w64-mingw32-ar
CC=i686-w64-mingw32-gcc
CXX=i686-w64-mingw32-g++
RANLIB=i686-w64-mingw32-ranlib
TARGET_DIR=./build/

CXXFLAGS=--std=gnu++17 -fpermissive

CFLAGS= \
-DKURENTO_MODULES_DIR="\".\"" \
-DPACKAGE="\"kms-core\"" \
-DVERSION="\"6.4.1.6\"" \
-Duint="unsigned" \
-DWIN32_LEAN_AND_MEAN=1 \
-I/usr/i686-w64-mingw32/sys-root/mingw/include/gstreamer-1.0 \
-I/usr/i686-w64-mingw32/sys-root/mingw/lib/gstreamer-1.0/include \
-I/usr/i686-w64-mingw32/sys-root/mingw/include/glib-2.0 \
-I/usr/i686-w64-mingw32/sys-root/mingw/lib/glib-2.0/include \
-I/usr/i686-w64-mingw32/sys-root/mingw/include/glibmm-2.4 \
-I/usr/i686-w64-mingw32/sys-root/mingw/lib/glibmm-2.4/include \
-I/usr/i686-w64-mingw32/sys-root/mingw/include/sigc++-2.0 \
-I/usr/i686-w64-mingw32/sys-root/mingw/lib/sigc++-2.0/include \
-I./win32/server/implementation/generated-cpp/ \
-I./win32/server/interface/generated-cpp/  \
-I./src/server/implementation/ \
-I./src/server/interface/ \
-I./src/gst-plugins/ \
-I./src/gst-plugins/commons/ \
-I../jsoncpp/include/ \
-I../kms-jsonrpc/src/ \
-I./src/server/implementation/objects/ \
-I./win32

KMSUTILS_TARGET=libkmsutils.a
KMSUTILS_SRC=./src/gst-plugins/commons/kmsutils.c
SDPUTILS_TARGET=libsdputils.a
SDPUTILS_SRC=./src/gst-plugins/commons/sdp_utils.c
KMSREFSTRUCT_TARGET=libkmsrefstruct.a
KMSREFSTRUCT_SRC=./src/gst-plugins/commons/kmsrefstruct.c

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
./win32/kms-sdp-agent-marshal.c

SDPAGENT_LIBS= \
-L/usr/i686-w64-mingw32/sys-root/mingw/lib \
-L/usr/lib/gcc/i686-w64-mingw32/5.2.0 \
-L/usr/i686-w64-mingw32/lib/ \
-L./build/ \
-lsdputils \
-lkmsutils \
-lkmsrefstruct \
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

KMSCOREINTERFACE_TARGET=libkmscoreinterface.a

KMSCOREINTERFACE_SRC= \
./win32/server/interface/generated-cpp/MediaObjectInternal.cpp \
./win32/server/interface/generated-cpp/ServerManagerInternal.cpp \
./win32/server/interface/generated-cpp/SessionEndpointInternal.cpp \
./win32/server/interface/generated-cpp/HubInternal.cpp \
./win32/server/interface/generated-cpp/FilterInternal.cpp \
./win32/server/interface/generated-cpp/EndpointInternal.cpp \
./win32/server/interface/generated-cpp/HubPortInternal.cpp \
./win32/server/interface/generated-cpp/PassThroughInternal.cpp \
./win32/server/interface/generated-cpp/UriEndpointInternal.cpp \
./win32/server/interface/generated-cpp/MediaPipelineInternal.cpp \
./win32/server/interface/generated-cpp/SdpEndpointInternal.cpp \
./win32/server/interface/generated-cpp/BaseRtpEndpointInternal.cpp \
./win32/server/interface/generated-cpp/MediaElementInternal.cpp \
./win32/server/interface/generated-cpp/MediaObject.cpp \
./win32/server/interface/generated-cpp/ServerManager.cpp \
./win32/server/interface/generated-cpp/SessionEndpoint.cpp \
./win32/server/interface/generated-cpp/Hub.cpp \
./win32/server/interface/generated-cpp/Filter.cpp \
./win32/server/interface/generated-cpp/Endpoint.cpp \
./win32/server/interface/generated-cpp/HubPort.cpp \
./win32/server/interface/generated-cpp/PassThrough.cpp \
./win32/server/interface/generated-cpp/UriEndpoint.cpp \
./win32/server/interface/generated-cpp/MediaPipeline.cpp \
./win32/server/interface/generated-cpp/SdpEndpoint.cpp \
./win32/server/interface/generated-cpp/BaseRtpEndpoint.cpp \
./win32/server/interface/generated-cpp/MediaElement.cpp \
./win32/server/interface/generated-cpp/RaiseBase.cpp \
./win32/server/interface/generated-cpp/Error.cpp \
./win32/server/interface/generated-cpp/MediaSessionTerminated.cpp \
./win32/server/interface/generated-cpp/MediaSessionStarted.cpp \
./win32/server/interface/generated-cpp/Media.cpp \
./win32/server/interface/generated-cpp/ObjectCreated.cpp \
./win32/server/interface/generated-cpp/ObjectDestroyed.cpp \
./win32/server/interface/generated-cpp/MediaStateChanged.cpp \
./win32/server/interface/generated-cpp/ConnectionStateChanged.cpp \
./win32/server/interface/generated-cpp/MediaFlowOutStateChange.cpp \
./win32/server/interface/generated-cpp/MediaFlowInStateChange.cpp \
./win32/server/interface/generated-cpp/ElementConnected.cpp \
./win32/server/interface/generated-cpp/ElementDisconnected.cpp \
./win32/server/interface/generated-cpp/ServerInfo.cpp \
./win32/server/interface/generated-cpp/ServerType.cpp \
./win32/server/interface/generated-cpp/GstreamerDotDetails.cpp \
./win32/server/interface/generated-cpp/ModuleInfo.cpp \
./win32/server/interface/generated-cpp/MediaState.cpp \
./win32/server/interface/generated-cpp/MediaFlowState.cpp \
./win32/server/interface/generated-cpp/ConnectionState.cpp \
./win32/server/interface/generated-cpp/MediaType.cpp \
./win32/server/interface/generated-cpp/FilterType.cpp \
./win32/server/interface/generated-cpp/VideoCodec.cpp \
./win32/server/interface/generated-cpp/AudioCodec.cpp \
./win32/server/interface/generated-cpp/Fraction.cpp \
./win32/server/interface/generated-cpp/AudioCaps.cpp \
./win32/server/interface/generated-cpp/VideoCaps.cpp \
./win32/server/interface/generated-cpp/ElementConnectionData.cpp \
./win32/server/interface/generated-cpp/Tag.cpp \
./win32/server/interface/generated-cpp/StatsType.cpp \
./win32/server/interface/generated-cpp/MediaLatencyStat.cpp \
./win32/server/interface/generated-cpp/Stats.cpp \
./win32/server/interface/generated-cpp/ElementStats.cpp \
./win32/server/interface/generated-cpp/EndpointStats.cpp \
./win32/server/interface/generated-cpp/RTCStats.cpp \
./win32/server/interface/generated-cpp/RTCRTPStreamStats.cpp \
./win32/server/interface/generated-cpp/RTCCodec.cpp \
./win32/server/interface/generated-cpp/RTCInboundRTPStreamStats.cpp \
./win32/server/interface/generated-cpp/RTCOutboundRTPStreamStats.cpp \
./win32/server/interface/generated-cpp/RTCPeerConnectionStats.cpp \
./win32/server/interface/generated-cpp/RTCMediaStreamStats.cpp \
./win32/server/interface/generated-cpp/RTCMediaStreamTrackStats.cpp \
./win32/server/interface/generated-cpp/RTCDataChannelState.cpp \
./win32/server/interface/generated-cpp/RTCDataChannelStats.cpp \
./win32/server/interface/generated-cpp/RTCTransportStats.cpp \
./win32/server/interface/generated-cpp/RTCStatsIceCandidateType.cpp \
./win32/server/interface/generated-cpp/RTCIceCandidateAttributes.cpp \
./win32/server/interface/generated-cpp/RTCStatsIceCandidatePairState.cpp \
./win32/server/interface/generated-cpp/RTCIceCandidatePairStats.cpp \
./win32/server/interface/generated-cpp/RTCCertificateStats.cpp \
./win32/server/interface/generated-cpp/CodecConfiguration.cpp \
./win32/server/interface/generated-cpp/RembParams.cpp

KMSCOREIMPL_TARGET=libkmscoreimpl.dll

KMSCOREIMPL_SRC= \
./src/server/implementation/EventHandler.cpp \
./src/server/implementation/Factory.cpp \
./src/server/implementation/MediaSet.cpp \
./src/server/implementation/ModuleManager.cpp \
./src/server/implementation/WorkerPool.cpp \
./src/server/implementation/UUIDGenerator.cpp \
./src/server/implementation/RegisterParent.cpp \
./src/server/implementation/DotGraph.cpp \
./src/server/implementation/objects/MediaObjectImpl.cpp \
./src/server/implementation/objects/ServerManagerImpl.cpp \
./src/server/implementation/objects/SessionEndpointImpl.cpp \
./src/server/implementation/objects/HubImpl.cpp \
./src/server/implementation/objects/FilterImpl.cpp \
./src/server/implementation/objects/EndpointImpl.cpp \
./src/server/implementation/objects/HubPortImpl.cpp \
./src/server/implementation/objects/PassThroughImpl.cpp \
./src/server/implementation/objects/UriEndpointImpl.cpp \
./src/server/implementation/objects/MediaPipelineImpl.cpp \
./src/server/implementation/objects/SdpEndpointImpl.cpp \
./src/server/implementation/objects/BaseRtpEndpointImpl.cpp \
./src/server/implementation/objects/MediaElementImpl.cpp \
./win32/server/implementation/generated-cpp/SerializerExpanderCore.cpp \
./win32/server/implementation/generated-cpp/MediaObjectImplInternal.cpp \
./win32/server/implementation/generated-cpp/ServerManagerImplInternal.cpp \
./win32/server/implementation/generated-cpp/SessionEndpointImplInternal.cpp \
./win32/server/implementation/generated-cpp/HubImplInternal.cpp \
./win32/server/implementation/generated-cpp/FilterImplInternal.cpp \
./win32/server/implementation/generated-cpp/EndpointImplInternal.cpp \
./win32/server/implementation/generated-cpp/HubPortImplInternal.cpp \
./win32/server/implementation/generated-cpp/PassThroughImplInternal.cpp \
./win32/server/implementation/generated-cpp/UriEndpointImplInternal.cpp \
./win32/server/implementation/generated-cpp/MediaPipelineImplInternal.cpp \
./win32/server/implementation/generated-cpp/SdpEndpointImplInternal.cpp \
./win32/server/implementation/generated-cpp/BaseRtpEndpointImplInternal.cpp \
./win32/server/implementation/generated-cpp/MediaElementImplInternal.cpp

KMSCOREIMPL_LIBS= \
-L/usr/i686-w64-mingw32/sys-root/mingw/lib \
-L/usr/lib/gcc/i686-w64-mingw32/5.2.0 \
-L/usr/i686-w64-mingw32/lib/ \
-L../kms-jsonrpc/build/ \
-L../jsoncpp/build/ \
-L./build/ \
-lsdputils \
-lkmsutils \
-lkmsrefstruct \
-lkmscoreinterface \
-lsigc-2.0 \
-lkmsjsoncpp.dll \
-lkmsjsonrpc.dll \
-lgstreamer-1.0 \
-lws2_32 \
-lgstvideo-1.0 \
-lgstsdp-1.0.dll \
-lglibmm-2.4 \
-lgobject-2.0 \
-lglib-2.0 \
-lrpcrt4 \
-lole32 \
-lkmssdpagent \
-lkmsgstcommons \
-lboost_system-mt \
-lboost_log-mt \
-lboost_log_setup-mt \
-lboost_program_options-mt \
-lboost_filesystem-mt \
-lboost_thread-mt

KMSCOREMODULE_TARGET=libkmscoremodule.dll

KMSCOREMODULE_SRC= \
./win32/server/implementation/generated-cpp/Module.cpp \
./win32/server/module_generation_time.cpp \
./win32/server/module_version.cpp \
./win32/server/module_name.cpp \
./win32/server/module_descriptor.cpp

KMSCOREMODULE_LIBS= \
-L/usr/i686-w64-mingw32/sys-root/mingw/lib \
-L/usr/lib/gcc/i686-w64-mingw32/5.2.0 \
-L/usr/i686-w64-mingw32/lib/ \
-L../kms-jsonrpc/build/ \
-L../jsoncpp/build/ \
-L./build/ \
-lsdputils \
-lkmsutils \
-lkmsrefstruct \
-lkmscoreinterface \
-lkmscoreimpl.dll \
-lkmsjsoncpp.dll \
-lkmsjsonrpc.dll \
-lglibmm-2.4 \
-lgobject-2.0 \
-lglib-2.0

KMSCOREPLUGINS_TARGET=libkmscoreplugins.dll

KMSCOREPLUGINS_SRC= \
./src/gst-plugins/kmscore.c \
./src/gst-plugins/kmsagnosticbin.c \
./src/gst-plugins/kmsagnosticbin3.c \
./src/gst-plugins/kmsfilterelement.c \
./src/gst-plugins/kmsaudiomixer.c \
./src/gst-plugins/kmsaudiomixerbin.c \
./src/gst-plugins/kmsbitratefilter.c \
./src/gst-plugins/kmsbufferinjector.c \
./src/gst-plugins/kmspassthrough.c \
./src/gst-plugins/kmsdummysrc.c \
./src/gst-plugins/kmsdummysink.c \
./src/gst-plugins/kmsdummyduplex.c \
./src/gst-plugins/kmsdummysdp.c \
./src/gst-plugins/kmsdummyrtp.c \
./src/gst-plugins/kmsdummyuri.c

KMSCOREPLUGINS_LIBS= \
-L/usr/i686-w64-mingw32/sys-root/mingw/lib \
-L/usr/lib/gcc/i686-w64-mingw32/5.2.0 \
-L/usr/i686-w64-mingw32/lib/ \
-L./build/ \
-lkmssdpagent \
-lkmsgstcommons \
-lgstbase-1.0.dll \
-lgstreamer-1.0 \
-lgobject-2.0 \
-lglib-2.0

VP8PARSE_TARGET=libvp8parse.dll

VP8PARSE_SRC= \
src/gst-plugins/vp8parse/kmsvp8parse.c \
src/gst-plugins/vp8parse/vp8parse.c

VP8PARSE_LIBS= \
-L/usr/i686-w64-mingw32/sys-root/mingw/lib \
-L/usr/lib/gcc/i686-w64-mingw32/5.2.0 \
-L/usr/i686-w64-mingw32/lib/ \
-lgstreamer-1.0 \
-lgobject-2.0 \
-lglib-2.0

SDPAGENT_OBJS=$(SDPAGENT_SRC:.c=.o)
KMSCOMMONS_OBJS=$(KMSCOMMONS_SRC:.c=.o)
KMSCOREINTERFACE_OBJS=$(KMSCOREINTERFACE_SRC:.cpp=.o)
KMSCOREIMPL_OBJS=$(KMSCOREIMPL_SRC:.cpp=.o)
KMSCOREMODULE_OBJS=$(KMSCOREMODULE_SRC:.cpp=.o)
KMSCOREPLUGINS_OBJS=$(KMSCOREPLUGINS_SRC:.c=.o)

all: \
$(TARGET_DIR)/$(KMSUTILS_TARGET) \
$(TARGET_DIR)/$(SDPUTILS_TARGET) \
$(TARGET_DIR)/$(KMSREFSTRUCT_TARGET) \
$(TARGET_DIR)/$(SDPAGENT_TARGET) \
$(TARGET_DIR)/$(KMSCOMMONS_TARGET) \
$(TARGET_DIR)/$(KMSCOREINTERFACE_TARGET) \
$(TARGET_DIR)/$(KMSCOREIMPL_TARGET) \
$(TARGET_DIR)/$(KMSCOREMODULE_TARGET) \

#$(TARGET_DIR)/$(KMSCOREPLUGINS_TARGET) \
#$(TARGET_DIR)/$(VP8PARSE_TARGET)

$(TARGET_DIR)/$(KMSUTILS_TARGET): $(KMSUTILS_SRC)
	mkdir -p $(TARGET_DIR)
	$(CC) -c $(CFLAGS) -o $(TARGET_DIR)/$(KMSUTILS_TARGET).o $(KMSUTILS_SRC)
	$(AR) cr $(TARGET_DIR)/$(KMSUTILS_TARGET) $(TARGET_DIR)/$(KMSUTILS_TARGET).o
	$(RANLIB) $(TARGET_DIR)/$(KMSUTILS_TARGET)

$(TARGET_DIR)/$(SDPUTILS_TARGET): $(SDPUTILS_SRC)
	mkdir -p $(TARGET_DIR)
	$(CC) -c $(CFLAGS) -o $(TARGET_DIR)/$(SDPUTILS_TARGET).o $(SDPUTILS_SRC)
	$(AR) cr $(TARGET_DIR)/$(SDPUTILS_TARGET) $(TARGET_DIR)/$(SDPUTILS_TARGET).o
	$(RANLIB) $(TARGET_DIR)/$(SDPUTILS_TARGET)

$(TARGET_DIR)/$(KMSREFSTRUCT_TARGET): $(KMSREFSTRUCT_SRC)
	mkdir -p $(TARGET_DIR)
	$(CC) -c $(CFLAGS) -o $(TARGET_DIR)/$(KMSREFSTRUCT_TARGET).o $(KMSREFSTRUCT_SRC)
	$(AR) cr $(TARGET_DIR)/$(KMSREFSTRUCT_TARGET) $(TARGET_DIR)/$(KMSREFSTRUCT_TARGET).o
	$(RANLIB) $(TARGET_DIR)/$(KMSREFSTRUCT_TARGET)

$(TARGET_DIR)/$(SDPAGENT_TARGET): $(SDPAGENT_OBJS)
	mkdir -p $(TARGET_DIR)
	$(CC) -shared -o $(TARGET_DIR)/$(SDPAGENT_TARGET) $(CFLAGS) $(SDPAGENT_OBJS) $(SDPAGENT_LIBS) -Wl,--out-implib,$(TARGET_DIR)/$(SDPAGENT_TARGET).a

$(TARGET_DIR)/$(KMSCOMMONS_TARGET): $(KMSCOMMONS_OBJS)
	mkdir -p $(TARGET_DIR)
	$(CC) -shared -o $(TARGET_DIR)/$(KMSCOMMONS_TARGET) $(CFLAGS) $(KMSCOMMONS_OBJS) $(KMSCOMMONS_LIBS) -Wl,--out-implib,$(TARGET_DIR)/$(KMSCOMMONS_TARGET).a

$(TARGET_DIR)/$(KMSCOREINTERFACE_TARGET): $(KMSCOREINTERFACE_OBJS)
	mkdir -p $(TARGET_DIR)
	$(AR) cr $(TARGET_DIR)/$(KMSCOREINTERFACE_TARGET) $(KMSCOREINTERFACE_OBJS)
	$(RANLIB) $(TARGET_DIR)/$(KMSCOREINTERFACE_TARGET)

$(TARGET_DIR)/$(KMSCOREIMPL_TARGET): $(KMSCOREIMPL_OBJS)
	mkdir -p $(TARGET_DIR)
	$(CXX) -shared -o $(TARGET_DIR)/$(KMSCOREIMPL_TARGET) $(CFLAGS) $(KMSCOREIMPL_OBJS) $(KMSCOREIMPL_LIBS) -Wl,--out-implib,$(TARGET_DIR)/$(KMSCOREIMPL_TARGET).a

$(TARGET_DIR)/$(KMSCOREMODULE_TARGET): $(KMSCOREMODULE_OBJS)
	mkdir -p $(TARGET_DIR)
	$(CXX) -shared -o $(TARGET_DIR)/$(KMSCOREMODULE_TARGET) $(CFLAGS) $(KMSCOREMODULE_OBJS) $(KMSCOREMODULE_LIBS) -Wl,--out-implib,$(TARGET_DIR)/$(KMSCOREMODULE_TARGET).a

$(TARGET_DIR)/$(KMSCOREPLUGINS_TARGET): $(KMSCOREPLUGINS_OBJS)
	mkdir -p $(TARGET_DIR)
	$(CC) -shared -o $(TARGET_DIR)/$(KMSCOREPLUGINS_TARGET) $(CFLAGS) $(KMSCOREPLUGINS_OBJS) $(KMSCOREPLUGINS_LIBS) -Wl,--out-implib,$(TARGET_DIR)/$(KMSCOREPLUGINS_TARGET).a

$(TARGET_DIR)/$(VP8PARSE_TARGET): $(VP8PARSE_OBJS)
	mkdir -p $(TARGET_DIR)
	$(CC) -shared -o $(TARGET_DIR)/$(VP8PARSE_TARGET) $(CFLAGS) $(VP8PARSE_OBJS) $(VP8PARSE_LIBS) -Wl,--out-implib,$(TARGET_DIR)/$(VP8PARSE_TARGET).a

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.cpp
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(TARGET_DIR)/$(KMSUTILS_TARGET)
	rm -f $(TARGET_DIR)/$(SDPUTILS_TARGET)
	rm -f $(TARGET_DIR)/$(KMSREFSTRUCT_TARGET)
	rm -f $(SDPAGENT_OBJS)
	rm -f $(TARGET_DIR)/$(SDPAGENT_TARGET)
	rm -f $(TARGET_DIR)/$(SDPAGENT_TARGET).a
	rm -f $(KMSCOMMONS_OBJS)
	rm -f $(TARGET_DIR)/$(KMSCOMMONS_TARGET)
	rm -f $(TARGET_DIR)/$(KMSCOMMONS_TARGET).a
	rm -f $(KMSCOREINTERFACE_OBJS)
	rm -f $(TARGET_DIR)/$(KMSCOREINTERFACE_TARGET)
	rm -f $(KMSCOREIMPL_OBJS)
	rm -f $(TARGET_DIR)/$(KMSCOREIMPL_TARGET)
	rm -f $(TARGET_DIR)/$(KMSCOREIMPL_TARGET).a
	rm -f $(KMSCOREMODULE_OBJS)
	rm -f $(TARGET_DIR)/$(KMSCOREMODULE_TARGET)
	rm -f $(TARGET_DIR)/$(KMSCOREMODULE_TARGET).a
	rm -f $(KMSCOREPLUGINS_OBJS)
	rm -f $(TARGET_DIR)/$(KMSCOREPLUGINS_TARGET)
	rm -f $(TARGET_DIR)/$(KMSCOREPLUGINS_TARGET).a

