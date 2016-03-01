CC=i686-w64-mingw32-gcc
CXX=i686-w64-mingw32-g++
TARGET_DIR=./build/

CXXFLAGS=--std=gnu++17 -fpermissive

CFLAGS= \
-I/usr/i686-w64-mingw32/sys-root/mingw/include/gstreamer-1.0 \
-I/usr/i686-w64-mingw32/sys-root/mingw/lib/gstreamer-1.0/include \
-I/usr/i686-w64-mingw32/sys-root/mingw/include/glib-2.0 \
-I/usr/i686-w64-mingw32/sys-root/mingw/lib/glib-2.0/include \
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
-L../kms-jsonrpc/build/ \
-L../jsoncpp/build/ \
-L./build/ \
-lsigc-2.0 \
-lkmsjsoncpp.dll \
-lkmsjsonrpc.dll \
-lkmssdpagent \
-lkmsgstcommons

KMSCOREIMPL_SRC= \
./src/server/implementation/DotGraph.cpp \
./src/server/implementation/RegisterParent.cpp \
./src/server/implementation/objects/ServerManagerImpl.cpp \
./src/server/implementation/objects/PassThroughImpl.cpp \
./src/server/implementation/objects/HubImpl.cpp \
./src/server/implementation/objects/MediaPipelineImpl.cpp \
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
./src/server/implementation/EventHandler.cpp \
\
./win32/server/module_version.cpp \
./win32/server/module_name.cpp \
./win32/server/implementation/generated-cpp/SessionEndpointImplInternal.cpp \
./win32/server/implementation/generated-cpp/MediaElementImplInternal.cpp \
./win32/server/implementation/generated-cpp/MediaPipelineImplInternal.cpp \
./win32/server/implementation/generated-cpp/UriEndpointImplInternal.cpp \
./win32/server/implementation/generated-cpp/SdpEndpointImplInternal.cpp \
./win32/server/implementation/generated-cpp/HubPortImplInternal.cpp \
./win32/server/implementation/generated-cpp/HubImplInternal.cpp \
./win32/server/implementation/generated-cpp/SerializerExpanderCore.cpp \
./win32/server/implementation/generated-cpp/EndpointImplInternal.cpp \
./win32/server/implementation/generated-cpp/FilterImplInternal.cpp \
./win32/server/implementation/generated-cpp/PassThroughImplInternal.cpp \
./win32/server/implementation/generated-cpp/ServerManagerImplInternal.cpp \
./win32/server/implementation/generated-cpp/MediaObjectImplInternal.cpp \
./win32/server/implementation/generated-cpp/BaseRtpEndpointImplInternal.cpp \
./win32/server/implementation/generated-cpp/Module.cpp \
./win32/server/module_descriptor.cpp \
./win32/server/interface/generated-cpp/RTCStatsIceCandidatePairState.cpp \
./win32/server/interface/generated-cpp/RTCTransportStats.cpp \
./win32/server/interface/generated-cpp/RaiseBase.cpp \
./win32/server/interface/generated-cpp/ObjectDestroyed.cpp \
./win32/server/interface/generated-cpp/Error.cpp \
./win32/server/interface/generated-cpp/ElementDisconnected.cpp \
./win32/server/interface/generated-cpp/SdpEndpointInternal.cpp \
./win32/server/interface/generated-cpp/EndpointInternal.cpp \
./win32/server/interface/generated-cpp/CodecConfiguration.cpp \
./win32/server/interface/generated-cpp/Hub.cpp \
./win32/server/interface/generated-cpp/RTCInboundRTPStreamStats.cpp \
./win32/server/interface/generated-cpp/HubInternal.cpp \
./win32/server/interface/generated-cpp/ElementStats.cpp \
./win32/server/interface/generated-cpp/UriEndpoint.cpp \
./win32/server/interface/generated-cpp/ModuleInfo.cpp \
./win32/server/interface/generated-cpp/RTCStats.cpp \
./win32/server/interface/generated-cpp/Filter.cpp \
./win32/server/interface/generated-cpp/MediaState.cpp \
./win32/server/interface/generated-cpp/RTCMediaStreamStats.cpp \
./win32/server/interface/generated-cpp/MediaPipeline.cpp \
./win32/server/interface/generated-cpp/EndpointStats.cpp \
./win32/server/interface/generated-cpp/ConnectionStateChanged.cpp \
./win32/server/interface/generated-cpp/RTCRTPStreamStats.cpp \
./win32/server/interface/generated-cpp/RTCDataChannelState.cpp \
./win32/server/interface/generated-cpp/MediaElementInternal.cpp \
./win32/server/interface/generated-cpp/MediaStateChanged.cpp \
./win32/server/interface/generated-cpp/ElementConnectionData.cpp \
./win32/server/interface/generated-cpp/MediaFlowInStateChange.cpp \
./win32/server/interface/generated-cpp/MediaObject.cpp \
./win32/server/interface/generated-cpp/ObjectCreated.cpp \
./win32/server/interface/generated-cpp/SessionEndpoint.cpp \
./win32/server/interface/generated-cpp/AudioCaps.cpp \
./win32/server/interface/generated-cpp/RTCIceCandidatePairStats.cpp \
./win32/server/interface/generated-cpp/RTCMediaStreamTrackStats.cpp \
./win32/server/interface/generated-cpp/MediaSessionTerminated.cpp \
./win32/server/interface/generated-cpp/ServerManager.cpp \
./win32/server/interface/generated-cpp/SessionEndpointInternal.cpp \
./win32/server/interface/generated-cpp/Media.cpp \
./win32/server/interface/generated-cpp/ServerInfo.cpp \
./win32/server/interface/generated-cpp/BaseRtpEndpointInternal.cpp \
./win32/server/interface/generated-cpp/RTCIceCandidateAttributes.cpp \
./win32/server/interface/generated-cpp/Endpoint.cpp \
./win32/server/interface/generated-cpp/MediaFlowOutStateChange.cpp \
./win32/server/interface/generated-cpp/PassThroughInternal.cpp \
./win32/server/interface/generated-cpp/MediaObjectInternal.cpp \
./win32/server/interface/generated-cpp/MediaElement.cpp \
./win32/server/interface/generated-cpp/RTCCertificateStats.cpp \
./win32/server/interface/generated-cpp/StatsType.cpp \
./win32/server/interface/generated-cpp/Stats.cpp \
./win32/server/interface/generated-cpp/RTCCodec.cpp \
./win32/server/interface/generated-cpp/PassThrough.cpp \
./win32/server/interface/generated-cpp/RTCPeerConnectionStats.cpp \
./win32/server/interface/generated-cpp/AudioCodec.cpp \
./win32/server/interface/generated-cpp/HubPortInternal.cpp \
./win32/server/interface/generated-cpp/Tag.cpp \
./win32/server/interface/generated-cpp/FilterType.cpp \
./win32/server/interface/generated-cpp/MediaType.cpp \
./win32/server/interface/generated-cpp/FilterInternal.cpp \
./win32/server/interface/generated-cpp/UriEndpointInternal.cpp \
./win32/server/interface/generated-cpp/MediaLatencyStat.cpp \
./win32/server/interface/generated-cpp/SdpEndpoint.cpp \
./win32/server/interface/generated-cpp/VideoCodec.cpp \
./win32/server/interface/generated-cpp/RTCStatsIceCandidateType.cpp \
./win32/server/interface/generated-cpp/RembParams.cpp \
./win32/server/interface/generated-cpp/MediaPipelineInternal.cpp \
./win32/server/interface/generated-cpp/Fraction.cpp \
./win32/server/interface/generated-cpp/RTCDataChannelStats.cpp \
./win32/server/interface/generated-cpp/ServerType.cpp \
./win32/server/interface/generated-cpp/RTCOutboundRTPStreamStats.cpp \
./win32/server/interface/generated-cpp/HubPort.cpp \
./win32/server/interface/generated-cpp/MediaFlowState.cpp \
./win32/server/interface/generated-cpp/GstreamerDotDetails.cpp \
./win32/server/interface/generated-cpp/ConnectionState.cpp \
./win32/server/interface/generated-cpp/ElementConnected.cpp \
./win32/server/interface/generated-cpp/ServerManagerInternal.cpp \
./win32/server/interface/generated-cpp/VideoCaps.cpp \
./win32/server/interface/generated-cpp/BaseRtpEndpoint.cpp \
./win32/server/interface/generated-cpp/MediaSessionStarted.cpp \
./win32/server/module_generation_time.cpp

#./src/server/implementation/objects/MediaPadImpl.cpp \

SDPAGENT_OBJS=$(SDPAGENT_SRC:.c=.o)
KMSCOMMONS_OBJS=$(KMSCOMMONS_SRC:.c=.o)
KMSCOREIMPL_OBJS=$(KMSCOREIMPL_SRC:.cpp=.o)

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

%.o: %.cpp
	$(CC) -c $(CFLAGS) $(CXXFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm $(TARGET_DIR)/$(SDPAGENT_TARGET)
	rm $(TARGET_DIR)/$(SDPAGENT_TARGET).a

