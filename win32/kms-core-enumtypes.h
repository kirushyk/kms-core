


#ifndef __KMS_KMS_CORE_ENUMTYPES_ENUM_TYPES_H__
#define __KMS_KMS_CORE_ENUMTYPES_ENUM_TYPES_H__

#include <glib-object.h>

G_BEGIN_DECLS

/* enumerations from "kmsuriendpointstate.h" */
GType kms_uri_endpoint_state_get_type (void);
#define KMS_TYPE_URI_ENDPOINT_STATE (kms_uri_endpoint_state_get_type())

/* enumerations from "kmsrecordingprofile.h" */
GType kms_recording_profile_get_type (void);
#define KMS_TYPE_RECORDING_PROFILE (kms_recording_profile_get_type())

/* enumerations from "kmsmediatype.h" */
GType kms_media_type_get_type (void);
#define KMS_TYPE_MEDIA_TYPE (kms_media_type_get_type())

/* enumerations from "kmsfiltertype.h" */
GType kms_filter_type_get_type (void);
#define KMS_TYPE_FILTER_TYPE (kms_filter_type_get_type())

/* enumerations from "kmselementpadtype.h" */
GType kms_element_pad_type_get_type (void);
#define KMS_TYPE_ELEMENT_PAD_TYPE (kms_element_pad_type_get_type())

/* enumerations from "kmsmediastate.h" */
GType kms_media_state_get_type (void);
#define KMS_TYPE_MEDIA_STATE (kms_media_state_get_type())

/* enumerations from "kmsconnectionstate.h" */
GType kms_connection_state_get_type (void);
#define KMS_TYPE_CONNECTION_STATE (kms_connection_state_get_type())

G_END_DECLS

#endif /* __KMS_KMS_CORE_ENUMTYPES_ENUM_TYPES_H__ */



