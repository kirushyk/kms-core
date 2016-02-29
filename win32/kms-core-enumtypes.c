


#include "kms-core-enumtypes.h"
#include "kmsuriendpointstate.h"
#include "kmsrecordingprofile.h"
#include "kmsmediatype.h"
#include "kmsfiltertype.h"
#include "kmselementpadtype.h"
#include "kmsmediastate.h"
#include "kmsconnectionstate.h"

/* enumerations from "kmsuriendpointstate.h" */
GType
kms_uri_endpoint_state_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { KMS_URI_ENDPOINT_STATE_STOP, "KMS_URI_ENDPOINT_STATE_STOP", "stop" },
      { KMS_URI_ENDPOINT_STATE_START, "KMS_URI_ENDPOINT_STATE_START", "start" },
      { KMS_URI_ENDPOINT_STATE_PAUSE, "KMS_URI_ENDPOINT_STATE_PAUSE", "pause" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = g_enum_register_static ("KmsUriEndpointState", values);
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
  return g_define_type_id__volatile;
}

/* enumerations from "kmsrecordingprofile.h" */
GType
kms_recording_profile_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { KMS_RECORDING_PROFILE_NONE, "KMS_RECORDING_PROFILE_NONE", "none" },
      { KMS_RECORDING_PROFILE_WEBM, "KMS_RECORDING_PROFILE_WEBM", "webm" },
      { KMS_RECORDING_PROFILE_MP4, "KMS_RECORDING_PROFILE_MP4", "mp4" },
      { KMS_RECORDING_PROFILE_WEBM_VIDEO_ONLY, "KMS_RECORDING_PROFILE_WEBM_VIDEO_ONLY", "webm-video-only" },
      { KMS_RECORDING_PROFILE_WEBM_AUDIO_ONLY, "KMS_RECORDING_PROFILE_WEBM_AUDIO_ONLY", "webm-audio-only" },
      { KMS_RECORDING_PROFILE_MP4_VIDEO_ONLY, "KMS_RECORDING_PROFILE_MP4_VIDEO_ONLY", "mp4-video-only" },
      { KMS_RECORDING_PROFILE_MP4_AUDIO_ONLY, "KMS_RECORDING_PROFILE_MP4_AUDIO_ONLY", "mp4-audio-only" },
      { KMS_RECORDING_PROFILE_KSR, "KMS_RECORDING_PROFILE_KSR", "ksr" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = g_enum_register_static ("KmsRecordingProfile", values);
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
  return g_define_type_id__volatile;
}

/* enumerations from "kmsmediatype.h" */
GType
kms_media_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { KMS_MEDIA_TYPE_AUDIO, "KMS_MEDIA_TYPE_AUDIO", "audio" },
      { KMS_MEDIA_TYPE_VIDEO, "KMS_MEDIA_TYPE_VIDEO", "video" },
      { KMS_MEDIA_TYPE_DATA, "KMS_MEDIA_TYPE_DATA", "data" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = g_enum_register_static ("KmsMediaType", values);
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
  return g_define_type_id__volatile;
}

/* enumerations from "kmsfiltertype.h" */
GType
kms_filter_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { KMS_FILTER_TYPE_AUTODETECT, "KMS_FILTER_TYPE_AUTODETECT", "autodetect" },
      { KMS_FILTER_TYPE_AUDIO, "KMS_FILTER_TYPE_AUDIO", "audio" },
      { KMS_FILTER_TYPE_VIDEO, "KMS_FILTER_TYPE_VIDEO", "video" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = g_enum_register_static ("KmsFilterType", values);
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
  return g_define_type_id__volatile;
}

/* enumerations from "kmselementpadtype.h" */
GType
kms_element_pad_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { KMS_ELEMENT_PAD_TYPE_DATA, "KMS_ELEMENT_PAD_TYPE_DATA", "data" },
      { KMS_ELEMENT_PAD_TYPE_AUDIO, "KMS_ELEMENT_PAD_TYPE_AUDIO", "audio" },
      { KMS_ELEMENT_PAD_TYPE_VIDEO, "KMS_ELEMENT_PAD_TYPE_VIDEO", "video" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = g_enum_register_static ("KmsElementPadType", values);
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
  return g_define_type_id__volatile;
}

/* enumerations from "kmsmediastate.h" */
GType
kms_media_state_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { KMS_MEDIA_STATE_DISCONNECTED, "KMS_MEDIA_STATE_DISCONNECTED", "disconnected" },
      { KMS_MEDIA_STATE_CONNECTED, "KMS_MEDIA_STATE_CONNECTED", "connected" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = g_enum_register_static ("KmsMediaState", values);
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
  return g_define_type_id__volatile;
}

/* enumerations from "kmsconnectionstate.h" */
GType
kms_connection_state_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { KMS_CONNECTION_STATE_DISCONNECTED, "KMS_CONNECTION_STATE_DISCONNECTED", "disconnected" },
      { KMS_CONNECTION_STATE_CONNECTED, "KMS_CONNECTION_STATE_CONNECTED", "connected" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = g_enum_register_static ("KmsConnectionState", values);
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
  return g_define_type_id__volatile;
}



