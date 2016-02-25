/*
 * (C) Copyright 2014 Kurento (http://kurento.org/)
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/check/gstcheck.h>
#include <gst/gst.h>
#include <glib.h>

#include "sdp_utils.h"
#include "kmssdpagent.h"
#include "kmsisdppayloadmanager.h"
#include "kmssdpmediahandler.h"
#include "kmssdppayloadmanager.h"
#include "kmssdpsctpmediahandler.h"
#include "kmssdprtpavpmediahandler.h"
#include "kmssdprtpavpfmediahandler.h"
#include "kmssdprtpsavpmediahandler.h"
#include "kmssdprtpsavpfmediahandler.h"
#include "kmssdpsdesext.h"
#include "kmssdpconnectionext.h"

#include "kmssdpbundlegroup.h"
#include "kmssdpagentcommon.h"

#define OFFERER_ADDR "222.222.222.222"
#define ANSWERER_ADDR "111.111.111.111"

typedef void (*CheckSdpNegotiationFunc) (const GstSDPMessage * offer,
    const GstSDPMessage * answer, gpointer data);

static gchar *audio_codecs[] = {
  "PCMU/8000/1",
  "opus/48000/2",
  "AMR/8000/1"
};

static gchar *video_codecs[] = {
  "H263-1998/90000",
  "VP8/90000",
  "MP4V-ES/90000",
  "H264/90000"
};

static void
set_default_codecs (KmsSdpRtpAvpMediaHandler * handler, gchar ** audio_list,
    guint audio_len, gchar ** video_list, guint video_len)
{
  KmsSdpPayloadManager *ptmanager;
  GError *err = NULL;
  guint i;

  ptmanager = kms_sdp_payload_manager_new ();
  kms_sdp_rtp_avp_media_handler_use_payload_manager (handler,
      KMS_I_SDP_PAYLOAD_MANAGER (ptmanager), &err);

  for (i = 0; i < audio_len; i++) {
    fail_unless (kms_sdp_rtp_avp_media_handler_add_audio_codec (handler,
            audio_list[i], &err));
  }

  for (i = 0; i < video_len; i++) {
    fail_unless (kms_sdp_rtp_avp_media_handler_add_video_codec (handler,
            video_list[i], &err));
  }
}

static void
sdp_agent_create_offer (KmsSdpAgent * agent)
{
  GError *err = NULL;
  GstSDPMessage *offer;
  gchar *sdp_str = NULL;

  offer = kms_sdp_agent_create_offer (agent, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  gst_sdp_message_free (offer);

  fail_if (!kms_sdpagent_cancel_offer (agent, &err));
}

GST_START_TEST (sdp_agent_test_create_offer)
{
  KmsSdpAgent *agent;

  agent = kms_sdp_agent_new ();
  fail_if (agent == NULL);

  g_object_set (agent, "addr", OFFERER_ADDR, NULL);

  sdp_agent_create_offer (agent);

  g_object_set (agent, "use-ipv6", TRUE, "addr",
      "0:0:0:0:0:ffff:d4d4:d4d4", NULL);

  sdp_agent_create_offer (agent);

  g_object_unref (agent);
}

GST_END_TEST;

GST_START_TEST (sdp_agent_test_add_proto_handler)
{
  KmsSdpAgent *agent;
  KmsSdpMediaHandler *handler;
  gint id;

  agent = kms_sdp_agent_new ();
  fail_if (agent == NULL);

  handler =
      KMS_SDP_MEDIA_HANDLER (g_object_new (KMS_TYPE_SDP_MEDIA_HANDLER, NULL));
  fail_if (handler == NULL);

  /* Try to add an invalid handler */
  id = kms_sdp_agent_add_proto_handler (agent, "audio", handler);
  fail_if (id >= 0);
  g_object_unref (handler);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_sctp_media_handler_new ());
  fail_if (handler == NULL);

  id = kms_sdp_agent_add_proto_handler (agent, "application", handler);
  fail_if (id < 0);

  sdp_agent_create_offer (agent);

  g_object_unref (agent);
}

GST_END_TEST;

static const gchar *sdp_offer_str = "v=0\r\n"
    "o=- 0 0 IN IP4 0.0.0.0\r\n"
    "s=TestSession\r\n"
    "c=IN IP4 0.0.0.0\r\n"
    "t=2873397496 2873404696\r\n"
    "m=audio 9 RTP/AVP 0\r\n" "a=rtpmap:0 PCMU/8000\r\n" "a=sendonly\r\n"
    "m=video 9 RTP/AVP 96\r\n" "a=rtpmap:96 VP8/90000\r\n" "a=sendonly\r\n";

GST_START_TEST (sdp_agent_test_rejected_negotiation)
{
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  KmsSdpAgent *answerer;
  KmsSdpMediaHandler *handler;
  gint id;
  gchar *sdp_str = NULL;
  guint i, len;
  SdpMessageContext *ctx;

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_sctp_media_handler_new ());
  fail_if (handler == NULL);

  id = kms_sdp_agent_add_proto_handler (answerer, "application", handler);
  fail_if (id < 0);

  fail_unless (gst_sdp_message_new (&offer) == GST_SDP_OK);
  fail_unless (gst_sdp_message_parse_buffer ((const guint8 *)
          sdp_offer_str, -1, offer) == GST_SDP_OK);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  len = gst_sdp_message_medias_len (answer);

  for (i = 0; i < len; i++) {
    const GstSDPMedia *media;

    media = gst_sdp_message_get_media (answer, i);
    fail_if (media == NULL);

    /* Media should have been rejected */
    fail_if (media->port != 0);
  }

  gst_sdp_message_free (answer);
  g_object_unref (answerer);
}

GST_END_TEST;

static const gchar *sdp_offer_no_common_media_str = "v=0\r\n"
    "o=- 0 0 IN IP4 0.0.0.0\r\n"
    "s=Kurento Media Server\r\n"
    "c=IN IP4 0.0.0.0\r\n"
    "t=0 0\r\n"
    "m=audio 30000 RTP/AVP 0\r\n"
    "a=mid:1\r\n"
    "m=audio 30002 RTP/AVP 8\r\n"
    "a=mid:2\r\n" "m=audio 30004 RTP/AVP 3\r\n" "a=mid:3\r\n";

static void
test_sdp_pattern_offer (const gchar * sdp_patter, KmsSdpAgent * answerer,
    CheckSdpNegotiationFunc func, gpointer data)
{
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  gchar *sdp_str = NULL;
  SdpMessageContext *ctx;

  fail_unless (gst_sdp_message_new (&offer) == GST_SDP_OK);
  fail_unless (gst_sdp_message_parse_buffer ((const guint8 *)
          sdp_patter, -1, offer) == GST_SDP_OK);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  if (func) {
    func (offer, answer, data);
  }

  gst_sdp_message_free (answer);
}

static void
check_unsupported_medias (const GstSDPMessage * offer,
    const GstSDPMessage * answer, gpointer data)
{
  guint i, len;

  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  len = gst_sdp_message_medias_len (answer);
  for (i = 0; i < len; i++) {
    const GstSDPMedia *media;

    media = gst_sdp_message_get_media (answer, i);
    if (i < 1) {
      /* Only medias from 1 forward must be rejected */
      continue;
    }

    fail_if (media->port != 0);
  }
}

GST_START_TEST (sdp_agent_test_rejected_unsupported_media)
{
  KmsSdpAgent *answerer;
  KmsSdpMediaHandler *handler;
  gint id;

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id < 0);

  test_sdp_pattern_offer (sdp_offer_no_common_media_str, answerer,
      check_unsupported_medias, NULL);

  g_object_unref (answerer);
}

GST_END_TEST;

static const gchar *sdp_offer_sctp_str = "v=0\r\n"
    "o=- 0 0 IN IP4 0.0.0.0\r\n"
    "s=TestSession\r\n"
    "c=IN IP4 0.0.0.0\r\n"
    "t=2873397496 2873404696\r\n"
    "m=audio 9 RTP/AVP 0\r\n" "a=rtpmap:0 PCMU/8000\r\n" "a=sendonly\r\n"
    "m=video 9 RTP/AVP 96\r\n" "a=rtpmap:96 VP8/90000\r\n" "a=sendonly\r\n"
    "m=application 9 DTLS/SCTP 5000 5001 5002\r\n"
    "a=setup:actpass\r\n"
    "a=sendonly\r\n"
    "a=sctpmap:5000 webrtc-datachannel 1024\r\n"
    "a=sctpmap:5001 bfcp 2\r\n"
    "a=sctpmap:5002 t38 1\r\n"
    "a=webrtc-datachannel:5000 stream=1;label=\"channel 1\";subprotocol=\"chat\"\r\n"
    "a=webrtc-datachannel:5000 stream=2;label=\"channel 2\";subprotocol=\"file transfer\";max_retr=3\r\n"
    "a=bfcp:5000 stream=2;label=\"channel 2\";subprotocol=\"file transfer\";max_retr=3\r\n";

GST_START_TEST (sdp_agent_test_sctp_negotiation)
{
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  KmsSdpAgent *answerer;
  KmsSdpMediaHandler *handler;
  gint id;
  gchar *sdp_str = NULL;
  guint i, len;
  SdpMessageContext *ctx;

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_sctp_media_handler_new ());
  fail_if (handler == NULL);

  id = kms_sdp_agent_add_proto_handler (answerer, "application", handler);
  fail_if (id < 0);

  fail_unless (gst_sdp_message_new (&offer) == GST_SDP_OK);
  fail_unless (gst_sdp_message_parse_buffer ((const guint8 *)
          sdp_offer_sctp_str, -1, offer) == GST_SDP_OK);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  len = gst_sdp_message_medias_len (answer);

  for (i = 0; i < len; i++) {
    const GstSDPMedia *media;

    media = gst_sdp_message_get_media (answer, i);
    fail_if (media == NULL);

    /* Media should have been rejected */
    if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0) {
      fail_if (media->port != 0);
      continue;
    } else {
      fail_if (media->port == 0);
    }

    /* This negotiation should only have 5 attributes */
    fail_if (gst_sdp_media_attributes_len (media) != 5);
  }

  gst_sdp_message_free (answer);
  g_object_unref (answerer);
}

GST_END_TEST;

static gboolean
set_media_direction (GstSDPMedia * media, const gchar * direction)
{
  return gst_sdp_media_add_attribute (media, direction, "") == GST_SDP_OK;
}

static gboolean
expected_media_direction (const GstSDPMedia * media, const gchar * expected)
{
  guint i, attrs_len;

  attrs_len = gst_sdp_media_attributes_len (media);

  for (i = 0; i < attrs_len; i++) {
    const GstSDPAttribute *attr;

    attr = gst_sdp_media_get_attribute (media, i);

    if (g_ascii_strcasecmp (attr->key, expected) == 0) {
      return TRUE;
    }
  }

  return FALSE;
}

static void
negotiate_rtp_avp (const gchar * direction, const gchar * expected)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  gint id;
  gchar *sdp_str = NULL;
  const GstSDPMedia *media;
  SdpMessageContext *ctx;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  g_object_set (offerer, "addr", OFFERER_ADDR, NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  g_object_set (answerer, "addr", ANSWERER_ADDR, NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  fail_unless (sdp_utils_for_each_media (offer,
          (GstSDPMediaFunc) set_media_direction, (gpointer) direction));

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  /* Check only audio media */
  media = gst_sdp_message_get_media (answer, 1);
  fail_if (media == NULL);

  fail_if (!expected_media_direction (media, expected));

  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  gst_sdp_message_free (answer);

  g_object_unref (offerer);
  g_object_unref (answerer);
}

GST_START_TEST (sdp_agent_test_rtp_avp_negotiation)
{
  negotiate_rtp_avp ("sendonly", "recvonly");
  negotiate_rtp_avp ("recvonly", "sendonly");
  negotiate_rtp_avp ("sendrecv", "sendrecv");
}

GST_END_TEST static void
check_all_is_negotiated (GstSDPMessage * offer, GstSDPMessage * answer)
{
  guint i, len;

  /* Same number of medias must be in answer */
  len = gst_sdp_message_medias_len (offer);

  fail_if (len != gst_sdp_message_medias_len (answer));

  for (i = 0; i < len; i++) {
    const GstSDPMedia *m_offer, *m_answer;

    m_offer = gst_sdp_message_get_media (offer, i);
    m_answer = gst_sdp_message_get_media (answer, i);

    /* Media should be active */
    fail_if (gst_sdp_media_get_port (m_answer) == 0);

    /* Media should have the same protocol */
    fail_if (g_strcmp0 (gst_sdp_media_get_proto (m_offer),
            gst_sdp_media_get_proto (m_answer)) != 0);
  }
}

GST_START_TEST (sdp_agent_test_rtp_avpf_negotiation)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  gint id;
  gchar *sdp_str = NULL;
  SdpMessageContext *ctx;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  g_object_set (offerer, "addr", OFFERER_ADDR, NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  g_object_set (answerer, "addr", ANSWERER_ADDR, NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  check_all_is_negotiated (offer, answer);

  gst_sdp_message_free (answer);
  g_object_unref (offerer);
  g_object_unref (answerer);
}

GST_END_TEST
GST_START_TEST (sdp_agent_test_rtp_savp_negotiation)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  gint id;
  gchar *sdp_str = NULL;
  SdpMessageContext *ctx;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  g_object_set (offerer, "addr", OFFERER_ADDR, NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  g_object_set (answerer, "addr", ANSWERER_ADDR, NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  g_object_ref (handler);
  id = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  check_all_is_negotiated (offer, answer);

  gst_sdp_message_free (answer);
  g_object_unref (offerer);
  g_object_unref (answerer);
}

GST_END_TEST
GST_START_TEST (sdp_agent_test_rtp_savpf_negotiation)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  gint id;
  gchar *sdp_str = NULL;
  SdpMessageContext *ctx;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  g_object_set (offerer, "addr", OFFERER_ADDR, NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  g_object_set (answerer, "addr", ANSWERER_ADDR, NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  check_all_is_negotiated (offer, answer);

  gst_sdp_message_free (answer);
  g_object_unref (offerer);
  g_object_unref (answerer);
}

GST_END_TEST;

static gboolean
check_mid_attr (const GstSDPMedia * media, gpointer user_data)
{
  fail_if (gst_sdp_media_get_attribute_val (media, "mid") == NULL);

  return TRUE;
}

static void
test_bundle_group (gboolean expected_bundle)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  gint gid, hid;
  gchar *sdp_str = NULL;
  SdpMessageContext *ctx;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  gid = kms_sdp_agent_create_bundle_group (offerer);
  fail_if (gid < 0);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  hid = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (hid < 0);

  /* Add video to bundle group */
  fail_unless (kms_sdp_agent_add_handler_to_group (offerer, gid, hid));

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  hid = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (hid < 0);

  /* Add audio to bundle group */
  fail_unless (kms_sdp_agent_add_handler_to_group (offerer, gid, hid));

  if (expected_bundle) {
    gid = kms_sdp_agent_create_bundle_group (answerer);
  }

  /* re-use handler for video in answerer */
  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  hid = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (hid < 0);

  if (expected_bundle) {
    fail_unless (kms_sdp_agent_add_handler_to_group (answerer, gid, hid));
  }

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  hid = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (hid < 0);

  if (expected_bundle) {
    fail_unless (kms_sdp_agent_add_handler_to_group (answerer, gid, hid));
  }

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  if (!expected_bundle) {
    fail_if (gst_sdp_message_get_attribute_val (answer, "group") != NULL);
  } else {
    fail_if (gst_sdp_message_get_attribute_val (answer, "group") == NULL);
  }

  sdp_utils_for_each_media (answer, check_mid_attr, NULL);

  gst_sdp_message_free (answer);
  g_object_unref (offerer);
  g_object_unref (answerer);
}

static const gchar *sdp_no_bundle_group_str = "v=0\r\n"
    "o=- 0 0 IN IP4 0.0.0.0\r\n"
    "s=Kurento Media Server\r\n"
    "c=IN IP4 0.0.0.0\r\n"
    "t=0 0\r\n"
    "a=group:LS video0 audio0\r\n"
    "m=video 1 RTP/SAVPF 96 97 100 101\r\n"
    "a=rtpmap:96 H263-1998/90000\r\n"
    "a=rtpmap:97 VP8/90000\r\n"
    "a=rtpmap:100 MP4V-ES/90000\r\n"
    "a=rtpmap:101 H264/90000\r\n"
    "a=rtcp-fb:97 nack\r\n"
    "a=rtcp-fb:97 nack pli\r\n"
    "a=rtcp-fb:97 ccm fir\r\n"
    "a=rtcp-fb:97 goog-remb\r\n"
    "a=rtcp-fb:101 nack\r\n"
    "a=rtcp-fb:101 nack pli\r\n"
    "a=rtcp-fb:101 ccm fir\r\n"
    "a=mid:video0\r\n"
    "m=audio 1 RTP/SAVPF 98 99 0\r\n"
    "a=rtpmap:98 OPUS/48000/2\r\n"
    "a=rtpmap:99 AMR/8000/1\r\n" "a=mid:audio0\r\n";

static const gchar *sdp_bundle_group_str = "v=0\r\n"
    "o=- 0 0 IN IP4 0.0.0.0\r\n"
    "s=Kurento Media Server\r\n"
    "c=IN IP4 0.0.0.0\r\n"
    "t=0 0\r\n"
    "a=group:BUNDLE 1 2 3\r\n"
    "m=audio 30000 RTP/SAVPF 0\r\n"
    "a=mid:1\r\n"
    "m=audio 30002 RTP/SAVPF 8\r\n"
    "a=mid:2\r\n" "m=audio 30004 RTP/SAVPF 3\r\n" "a=mid:3\r\n";

static void
check_no_group_attr (const GstSDPMessage * offer,
    const GstSDPMessage * answer, gpointer data)
{
  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  /* Only BUNDLE group is supported. Fail if any group */
  /* attribute is found in the answer */
  fail_unless (gst_sdp_message_get_attribute_val (answer, "group") == NULL);
}

static void
check_fmt_group_attr (const GstSDPMessage * offer,
    const GstSDPMessage * answer, gpointer data)
{
  const gchar *val;
  gchar **mids;

  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  val = gst_sdp_message_get_attribute_val (answer, "group");

  fail_if (val == NULL);
  mids = g_strsplit (val, " ", 0);

  /* only 1 media is supported for this group */
  fail_if (g_strv_length (mids) > 2);

  fail_if (g_strcmp0 (mids[0], "BUNDLE") != 0);

  /* Only mid 1 is supported */
  fail_if (g_strcmp0 (mids[1], "1") != 0);

  g_strfreev (mids);
}

static void
test_group_with_pattern (const gchar * sdp_pattern,
    CheckSdpNegotiationFunc func, gpointer data)
{
  KmsSdpMediaHandler *handler;
  KmsSdpAgent *answerer;
  gint gid, hid;

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  gid = kms_sdp_agent_create_bundle_group (answerer);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  hid = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (hid < 0);

  fail_unless (kms_sdp_agent_add_handler_to_group (answerer, gid, hid));

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  hid = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (hid < 0);

  fail_unless (kms_sdp_agent_add_handler_to_group (answerer, gid, hid));

  test_sdp_pattern_offer (sdp_pattern, answerer, func, NULL);

  g_object_unref (answerer);
}

static void
check_valid_bundle_answer (GstSDPMessage * answer)
{
  const gchar *val;
  gchar **mids;

  val = gst_sdp_message_get_attribute_val (answer, "group");

  fail_if (val == NULL);
  mids = g_strsplit (val, " ", 0);

  /* BUNDLE group must be empty */
  fail_if (g_strv_length (mids) > 1);

  fail_if (g_strcmp0 (mids[0], "BUNDLE") != 0);

  g_strfreev (mids);
}

static void
sdp_agent_test_bundle_group_without_answerer_handlers ()
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  gint id, gid;
  gchar *sdp_str = NULL;
  SdpMessageContext *ctx;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);
  gid = kms_sdp_agent_create_bundle_group (offerer);
  fail_if (gid < 0);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);
  gid = kms_sdp_agent_create_bundle_group (answerer);
  fail_if (gid < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);
  fail_unless (kms_sdp_agent_add_handler_to_group (offerer, gid, id));

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (id < 0);
  fail_unless (kms_sdp_agent_add_handler_to_group (offerer, gid, id));

  /* Not adding any handler to answerer */

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);

  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  check_valid_bundle_answer (answer);

  gst_sdp_message_free (answer);
  g_object_unref (offerer);
  g_object_unref (answerer);
}

GST_START_TEST (sdp_agent_test_bundle_group)
{
  test_bundle_group (FALSE);
  test_bundle_group (TRUE);
  test_group_with_pattern (sdp_no_bundle_group_str, check_no_group_attr, NULL);
  test_group_with_pattern (sdp_bundle_group_str, check_fmt_group_attr, NULL);
  sdp_agent_test_bundle_group_without_answerer_handlers ();
}

GST_END_TEST static gboolean
is_rtcp_fb_in_media (const GstSDPMessage * msg, const gchar * type)
{
  guint i, len;

  len = gst_sdp_message_medias_len (msg);
  for (i = 0; i < len; i++) {
    const GstSDPMedia *media;
    guint j;

    media = gst_sdp_message_get_media (msg, i);

    for (j = 0;; j++) {
      const gchar *val;
      gchar **opts;

      val = gst_sdp_media_get_attribute_val_n (media, "rtcp-fb", j);

      if (val == NULL) {
        return FALSE;
      }

      opts = g_strsplit (val, " ", 0);

      if (g_strcmp0 (opts[1], type) == 0) {
        g_strfreev (opts);
        return TRUE;
      }

      g_strfreev (opts);
    }
  }

  return FALSE;
}

static void
fb_messages_disable_offer_prop (const gchar * prop)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  gint id;
  gchar *sdp_str = NULL;
  SdpMessageContext *ctx;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  g_object_set (handler, prop, FALSE, NULL);

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (is_rtcp_fb_in_media (offer, prop));

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_if (is_rtcp_fb_in_media (answer, prop));

  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  gst_sdp_message_free (answer);

  g_object_unref (offerer);
  g_object_unref (answerer);
}

static void
fb_messages_disable_answer_prop (const gchar * prop)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  gint id;
  gchar *sdp_str = NULL;
  SdpMessageContext *ctx;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));
  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  g_object_set (handler, prop, FALSE, NULL);

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_if (is_rtcp_fb_in_media (answer, prop));

  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  gst_sdp_message_free (answer);

  g_object_unref (offerer);
  g_object_unref (answerer);
}

GST_START_TEST (sdp_agent_test_fb_messages)
{
  fb_messages_disable_offer_prop ("nack");
  fb_messages_disable_answer_prop ("nack");
  fb_messages_disable_offer_prop ("goog-remb");
  fb_messages_disable_answer_prop ("goog-remb");
}

GST_END_TEST static void
test_handler_offer (KmsSdpAgent * offerer, KmsSdpAgent * answerer,
    CheckSdpNegotiationFunc func, gpointer data)
{
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  gchar *sdp_str = NULL;
  SdpMessageContext *ctx;

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  if (func) {
    func (offer, answer, data);
  }

  gst_sdp_message_free (answer);
}

static gboolean
is_rtcp_mux_in_media (const GstSDPMessage * msg)
{
  guint i, len;

  len = gst_sdp_message_medias_len (msg);
  for (i = 0; i < len; i++) {
    const GstSDPMedia *media = gst_sdp_message_get_media (msg, i);

    if (gst_sdp_media_get_attribute_val (media, "rtcp-mux") != NULL) {
      return TRUE;
    }
  }

  return FALSE;
}

static void
check_rtcp_mux_enabled (const GstSDPMessage * offer,
    const GstSDPMessage * answer, gpointer data)
{
  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  fail_unless (is_rtcp_mux_in_media (offer));
  fail_unless (is_rtcp_mux_in_media (answer));
}

static void
test_rtcp_mux_offer_enabled ()
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  gint id;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  test_handler_offer (offerer, answerer, check_rtcp_mux_enabled, NULL);

  g_object_unref (offerer);
  g_object_unref (answerer);
}

static void
check_rtcp_mux_offer_disabled (const GstSDPMessage * offer,
    const GstSDPMessage * answer, gpointer data)
{
  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  fail_if (is_rtcp_mux_in_media (offer));
  fail_if (is_rtcp_mux_in_media (answer));
}

static void
check_rtcp_mux_answer_disabled (const GstSDPMessage * offer,
    const GstSDPMessage * answer, gpointer data)
{
  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  fail_unless (is_rtcp_mux_in_media (offer));
  fail_if (is_rtcp_mux_in_media (answer));
}

static void
test_rtcp_mux_offer_disabled ()
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  gint id;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  /* Offerer can not manage rtcp-mux */
  g_object_set (handler, "rtcp-mux", FALSE, NULL);

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  test_handler_offer (offerer, answerer, check_rtcp_mux_offer_disabled, NULL);

  g_object_unref (offerer);
  g_object_unref (answerer);
}

static void
test_rtcp_mux_answer_disabled ()
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  gint id;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  /* Answerer can not manage rtcp-mux */
  g_object_set (handler, "rtcp-mux", FALSE, NULL);

  test_handler_offer (offerer, answerer, check_rtcp_mux_answer_disabled, NULL);

  g_object_unref (offerer);
  g_object_unref (answerer);
}

GST_START_TEST (sdp_agent_test_rtcp_mux)
{
  test_rtcp_mux_offer_enabled ();
  test_rtcp_mux_offer_disabled ();
  test_rtcp_mux_answer_disabled ();
}

GST_END_TEST static void
check_multi_m_lines (const GstSDPMessage * offer, const GstSDPMessage * answer,
    gpointer data)
{
  guint i, len;

  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  len = gst_sdp_message_medias_len (offer);

  for (i = 0; i < len; i++) {
    const GstSDPMedia *offer_m, *answer_m;

    offer_m = gst_sdp_message_get_media (offer, i);
    answer_m = gst_sdp_message_get_media (answer, i);

    fail_unless (g_strcmp0 (gst_sdp_media_get_media (offer_m),
            gst_sdp_media_get_media (answer_m)) == 0);

    fail_unless (g_strcmp0 (gst_sdp_media_get_proto (offer_m),
            gst_sdp_media_get_proto (answer_m)) == 0);
  }
}

GST_START_TEST (sdp_agent_test_multi_m_lines)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  gint id;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  /* First video entry */
  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id < 0);

  test_handler_offer (offerer, answerer, check_multi_m_lines, NULL);

  g_object_unref (offerer);
  g_object_unref (answerer);
}

GST_END_TEST
    static const gchar *sdp_offer_unknown_attrs_str = "v=0\r\n"
    "o=- 0 0 IN IP4 0.0.0.0\r\n"
    "s=Kurento Media Server\r\n"
    "c=IN IP4 0.0.0.0\r\n"
    "t=0 0\r\n"
    "m=audio 30000 RTP/AVP 0\r\n"
    "a=mid:1\r\n"
    "a=audiotestattr1:1\r\n"
    "a=audiotestattr2:1\r\n"
    "m=video 30002 RTP/AVP 8\r\n"
    "a=mid:2\r\n" "m=audio 30004 RTP/AVP 3\r\n" "a=mid:3\r\n"
    "a=videotestattr1:1\r\n";

static gboolean
check_media_attrs (const GstSDPMedia * media, gpointer user_data)
{
  if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") == 0) {
    fail_if (gst_sdp_media_get_attribute_val (media, "audiotestattr1"));
    fail_if (gst_sdp_media_get_attribute_val (media, "audiotestattr2"));
  } else if (g_strcmp0 (gst_sdp_media_get_media (media), "video") == 0) {
    fail_if (gst_sdp_media_get_attribute_val (media, "videotestattr1"));
  } else {
    fail ("Media not included in offer");
  }

  return TRUE;
}

static void
check_unsupported_attrs (const GstSDPMessage * offer,
    const GstSDPMessage * answer, gpointer data)
{
  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  sdp_utils_for_each_media (answer, check_media_attrs, NULL);
}

GST_START_TEST (sdp_agent_test_filter_unknown_attr)
{
  KmsSdpAgent *answerer;
  KmsSdpMediaHandler *handler;
  gint id;

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id < 0);

  test_sdp_pattern_offer (sdp_offer_unknown_attrs_str, answerer,
      check_unsupported_attrs, NULL);

  g_object_unref (answerer);
}

GST_END_TEST
    static const gchar *sdp_offer_supported_attrs_str = "v=0\r\n"
    "o=- 0 0 IN IP4 0.0.0.0\r\n"
    "s=Kurento Media Server\r\n"
    "c=IN IP4 0.0.0.0\r\n"
    "t=0 0\r\n"
    "m=video 1 RTP/AVPF 96 97 100 101\r\n"
    "a=rtpmap:96 H265/90000\r\n"
    "a=rtpmap:97 VP8/90000\r\n"
    "a=rtpmap:100 MP4V-ES/90000\r\n"
    "a=rtpmap:101 H264/90000\r\n"
    "a=rtcp-fb:97 nack\r\n"
    "a=rtcp-fb:97 nack pli\r\n"
    "a=rtcp-fb:97 goog-remb\r\n"
    "a=rtcp-fb:97 ccm fir\r\n"
    "a=rtcp-fb:101 nack\r\n"
    "a=rtcp-fb:101 nack pli\r\n"
    "a=rtcp-fb:101 ccm fir\r\n"
    "a=fmtp:96 minptime=10; useinbandfec=1\r\n"
    "a=fmtp:97 minptime=10; useinbandfec=1\r\n"
    "a=fmtp:111 minptime=10; useinbandfec=1\r\n"
    "a=rtcp-mux\r\n"
    "a=quality:10\r\n" "a=maxptime:60\r\n" "a=setup:actpass\r\n";

static gboolean
check_supported_media_attrs (const GstSDPMedia * media, gpointer user_data)
{
  guint i;

  for (i = 0;; i++) {
    const gchar *val;

    val = gst_sdp_media_get_attribute_val_n (media, "fmtp", i);
    if (val == NULL) {
      /* no more fmtp attributes */
      break;
    }

    /* Only fmtp:97 must be in answer */
    fail_unless (g_str_has_prefix (val, "97"));
  }

  /* Only one fmtp lines should have been processed */
  fail_unless (i == 1);

  fail_if (gst_sdp_media_get_attribute_val (media, "quality") == NULL);
  fail_if (gst_sdp_media_get_attribute_val (media, "maxptime") == NULL);

  return TRUE;
}

static void
check_supported_attrs (const GstSDPMessage * offer,
    const GstSDPMessage * answer, gpointer data)
{
  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  sdp_utils_for_each_media (answer, check_supported_media_attrs, NULL);
}

GST_START_TEST (sdp_agent_test_supported_attrs)
{
  KmsSdpAgent *answerer;
  KmsSdpMediaHandler *handler;
  gint id;

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  test_sdp_pattern_offer (sdp_offer_supported_attrs_str, answerer,
      check_supported_attrs, NULL);

  g_object_unref (answerer);
}

GST_END_TEST typedef struct _BandwitdthData
{
  gboolean offered;
  guint offer;
  gboolean answered;
  guint answer;
} BandwitdthData;

static void
check_bandwidth_medias_attrs (const GstSDPMessage * offer,
    const GstSDPMessage * answer, gpointer data)
{
  const GstSDPBandwidth *offer_bw, *answer_bw;
  const GstSDPMedia *offered, *answered;
  BandwitdthData *bw = (BandwitdthData *) data;

  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  offered = gst_sdp_message_get_media (offer, 0);
  fail_if (offered == NULL);
  offer_bw = gst_sdp_media_get_bandwidth (offered, 0);

  if (bw->offered) {
    fail_if (offer_bw == NULL);
    fail_unless (offer_bw->bandwidth == bw->offer);
  } else {
    fail_if (offer_bw != NULL);
  }

  answered = gst_sdp_message_get_media (answer, 0);
  fail_if (answered == NULL);
  answer_bw = gst_sdp_media_get_bandwidth (answered, 0);

  if (bw->answered) {
    fail_if (answer_bw == NULL);
    fail_unless (answer_bw->bandwidth == bw->answer);
  } else {
    fail_if (answer_bw != NULL);
  }

}

static void
test_agents_bandwidth (gboolean offer, guint offered, gboolean answer,
    guint answered)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler1, *handler2;
  BandwitdthData data;
  gint id;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  handler1 = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler1 == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler1), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler1);
  fail_if (id < 0);

  if (offer) {
    kms_sdp_media_handler_add_bandwidth (handler1, "AS", offered);
  }

  handler2 = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler2 == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler2), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  if (answer) {
    kms_sdp_media_handler_add_bandwidth (handler2, "AS", answered);
  }

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler2);
  fail_if (id < 0);

  data.offered = offer;
  data.answered = answer;
  data.offer = offered;
  data.answer = answered;

  test_handler_offer (offerer, answerer, check_bandwidth_medias_attrs, &data);

  g_object_unref (answerer);
  g_object_unref (offerer);
}

GST_START_TEST (sdp_agent_test_bandwidtth_attrs)
{
  test_agents_bandwidth (TRUE, 120, TRUE, 15);
  test_agents_bandwidth (FALSE, 0, TRUE, 16);
  test_agents_bandwidth (TRUE, 16, FALSE, 0);
  test_agents_bandwidth (FALSE, 0, FALSE, 0);
}

GST_END_TEST;

static void
check_extmap_attrs_add_twice ()
{
  KmsSdpRtpAvpMediaHandler *handler;
  GError *err = NULL;
  gboolean ret;

  handler = kms_sdp_rtp_avp_media_handler_new ();
  fail_if (handler == NULL);

  ret = kms_sdp_rtp_avp_media_handler_add_extmap (handler, 1, "URI-A", &err);
  fail_if (ret == FALSE);
  fail_if (err != NULL);

  ret = kms_sdp_rtp_avp_media_handler_add_extmap (handler, 2, "URI-B", &err);
  fail_if (ret == FALSE);
  fail_if (err != NULL);

  ret = kms_sdp_rtp_avp_media_handler_add_extmap (handler, 1, "URI-A", &err);
  fail_if (ret == TRUE);
  fail_if (err == NULL);
  g_error_free (err);

  g_object_unref (handler);
}

static void
check_extmap_attrs_into_offer ()
{
  KmsSdpAgent *agent;
  KmsSdpMediaHandler *handler1;
  gchar *sdp_str = NULL;
  GstSDPMessage *offer;
  const GstSDPMedia *media;
  const gchar *extmap;
  GError *err = NULL;
  gint id;

  agent = kms_sdp_agent_new ();
  fail_if (agent == NULL);

  handler1 = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler1 == NULL);

  kms_sdp_rtp_avp_media_handler_add_extmap (KMS_SDP_RTP_AVP_MEDIA_HANDLER
      (handler1), 1, "URI-A", &err);
  fail_if (err != NULL);

  id = kms_sdp_agent_add_proto_handler (agent, "video", handler1);
  fail_if (id < 0);

  offer = kms_sdp_agent_create_offer (agent, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  media = gst_sdp_message_get_media (offer, 0);
  extmap = gst_sdp_media_get_attribute_val (media, "extmap");
  fail_if (g_strcmp0 (extmap, "1 URI-A") != 0);

  gst_sdp_message_free (offer);
  g_object_unref (agent);
}

static void
check_extmap_attrs_negotiation ()
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler1, *handler2;
  gchar *sdp_str = NULL;
  GstSDPMessage *offer, *answer;
  const GstSDPMedia *media;
  SdpMessageContext *ctx;
  const gchar *extmap;
  GError *err = NULL;
  gint id;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  handler1 = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler1 == NULL);

  kms_sdp_rtp_avp_media_handler_add_extmap (KMS_SDP_RTP_AVP_MEDIA_HANDLER
      (handler1), 1, "URI-A", &err);
  fail_if (err != NULL);

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler1);
  fail_if (id < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler2 = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler2 == NULL);

  kms_sdp_rtp_avp_media_handler_add_extmap (KMS_SDP_RTP_AVP_MEDIA_HANDLER
      (handler2), 2, "URI-A", &err);
  fail_if (err != NULL);

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler2);
  fail_if (id < 0);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  kms_sdp_message_context_unref (ctx);
  fail_if (err != NULL);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  media = gst_sdp_message_get_media (answer, 0);
  extmap = gst_sdp_media_get_attribute_val (media, "extmap");
  fail_if (g_strcmp0 (extmap, "1 URI-A") != 0);

  gst_sdp_message_free (answer);
  g_object_unref (offerer);
  g_object_unref (answerer);
}

GST_START_TEST (sdp_agent_test_extmap_attrs)
{
  check_extmap_attrs_add_twice ();
  check_extmap_attrs_into_offer ();
  check_extmap_attrs_negotiation ();
}

GST_END_TEST;

static void
test_sdp_dynamic_pts (KmsSdpRtpAvpMediaHandler * handler)
{
  KmsSdpPayloadManager *ptmanager;
  GError *err = NULL;

  /* Try to assign a dynamic pt to an static codec. Expected to fail */
  fail_if (kms_sdp_rtp_avp_media_handler_add_video_codec (handler,
          "H263-1998/90000", &err));
  GST_DEBUG ("Expected error: %s", err->message);
  g_clear_error (&err);

  ptmanager = kms_sdp_payload_manager_new ();
  kms_sdp_rtp_avp_media_handler_use_payload_manager (handler,
      KMS_I_SDP_PAYLOAD_MANAGER (ptmanager), &err);

  fail_unless (kms_sdp_rtp_avp_media_handler_add_video_codec (handler,
          "VP8/90000", &err));
  fail_unless (kms_sdp_rtp_avp_media_handler_add_video_codec (handler,
          "MP4V-ES/90000", &err));

  /* Try to add and already added codec. Expected to fail */
  fail_if (kms_sdp_rtp_avp_media_handler_add_video_codec (handler,
          "VP8/90000", &err));
  GST_DEBUG ("Expected error: %s", err->message);
  g_clear_error (&err);
}

GST_START_TEST (sdp_agent_test_dynamic_pts)
{
  KmsSdpMediaHandler *handler;
  KmsSdpAgent *agent;
  gint id;

  agent = kms_sdp_agent_new ();
  fail_if (agent == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  id = kms_sdp_agent_add_proto_handler (agent, "audio", handler);
  fail_if (id < 0);

  test_sdp_dynamic_pts (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler));

  g_object_unref (agent);
}

GST_END_TEST;

static const gchar *sdp_offer_str1 = "v=0\r\n"
    "o=- 0 0 IN IP4 0.0.0.0\r\n"
    "s=TestSession\r\n"
    "c=IN IP4 0.0.0.0\r\n"
    "t=2873397496 2873404696\r\n"
    "m=audio 9 RTP/AVP 0\r\n" "a=rtpmap:0 PCMU/8000\r\n" "a=sendonly\r\n";

static const gchar *sdp_offer_str2 = "v=0\r\n"
    "o=- 0 0 IN IP4 0.0.0.0\r\n"
    "s=TestSession\r\n"
    "c=IN IP4 0.0.0.0\r\n"
    "t=2873397496 2873404696\r\n"
    "m=audio 9 RTP/AVP 0\r\n" "a=rtpmap:0 PCMU/8000/1\r\n" "a=sendonly\r\n";

static const gchar *sdp_offer_str3 = "v=0\r\n"
    "o=- 0 0 IN IP4 0.0.0.0\r\n"
    "s=TestSession\r\n"
    "c=IN IP4 0.0.0.0\r\n"
    "t=2873397496 2873404696\r\n" "m=audio 9 RTP/AVP 0\r\n" "a=sendonly\r\n";

static void
check_pcmu_without_number_of_channels (const GstSDPMessage * offer,
    const GstSDPMessage * answer, gpointer data)
{
  const GstSDPMedia *media;

  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  fail_if (gst_sdp_message_medias_len (answer) != 1);

  media = gst_sdp_message_get_media (answer, 0);

  fail_if (gst_sdp_media_get_port (media) == 0);
  fail_if (gst_sdp_media_formats_len (media) != 1);

  fail_unless (g_strcmp0 (gst_sdp_media_get_format (media, 0), "0") == 0);
}

static void
check_optional_number_of_channels (const gchar * offer, const gchar * codec)
{
  GError *err = NULL;
  KmsSdpAgent *answerer;
  KmsSdpMediaHandler *handler;
  gint id;

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  fail_unless (kms_sdp_rtp_avp_media_handler_add_audio_codec
      (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), codec, &err));

  id = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id < 0);

  test_sdp_pattern_offer (offer, answerer,
      check_pcmu_without_number_of_channels, NULL);

  g_object_unref (answerer);
}

GST_START_TEST (sdp_agent_test_optional_enc_parameters)
{
  check_optional_number_of_channels (sdp_offer_str1, "PCMU/8000/1");
  check_optional_number_of_channels (sdp_offer_str1, "PCMU/8000");
  check_optional_number_of_channels (sdp_offer_str2, "PCMU/8000/1");
  check_optional_number_of_channels (sdp_offer_str2, "PCMU/8000");
  check_optional_number_of_channels (sdp_offer_str3, "PCMU/8000/1");
  check_optional_number_of_channels (sdp_offer_str3, "PCMU/8000");
}

GST_END_TEST;

static gchar *offer_sdp_check_1 = "v=0\r\n"
    "o=- 123456 0 IN IP4 127.0.0.1\r\n"
    "s=TestSession\r\n"
    "c=IN IP4 127.0.0.1\r\n"
    "t=0 0\r\n"
    "m=video 3434 RTP/AVP 96 97 99\r\n"
    "a=rtpmap:96 MP4V-ES/90000\r\n"
    "a=rtpmap:97 H263-1998/90000\r\n"
    "a=rtpmap:99 H264/90000\r\n"
    "a=sendrecv\r\n"
    "m=video 6565 RTP/AVP 98\r\n"
    "a=rtpmap:98 VP8/90000\r\n"
    "a=sendrecv\r\n" "m=audio 4545 RTP/AVP 14\r\n" "a=sendrecv\r\n"
    "m=audio 1010 TCP 14\r\n";

static void
intersection_check_1 (const GstSDPMessage * offer,
    const GstSDPMessage * answer, gpointer data)
{
  const GstSDPMedia *media;

  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  /***************************/
  /* Check first media entry */
  /***************************/
  media = gst_sdp_message_get_media (answer, 0);
  fail_unless (media != NULL);

  /* Video must be supported */
  fail_if (gst_sdp_media_get_port (media) == 0);

  /* Only format 96 and 99 are supported */
  fail_unless (gst_sdp_media_formats_len (media) != 1);
  fail_unless (g_strcmp0 (gst_sdp_media_get_format (media, 0), "96") == 0);
  fail_unless (g_strcmp0 (gst_sdp_media_get_format (media, 1), "99") == 0);

  /* Check that rtpmap attributes */
  fail_if (sdp_utils_sdp_media_get_rtpmap (media, "96") == NULL);
  fail_if (sdp_utils_sdp_media_get_rtpmap (media, "97") != NULL);
  fail_if (sdp_utils_sdp_media_get_rtpmap (media, "99") == NULL);

  /****************************/
  /* Check second media entry */
  /****************************/
  media = gst_sdp_message_get_media (answer, 1);
  fail_unless (media != NULL);

  /* Video should not be supported */
  fail_unless (gst_sdp_media_get_port (media) == 0);
  fail_if (sdp_utils_sdp_media_get_rtpmap (media, "98") != NULL);

  /***************************/
  /* Check third media entry */
  /***************************/
  media = gst_sdp_message_get_media (answer, 2);
  fail_unless (media != NULL);

  /* Audio should be supported */
  fail_if (gst_sdp_media_get_port (media) == 0);

  /* Only format 14 is supported */
  fail_unless (gst_sdp_media_formats_len (media) != 0);
  fail_unless (g_strcmp0 (gst_sdp_media_get_format (media, 0), "14") == 0);

  /****************************/
  /* Check fourth media entry */
  /****************************/
  media = gst_sdp_message_get_media (answer, 3);
  fail_unless (media != NULL);

  /* Audio should not be supported */
  fail_unless (gst_sdp_media_get_port (media) == 0);
}

static void
regression_test_1 ()
{
  KmsSdpAgent *answerer;
  KmsSdpMediaHandler *handler;
  gint id;

  gchar *audio_codecs[] = {
    "MPA/90000"
  };

  gchar *video_codecs[] = {
    "MP4V-ES/90000",
    "H264/90000"
  };

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  /* Create video handler */
  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), NULL, 0,
      video_codecs, 2);

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  /* Create handler for audio */
  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs, 1,
      NULL, 0);

  id = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id < 0);

  /* Check response */
  test_sdp_pattern_offer (offer_sdp_check_1, answerer,
      intersection_check_1, NULL);

  g_object_unref (answerer);
}

static gchar *offer_sdp_check_2 = "v=0\r\n"
    "o=- 0 0 IN IP4 127.0.0.1\r\n"
    "s=-\r\n"
    "t=0 0\r\n"
    "m=video 1 RTP/SAVPF 103 100\r\n"
    "c=IN IP4 0.0.0.0\r\n"
    "a=sendrecv\r\n" "a=rtpmap:103 H264/90000\r\n" "a=rtpmap:100 VP8/90000\r\n";

static void
intersection_check_2 (const GstSDPMessage * offer,
    const GstSDPMessage * answer, gpointer data)
{
  const GstSDPMedia *media;

  /* Same number of medias must be in answer */
  fail_if (gst_sdp_message_medias_len (offer) !=
      gst_sdp_message_medias_len (answer));

  media = gst_sdp_message_get_media (answer, 0);
  fail_unless (media != NULL);

  /* Video must be supported */
  fail_if (gst_sdp_media_get_port (media) == 0);

  /* Only format 100 is supported */
  fail_unless (gst_sdp_media_formats_len (media) != 0);
  fail_unless (g_strcmp0 (gst_sdp_media_get_format (media, 0), "100") == 0);

  /* Check that rtpmap attributes are present */
  fail_if (sdp_utils_sdp_media_get_rtpmap (media, "100") == NULL);
}

static void
regression_test_2 ()
{
  KmsSdpAgent *answerer;
  KmsSdpMediaHandler *handler;
  gint id;

  gchar *video_codecs[] = {
    "VP8/90000"
  };

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  /* Create video handler */
  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), NULL, 0,
      video_codecs, 1);

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  /* Check response */
  test_sdp_pattern_offer (offer_sdp_check_2, answerer,
      intersection_check_2, NULL);

  g_object_unref (answerer);
}

GST_START_TEST (sdp_agent_regression_tests)
{
  regression_test_1 ();
  regression_test_2 ();
}

GST_END_TEST;

GST_START_TEST (sdp_agent_avp_avpf_negotiation)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  gint id;
  gchar *sdp_str = NULL;
  SdpMessageContext *ctx;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  g_object_set (offerer, "addr", OFFERER_ADDR, NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  g_object_set (answerer, "addr", ANSWERER_ADDR, NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  check_all_is_negotiated (offer, answer);

  gst_sdp_message_free (answer);
  g_object_unref (offerer);
  g_object_unref (answerer);
}

GST_END_TEST static void
check_payloads (GstSDPMessage * answer, gint fec_payload, gint red_payload)
{
  const GstSDPMedia *media = NULL;
  const gchar *attr;
  gchar *payload;
  guint i, l;

  l = gst_sdp_message_medias_len (answer);

  fail_if (l == 0);

  for (i = 0; i < l; i++) {
    media = gst_sdp_message_get_media (answer, i);

    if (g_strcmp0 (gst_sdp_media_get_media (media), "video") == 0) {
      break;
    }
  }

  fail_if (media == NULL ||
      g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  /* Check payloads */
  payload = g_strdup_printf ("%d", fec_payload);
  attr = sdp_utils_get_attr_map_value (media, "rtpmap", payload);
  g_free (payload);

  fail_if (attr == NULL);

  payload = g_strdup_printf ("%d", red_payload);
  attr = sdp_utils_get_attr_map_value (media, "rtpmap", payload);
  g_free (payload);

  fail_if (attr == NULL);

  payload = g_strdup_printf ("%d", red_payload);
  attr = sdp_utils_get_attr_map_value (media, "fmtp", payload);
  g_free (payload);

  fail_if (attr == NULL);
}

GST_START_TEST (sdp_agent_avp_generic_payload_negotiation)
{
  KmsSdpPayloadManager *ptmanager;
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  gint id, fec_payload, red_payload;
  gchar *sdp_str = NULL;
  SdpMessageContext *ctx;

  ptmanager = kms_sdp_payload_manager_new ();

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  g_object_set (offerer, "addr", OFFERER_ADDR, NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  g_object_set (answerer, "addr", ANSWERER_ADDR, NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  kms_sdp_rtp_avp_media_handler_use_payload_manager
      (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler),
      KMS_I_SDP_PAYLOAD_MANAGER (ptmanager), &err);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  fec_payload = kms_sdp_rtp_avp_media_handler_add_generic_video_payload
      (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), "ulpfec/90000", &err);
  fail_if (err != NULL);

  red_payload = kms_sdp_rtp_avp_media_handler_add_generic_video_payload
      (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), "red/8000", &err);
  fail_if (err != NULL);

  kms_sdp_rtp_avp_media_handler_add_fmtp (KMS_SDP_RTP_AVP_MEDIA_HANDLER
      (handler), red_payload, "0/5/100", &err);
  fail_if (err != NULL);

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  ptmanager = kms_sdp_payload_manager_new ();

  kms_sdp_rtp_avp_media_handler_use_payload_manager
      (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler),
      KMS_I_SDP_PAYLOAD_MANAGER (ptmanager), &err);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  kms_sdp_rtp_avp_media_handler_add_generic_video_payload
      (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), "ulpfec/90000", &err);
  fail_if (err != NULL);

  kms_sdp_rtp_avp_media_handler_add_generic_video_payload
      (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), "red/8000", &err);
  fail_if (err != NULL);

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  check_all_is_negotiated (offer, answer);
  check_payloads (answer, fec_payload, red_payload);

  gst_sdp_message_free (answer);

  g_object_unref (offerer);
  g_object_unref (answerer);
}

GST_END_TEST
    static const gchar *sdp_udp_tls_rtp_savpf_offer_str = "v=0\r\n"
    "o=- 0 0 IN IP4 0.0.0.0\r\n"
    "s=TestSession\r\n"
    "c=IN IP4 0.0.0.0\r\n"
    "t=2873397496 2873404696\r\n"
    "m=audio 9 UDP/TLS/RTP/SAVPF 0 96 97\r\n" "a=rtpmap:0 PCMU/8000\r\n"
    "a=rtpmap:96 opus/48000/2\r\n" "a=rtpmap:97 AMR/8000/1\r\n" "a=rtcp-mux\r\n"
    "m=video 9 UDP/TLS/RTP/SAVPF 98 99 1001 101\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n" "a=rtpmap:99 VP8/90000\r\n"
    "a=rtpmap:100 MP4V-ES/90000\r\n" "a=rtpmap:101 H264/90000\r\n"
    "a=rtcp-mux\r\n";

GST_START_TEST (sdp_agent_udp_tls_rtp_savpf_negotiation)
{
  KmsSdpAgent *answerer;
  KmsSdpMediaHandler *handler;
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  gint id;
  gchar *sdp_str = NULL;
  SdpMessageContext *ctx;

  fail_unless (gst_sdp_message_new (&offer) == GST_SDP_OK);
  fail_unless (gst_sdp_message_parse_buffer ((const guint8 *)
          sdp_udp_tls_rtp_savpf_offer_str, -1, offer) == GST_SDP_OK);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  g_object_set (answerer, "addr", ANSWERER_ADDR, NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id < 0);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  check_all_is_negotiated (offer, answer);

  gst_sdp_message_free (answer);
  g_object_unref (answerer);
}

GST_END_TEST;

#define ANSWER_KEY "test:answer_key"

static GArray *
on_offer_keys_cb (KmsSdpSdesExt * ext, gpointer data)
{
  GValue v1 = G_VALUE_INIT;
  GValue v2 = G_VALUE_INIT;
  GValue v3 = G_VALUE_INIT;
  guint mki, len;
  GArray *keys;

  keys = g_array_sized_new (FALSE, FALSE, sizeof (GValue), 3);

  /* Sets a function to clear an element of array */
  g_array_set_clear_func (keys, (GDestroyNotify) g_value_unset);

  fail_if (!kms_sdp_sdes_ext_create_key (1, "1abcdefgh",
          KMS_SDES_EXT_AES_256_CM_HMAC_SHA1_32, &v1));
  g_array_append_val (keys, v1);

  mki = 554;
  len = 4;

  fail_if (!kms_sdp_sdes_ext_create_key_detailed (2, "2abcdefgh",
          KMS_SDES_EXT_AES_256_CM_HMAC_SHA1_80, NULL, &mki, &len, &v2, NULL));
  g_array_append_val (keys, v2);

  fail_if (!kms_sdp_sdes_ext_create_key_detailed (3, "3abcdefgh",
          KMS_SDES_EXT_AES_CM_128_HMAC_SHA1_80, "2^20", NULL, NULL, &v3, NULL));
  g_array_append_val (keys, v3);

  return keys;
}

static gboolean
on_answer_keys_cb (KmsSdpSdesExt * ext, const GArray * keys, GValue * key)
{
  GValue *offer_val;
  SrtpCryptoSuite crypto;
  guint tag;

  if (keys->len == 0) {
    fail ("No key provided");
    return FALSE;
  }

  /* We only will support the first key */
  offer_val = &g_array_index (keys, GValue, 0);

  if (offer_val == NULL) {
    fail ("No offered keys");
    return FALSE;
  }

  if (!kms_sdp_sdes_ext_get_parameters_from_key (offer_val, KMS_SDES_TAG_FIELD,
          G_TYPE_UINT, &tag, KMS_SDES_CRYPTO, G_TYPE_UINT, &crypto, NULL)) {
    fail ("Invalid key offered");
    return FALSE;
  }

  fail_if (!kms_sdp_sdes_ext_create_key_detailed (tag, ANSWER_KEY, crypto, NULL,
          NULL, NULL, key, NULL));

  return TRUE;
}

static void
on_selected_key_cb (KmsSdpSdesExt * ext, const GValue * key)
{
  SrtpCryptoSuite crypto;
  gchar *str_key;
  guint tag;

  fail_if (!kms_sdp_sdes_ext_get_parameters_from_key (key, KMS_SDES_TAG_FIELD,
          G_TYPE_UINT, &tag, KMS_SDES_CRYPTO, G_TYPE_UINT, &crypto,
          KMS_SDES_KEY_FIELD, G_TYPE_STRING, &str_key, NULL));

  /* only the first one must be selected */
  fail_if (tag != 1);
  fail_if (crypto != KMS_SDES_EXT_AES_256_CM_HMAC_SHA1_32);
  fail_if (g_strcmp0 (str_key, ANSWER_KEY) != 0);

  g_free (str_key);
}

GST_START_TEST (sdp_agent_sdes_negotiation)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  SdpMessageContext *ctx;
  GstSDPMessage *offer, *answer;
  KmsSdpSdesExt *ext1, *ext2;
  GError *err = NULL;
  gchar *sdp_str = NULL;
  const GstSDPMedia *media;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  ext1 = kms_sdp_sdes_ext_new ();
  fail_if (!kms_sdp_media_handler_add_media_extension (handler,
          KMS_I_SDP_MEDIA_EXTENSION (ext1)));

  g_signal_connect (ext1, "on-offer-keys", G_CALLBACK (on_offer_keys_cb), NULL);
  g_signal_connect (ext1, "on-selected-key", G_CALLBACK (on_selected_key_cb),
      NULL);

  fail_if (kms_sdp_agent_add_proto_handler (offerer, "video", handler) < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  ext2 = kms_sdp_sdes_ext_new ();
  fail_if (!kms_sdp_media_handler_add_media_extension (handler,
          KMS_I_SDP_MEDIA_EXTENSION (ext2)));

  g_signal_connect (ext2, "on-answer-keys", G_CALLBACK (on_answer_keys_cb),
      NULL);

  fail_if (kms_sdp_agent_add_proto_handler (answerer, "video", handler) < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_if (gst_sdp_message_medias_len (answer) != 1);
  media = gst_sdp_message_get_media (answer, 0);
  fail_if (gst_sdp_media_get_port (media) == 0);

  fail_if (!kms_i_sdp_media_extension_process_answer_attributes
      (KMS_I_SDP_MEDIA_EXTENSION (ext1), media, &err));

  gst_sdp_message_free (answer);

  g_object_unref (offerer);
  g_object_unref (answerer);
}

GST_END_TEST;

static gchar *sdp_first_media_inactive = "v=0\r\n"
    "o=- 123456 0 IN IP4 127.0.0.1\r\n"
    "s=Kurento Media Server\r\n"
    "c=IN IP4 0.0.0.0\r\n"
    "t=0 0\r\n"
    "a=group:BUNDLE video0\r\n"
    "m=audio 0 RTP/SAVPF 0\r\n"
    "a=inactive\r\n"
    "a=mid:audio0\r\n"
    "m=video 1 RTP/SAVPF 97\r\n" "a=rtpmap:97 VP8/90000\r\n" "a=mid:video0\r\n";

GST_START_TEST (sdp_context_from_first_media_inactive)
{
  GstSDPMessage *sdp;
  SdpMessageContext *ctx;
  const GSList *item;
  guint i;
  GError *err = NULL;

  fail_unless (gst_sdp_message_new (&sdp) == GST_SDP_OK);
  fail_unless (gst_sdp_message_parse_buffer ((const guint8 *)
          sdp_first_media_inactive, -1, sdp) == GST_SDP_OK);
  ctx = kms_sdp_message_context_new_from_sdp (sdp, &err);
  gst_sdp_message_free (sdp);
  fail_if (err != NULL);

  i = 0;
  item = kms_sdp_message_context_get_medias (ctx);
  for (; item != NULL; item = g_slist_next (item)) {
    SdpMediaConfig *mconf = item->data;

    if (kms_sdp_media_config_is_inactive (mconf)) {
      fail_if (i != 0);
    }

    i++;
  }

  kms_sdp_message_context_unref (ctx);
}

GST_END_TEST;

static gboolean
check_if_media_is_removed (GstSDPMessage * msg, guint index)
{
  const GstSDPMedia *media;

  fail_if (index > gst_sdp_message_medias_len (msg));

  media = gst_sdp_message_get_media (msg, index);

  fail_if (media == NULL);

  return gst_sdp_media_get_port (media) == 0;
}

GST_START_TEST (sdp_agent_renegotiation_offer_new_media)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  gint id;
  gchar *sdp_str = NULL, *session;
  const GstSDPOrigin *o;
  guint64 v1, v2, v3;
  const GstSDPMedia *media;
  SdpMessageContext *ctx;

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  g_object_set (answerer, "addr", ANSWERER_ADDR, NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id < 0);

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  g_object_set (offerer, "addr", OFFERER_ADDR, NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (id < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  o = gst_sdp_message_get_origin (offer);
  v1 = g_ascii_strtoull (o->sess_version, NULL, 10);
  session = g_strdup (o->sess_id);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_unless (gst_sdp_message_medias_len (offer) == 2);

  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (check_if_media_is_removed (offer, 1));

  /* We set our local description for further renegotiations */
  fail_if (!kms_sdp_agent_set_local_description (offerer, offer, &err));
  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));

  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (offerer, answer, &err));
  fail_if (!kms_sdp_agent_set_local_description (answerer, answer, &err));

  GST_DEBUG ("Add a new media for data channels");
  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_sctp_media_handler_new ());
  fail_if (handler == NULL);

  id = kms_sdp_agent_add_proto_handler (offerer, "application", handler);
  fail_if (id < 0);

  /* Make a new offer */
  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  o = gst_sdp_message_get_origin (offer);
  v2 = g_ascii_strtoull (o->sess_version, NULL, 10);

  GST_DEBUG ("New Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_unless (gst_sdp_message_medias_len (offer) == 3);

  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (check_if_media_is_removed (offer, 1));
  fail_if (check_if_media_is_removed (offer, 2));

  /* This should be a new version of this session */
  fail_unless (g_strcmp0 (session, o->sess_id) == 0 && v1 + 1 == v2);

  /* Set new local description */
  kms_sdp_agent_set_local_description (offerer, offer, &err);
  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));

  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (offerer, answer, &err));
  fail_if (!kms_sdp_agent_set_local_description (answerer, answer, &err));

  /* Generate a new offer */
  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("New Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  o = gst_sdp_message_get_origin (offer);
  v3 = g_ascii_strtoull (o->sess_version, NULL, 10);

  fail_unless (gst_sdp_message_medias_len (offer) == 3);

  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (check_if_media_is_removed (offer, 1));
  fail_if (!check_if_media_is_removed (offer, 2));

  /* sdp is the same so version should not have changed */
  fail_unless (g_strcmp0 (session, o->sess_id) == 0 && v2 == v3);

  kms_sdp_agent_set_local_description (offerer, offer, &err);

  gst_sdp_message_free (offer);
  g_object_unref (offerer);
  g_object_unref (answerer);
  g_free (session);
}

GST_END_TEST;

GST_START_TEST (sdp_agent_renegotiation_offer_remove_media)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  gint id;
  gchar *sdp_str = NULL, *session;
  const GstSDPOrigin *o;
  guint64 v1, v2, v3, v4, v5;
  const GstSDPMedia *media;
  SdpMessageContext *ctx;

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  g_object_set (answerer, "addr", OFFERER_ADDR, NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_sctp_media_handler_new ());
  fail_if (handler == NULL);

  fail_if (kms_sdp_agent_add_proto_handler (answerer, "application",
          handler) < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  g_object_set (offerer, "addr", OFFERER_ADDR, NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (id < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_sctp_media_handler_new ());
  fail_if (handler == NULL);

  fail_if (kms_sdp_agent_add_proto_handler (offerer, "application",
          handler) < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  o = gst_sdp_message_get_origin (offer);
  v1 = g_ascii_strtoull (o->sess_version, NULL, 10);
  session = g_strdup (o->sess_id);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_unless (gst_sdp_message_medias_len (offer) == 3);

  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (check_if_media_is_removed (offer, 1));
  fail_if (check_if_media_is_removed (offer, 2));

  /* We set our local description for further renegotiations */
  kms_sdp_agent_set_local_description (offerer, offer, &err);
  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));

  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (offerer, answer, &err));
  fail_if (!kms_sdp_agent_set_local_description (answerer, answer, &err));

  GST_DEBUG ("Remove audio handler");

  fail_if (!kms_sdp_agent_remove_proto_handler (offerer, id));

  /* Make a new offer */
  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  o = gst_sdp_message_get_origin (offer);
  v2 = g_ascii_strtoull (o->sess_version, NULL, 10);

  GST_DEBUG ("New Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_unless (gst_sdp_message_medias_len (offer) == 3);

  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (!check_if_media_is_removed (offer, 1));
  fail_if (check_if_media_is_removed (offer, 2));

  /* This should be a new version of this session */
  fail_unless (g_strcmp0 (session, o->sess_id) == 0 && v1 + 1 == v2);

  /* Set new local description */
  kms_sdp_agent_set_local_description (offerer, offer, &err);
  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));

  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (offerer, answer, &err));
  fail_if (!kms_sdp_agent_set_local_description (answerer, answer, &err));

  /* Generate a new offer */
  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Next Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  o = gst_sdp_message_get_origin (offer);
  v3 = g_ascii_strtoull (o->sess_version, NULL, 10);

  /* sdp is the same so version should not have changed */
  fail_unless (g_strcmp0 (session, o->sess_id) == 0 && v2 == v3);
  fail_unless (gst_sdp_message_medias_len (offer) == 3);

  /* Check that medias are orderer and supported */
  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (!check_if_media_is_removed (offer, 1));
  fail_if (check_if_media_is_removed (offer, 2));

  /* Set new local description */
  kms_sdp_agent_set_local_description (offerer, offer, &err);
  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));

  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (offerer, answer, &err));
  fail_if (!kms_sdp_agent_set_local_description (answerer, answer, &err));

  GST_DEBUG ("Add a new video media. This should fill the removed slot");

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Next Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  o = gst_sdp_message_get_origin (offer);
  v4 = g_ascii_strtoull (o->sess_version, NULL, 10);

  /* sdp is should have changed */
  fail_unless (g_strcmp0 (session, o->sess_id) == 0 && v3 + 1 == v4);
  fail_unless (gst_sdp_message_medias_len (offer) == 3);

  /* Check that medias are orderer and supported */
  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (check_if_media_is_removed (offer, 1));
  fail_if (check_if_media_is_removed (offer, 2));

  kms_sdp_agent_set_local_description (offerer, offer, &err);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));

  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (offerer, answer, &err));
  fail_if (!kms_sdp_agent_set_local_description (answerer, answer, &err));

  GST_DEBUG ("Add a new audio handler, this should go to the last position");

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (id < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Last Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  kms_sdp_agent_set_local_description (offerer, offer, &err);

  o = gst_sdp_message_get_origin (offer);
  v5 = g_ascii_strtoull (o->sess_version, NULL, 10);

  /* sdp is should have changed */
  fail_unless (g_strcmp0 (session, o->sess_id) == 0 && v4 + 1 == v5);
  fail_unless (gst_sdp_message_medias_len (offer) == 4);

  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  media = gst_sdp_message_get_media (offer, 3);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (check_if_media_is_removed (offer, 1));
  fail_if (check_if_media_is_removed (offer, 2));
  fail_if (check_if_media_is_removed (offer, 3));

  gst_sdp_message_free (offer);
  g_object_unref (offerer);
  g_object_unref (answerer);
  g_free (session);
}

GST_END_TEST;

static gboolean
check_if_in_bundle_group (GstSDPMessage * msg, gchar * mid)
{
  gboolean ret = FALSE;
  const gchar *attr;
  gchar **mids;
  int i;

  attr = gst_sdp_message_get_attribute_val (msg, "group");

  fail_if (attr == NULL);

  mids = g_strsplit (attr, " ", 0);

  for (i = 0; mids[i] != NULL; i++) {
    ret = g_strcmp0 (mids[i], mid) == 0;
    if (ret) {
      break;
    }
  }

  g_strfreev (mids);

  return ret;
}

GST_START_TEST (sdp_agent_renegotiation_offer_remove_bundle_media)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  GError *err = NULL;
  GstSDPMessage *offer, *answer;
  gint gid, id, rid;
  gchar *sdp_str = NULL, *session;
  const GstSDPOrigin *o;
  guint64 v1, v2, v3, v4, v5;
  gboolean check_bundle = TRUE;
  const GstSDPMedia *media;
  SdpMessageContext *ctx;

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  gid = kms_sdp_agent_create_bundle_group (answerer);
  fail_if (gid < 0);

  g_object_set (answerer, "addr", OFFERER_ADDR, NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  fail_unless (kms_sdp_agent_add_handler_to_group (answerer, gid, id));

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  rid = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id < 0);

  fail_unless (kms_sdp_agent_add_handler_to_group (answerer, gid, rid));

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_sctp_media_handler_new ());
  fail_if (handler == NULL);

  id = kms_sdp_agent_add_proto_handler (answerer, "application", handler);
  fail_if (id < 0);

  fail_unless (kms_sdp_agent_add_handler_to_group (answerer, gid, id));

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  gid = kms_sdp_agent_create_bundle_group (offerer);
  fail_if (gid < 0);

  g_object_set (offerer, "addr", OFFERER_ADDR, NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  fail_unless (kms_sdp_agent_add_handler_to_group (offerer, gid, id));

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  rid = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (id < 0);

  fail_unless (kms_sdp_agent_add_handler_to_group (offerer, gid, rid));

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_sctp_media_handler_new ());
  fail_if (handler == NULL);

  id = kms_sdp_agent_add_proto_handler (offerer, "application", handler);
  fail_if (id < 0);

  fail_unless (kms_sdp_agent_add_handler_to_group (offerer, gid, id));

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  o = gst_sdp_message_get_origin (offer);
  v1 = g_ascii_strtoull (o->sess_version, NULL, 10);
  session = g_strdup (o->sess_id);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_unless (gst_sdp_message_medias_len (offer) == 3);

  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (!check_if_in_bundle_group (offer, "video0"));
  fail_if (!check_if_in_bundle_group (offer, "audio0"));
  fail_if (!check_if_in_bundle_group (offer, "application0"));

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (check_if_media_is_removed (offer, 1));
  fail_if (check_if_media_is_removed (offer, 2));

  sdp_utils_for_each_media (offer, check_mid_attr, &check_bundle);

  /* We set our local description for further renegotiations */
  kms_sdp_agent_set_local_description (offerer, offer, &err);
  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));

  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (offerer, answer, &err));
  fail_if (!kms_sdp_agent_set_local_description (answerer, answer, &err));

  GST_DEBUG ("Remove audio stream");
  fail_if (!kms_sdp_agent_remove_proto_handler (offerer, rid));

  /* Make a new offer */
  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  o = gst_sdp_message_get_origin (offer);
  v2 = g_ascii_strtoull (o->sess_version, NULL, 10);

  GST_DEBUG ("New Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_unless (gst_sdp_message_medias_len (offer) == 3);

  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (!check_if_in_bundle_group (offer, "video0"));
  fail_if (check_if_in_bundle_group (offer, "audio0"));
  fail_if (!check_if_in_bundle_group (offer, "application0"));

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (!check_if_media_is_removed (offer, 1));
  fail_if (check_if_media_is_removed (offer, 2));

  sdp_utils_for_each_media (offer, check_mid_attr, &check_bundle);

  /* This should be a new version of this session */
  fail_unless (g_strcmp0 (session, o->sess_id) == 0 && v1 + 1 == v2);

  kms_sdp_agent_set_local_description (offerer, offer, &err);
  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));

  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (offerer, answer, &err));
  fail_if (!kms_sdp_agent_set_local_description (answerer, answer, &err));

  GST_DEBUG ("Removing application handler from group");
  fail_if (!kms_sdp_agent_remove_handler_from_group (offerer, gid, id));

  /* Make a new offer */
  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Next Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  o = gst_sdp_message_get_origin (offer);
  v3 = g_ascii_strtoull (o->sess_version, NULL, 10);

  fail_unless (gst_sdp_message_medias_len (offer) == 3);

  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (!check_if_in_bundle_group (offer, "video0"));
  fail_if (check_if_in_bundle_group (offer, "audio0"));
  fail_if (check_if_in_bundle_group (offer, "application0"));

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (!check_if_media_is_removed (offer, 1));
  fail_if (check_if_media_is_removed (offer, 2));

  sdp_utils_for_each_media (offer, check_mid_attr, &check_bundle);

  /* sdp has the same version plus one */
  fail_unless (g_strcmp0 (session, o->sess_id) == 0 && v2 + 1 == v3);

  kms_sdp_agent_set_local_description (offerer, offer, &err);
  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));

  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (offerer, answer, &err));
  fail_if (!kms_sdp_agent_set_local_description (answerer, answer, &err));

  /* We add a new media, this should re-use the slot used by audio0 */
  GST_DEBUG ("Adding a new audio media");

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (id < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Next Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  o = gst_sdp_message_get_origin (offer);
  v4 = g_ascii_strtoull (o->sess_version, NULL, 10);

  /* sdp is should have changed */
  fail_unless (g_strcmp0 (session, o->sess_id) == 0 && v3 + 1 == v4);

  fail_unless (gst_sdp_message_medias_len (offer) == 3);

  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (!check_if_in_bundle_group (offer, "video0"));
  fail_if (check_if_in_bundle_group (offer, "audio1"));
  fail_if (check_if_in_bundle_group (offer, "application0"));

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (check_if_media_is_removed (offer, 1));
  fail_if (check_if_media_is_removed (offer, 2));

  kms_sdp_agent_set_local_description (offerer, offer, &err);
  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));

  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_remote_description (offerer, answer, &err));
  fail_if (!kms_sdp_agent_set_local_description (answerer, answer, &err));

  GST_DEBUG ("Adding a new video media to the BUNDLE group");
  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  fail_unless (kms_sdp_agent_add_handler_to_group (offerer, gid, id));

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Last Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  kms_sdp_agent_set_local_description (offerer, offer, &err);

  o = gst_sdp_message_get_origin (offer);
  v5 = g_ascii_strtoull (o->sess_version, NULL, 10);

  /* sdp is should have changed */
  fail_unless (g_strcmp0 (session, o->sess_id) == 0 && v4 + 1 == v5);

  fail_unless (gst_sdp_message_medias_len (offer) == 4);

  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  fail_if (!check_if_in_bundle_group (offer, "video0"));
  fail_if (check_if_in_bundle_group (offer, "audio1"));
  fail_if (check_if_in_bundle_group (offer, "application0"));
  fail_if (!check_if_in_bundle_group (offer, "video1"));

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (check_if_media_is_removed (offer, 1));
  fail_if (check_if_media_is_removed (offer, 2));
  fail_if (check_if_media_is_removed (offer, 2));

  gst_sdp_message_free (offer);

  g_object_unref (offerer);
  g_object_unref (answerer);

  g_free (session);
}

GST_END_TEST;

GST_START_TEST (sdp_agent_check_state_machine)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  GstSDPMessage *offer, *answer;
  gchar *sdp_str = NULL;
  GError *err = NULL;
  SdpMessageContext *ctx;
  gint id1, id2, gid1, gid2;
  const GstSDPOrigin *orig;
  guint64 v2, tmp;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id1 = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id1 < 0);

  gid1 = kms_sdp_agent_create_bundle_group (offerer);
  fail_if (gid1 < 0);

  fail_if (!kms_sdp_agent_add_handler_to_group (offerer, gid1, id1));

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id2 = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id2 < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offerer generates initial offer:\n%s",
      (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  orig = gst_sdp_message_get_origin (offer);

  /* Offerer is now in LOCAL_OFFER so it won't allowe to create a new offer */
  kms_sdp_agent_create_offer (offerer, &err);
  fail_unless (err != NULL);
  g_clear_error (&err);

  /* In this state it is not allowed to manipulate the SDP in any way */
  fail_unless (kms_sdp_agent_add_proto_handler (offerer, "audio", handler) < 0);
  fail_unless (!kms_sdp_agent_remove_proto_handler (offerer, id1));
  fail_unless (kms_sdp_agent_create_bundle_group (offerer) < 0);
  fail_unless (!kms_sdp_agent_remove_handler_from_group (offerer, gid1, id1));
  fail_unless (!kms_sdp_agent_add_handler_to_group (offerer, gid1, id1));

  fail_if (!kms_sdp_agent_set_local_description (offerer, offer, &err));

  /* Offerer is now in WAIT_NEGO, neither creataion of new offers nor further */
  /* modifications of the media will be allowed */
  kms_sdp_agent_create_offer (offerer, &err);
  fail_unless (err != NULL);
  g_clear_error (&err);

  fail_unless (kms_sdp_agent_add_proto_handler (offerer, "audio", handler) < 0);
  fail_unless (!kms_sdp_agent_remove_proto_handler (offerer, id1));
  fail_unless (kms_sdp_agent_create_bundle_group (offerer) < 0);
  fail_unless (!kms_sdp_agent_remove_handler_from_group (offerer, gid1, id1));
  fail_unless (!kms_sdp_agent_add_handler_to_group (offerer, gid1, id1));

  /* Now let's check the answerer */
  gid2 = kms_sdp_agent_create_bundle_group (answerer);
  fail_if (gid2 < 0);

  fail_if (!kms_sdp_agent_add_handler_to_group (answerer, gid2, id2));

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer processes the offer:\n%s",
      (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  /* Answerer is not in state REMOTE_OFFER, neither creataion of new offers */
  /* nor further modifications of the media will be allowed */
  kms_sdp_agent_create_offer (answerer, &err);
  fail_unless (err != NULL);
  g_clear_error (&err);

  fail_unless (kms_sdp_agent_add_proto_handler (answerer, "audio",
          handler) < 0);
  fail_unless (!kms_sdp_agent_remove_proto_handler (answerer, id2));
  fail_unless (kms_sdp_agent_create_bundle_group (answerer) < 0);
  fail_unless (!kms_sdp_agent_remove_handler_from_group (answerer, gid2, id2));
  fail_unless (!kms_sdp_agent_add_handler_to_group (answerer, gid2, id2));

  fail_if (!kms_sdp_agent_set_local_description (answerer, answer, &err));

  orig = gst_sdp_message_get_origin (answer);
  v2 = g_ascii_strtoull (orig->sess_version, NULL, 10);

  /* Answered is in state NEGOTIATED. Let's check the offerer again */
  fail_if (!kms_sdp_agent_set_remote_description (offerer, answer, &err));

  /* Both, offerer and answered are in state NEGOTIATED. Let's renegotiate */
  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  /* Add a new handler to be negotiated */
  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  GST_DEBUG ("Answer adds a new audio handler");
  id2 = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id2 < 0);

  offer = kms_sdp_agent_create_offer (answerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Answerer generates a new offer:\n%s",
      (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  orig = gst_sdp_message_get_origin (offer);
  tmp = g_ascii_strtoull (orig->sess_version, NULL, 10);

  /* Session version must have changed */
  fail_unless (v2 + 1 == tmp);

  gst_sdp_message_free (offer);

  /* Cancel offer */
  fail_if (!kms_sdpagent_cancel_offer (answerer, &err));

  offer = kms_sdp_agent_create_offer (answerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Answer offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  orig = gst_sdp_message_get_origin (offer);
  tmp = g_ascii_strtoull (orig->sess_version, NULL, 10);

  /* Session version must not have changed */
  fail_unless (v2 + 1 == tmp);
  gst_sdp_message_free (offer);

  /* Cancel offer */
  fail_if (!kms_sdpagent_cancel_offer (answerer, &err));

  /* Remove handler and generate the offer again */
  fail_if (!kms_sdp_agent_remove_proto_handler (answerer, id2));

  offer = kms_sdp_agent_create_offer (answerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  orig = gst_sdp_message_get_origin (offer);
  tmp = g_ascii_strtoull (orig->sess_version, NULL, 10);

  fail_unless (v2 + 1 == tmp);

  gst_sdp_message_free (offer);

  g_object_unref (offerer);
  g_object_unref (answerer);
}

GST_END_TEST;

GST_START_TEST (sdp_agent_renegotiation_disordered_media_handlers)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  gint id1, id2, id3, id4, id5, gid1, gid2;
  SdpMessageContext *ctx;
  GstSDPMessage *offer, *answer;
  gchar *sdp_str = NULL, *session;
  GError *err = NULL;
  const GstSDPMedia *media;
  const GstSDPOrigin *o;
  guint64 v1, v2, v3;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  gid1 = kms_sdp_agent_create_bundle_group (offerer);
  fail_if (gid1 < 0);

  gid2 = kms_sdp_agent_create_bundle_group (answerer);
  fail_if (gid2 < 0);

  /* Configure offerer */
  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id1 = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id1 < 0);

  fail_if (!kms_sdp_agent_add_handler_to_group (offerer, gid1, id1));

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id2 = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (id2 < 0);

  fail_if (!kms_sdp_agent_add_handler_to_group (offerer, gid1, id2));

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_sctp_media_handler_new ());
  fail_if (handler == NULL);

  id3 = kms_sdp_agent_add_proto_handler (offerer, "application", handler);
  fail_if (id3 < 0);

  fail_if (!kms_sdp_agent_add_handler_to_group (offerer, gid1, id3));

  /* Configure answerer */
  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id4 = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id4 < 0);

  fail_if (!kms_sdp_agent_add_handler_to_group (answerer, gid2, id4));

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id5 = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id5 < 0);

  fail_if (!kms_sdp_agent_add_handler_to_group (answerer, gid2, id5));

  /* Now both agents have nearly the same handlers but in different order, */
  /* offers generated by them will be compatible but the order of medias  */
  /* will be different, we must check that renegotiation of SDP will keep */
  /* the order set by the offerer */

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  o = gst_sdp_message_get_origin (offer);
  v1 = g_ascii_strtoull (o->sess_version, NULL, 10);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_local_description (offerer, offer, &err));
  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);

  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  fail_if (!kms_sdp_agent_set_local_description (answerer, answer, &err));
  fail_if (!kms_sdp_agent_set_remote_description (offerer, answer, &err));

  o = gst_sdp_message_get_origin (answer);
  session = g_strdup (o->sess_id);
  v2 = g_ascii_strtoull (o->sess_version, NULL, 10);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  GST_DEBUG ("Offerer version: %" G_GUINT64_FORMAT, v1);
  GST_DEBUG ("Answerer version: %" G_GUINT64_FORMAT, v2);

  /* Now, both agent are in state NEGOTIATED, so new offers should keep the */
  /* same order of media if they have not been removed. Let's create an     */
  /* offer with answerer that has a different order of media handlers       */

  offer = kms_sdp_agent_create_offer (answerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer generated by the answerer:\n%s",
      (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  o = gst_sdp_message_get_origin (answer);
  v3 = g_ascii_strtoull (o->sess_version, NULL, 10);

  fail_unless (g_strcmp0 (session, o->sess_id) == 0 && v2 == v3);

  /* Check that medias are orderer and supported */
  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);
  fail_if (gst_sdp_media_get_port (media) == 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);
  fail_if (gst_sdp_media_get_port (media) == 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);
  fail_if (gst_sdp_media_get_port (media) != 0);

  gst_sdp_message_free (offer);

  g_object_unref (offerer);
  g_object_unref (answerer);
  g_free (session);
}

GST_END_TEST;

GST_START_TEST (sdp_agent_renegotiation_complex_case)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  gint id1, id2, id3, id4, id5, id6, id7, id8, gid1, gid2;
  SdpMessageContext *ctx;
  GstSDPMessage *offer, *answer;
  gchar *sdp_str = NULL, *session1, *session2;
  GError *err = NULL;
  gboolean check_bundle = TRUE;
  const GstSDPMedia *media;
  const GstSDPOrigin *o;
  guint64 v1, v2, tmp;

  /* Configure offerer */
  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  gid1 = kms_sdp_agent_create_bundle_group (offerer);
  fail_if (gid1 < 0);

  g_object_set (offerer, "addr", OFFERER_ADDR, NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id1 = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id1 < 0);

  fail_unless (kms_sdp_agent_add_handler_to_group (offerer, gid1, id1));

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id2 = kms_sdp_agent_add_proto_handler (offerer, "audio", handler);
  fail_if (id2 < 0);

  fail_unless (kms_sdp_agent_add_handler_to_group (offerer, gid1, id2));

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_sctp_media_handler_new ());
  fail_if (handler == NULL);

  id3 = kms_sdp_agent_add_proto_handler (offerer, "application", handler);
  fail_if (id3 < 0);

  fail_unless (kms_sdp_agent_add_handler_to_group (offerer, gid1, id3));

  /* Configure answerer using dispordered handlers */
  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  gid2 = kms_sdp_agent_create_bundle_group (answerer);
  fail_if (gid1 < 0);

  g_object_set (answerer, "addr", OFFERER_ADDR, NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_sctp_media_handler_new ());
  fail_if (handler == NULL);

  id4 = kms_sdp_agent_add_proto_handler (answerer, "application", handler);
  fail_if (id4 < 0);

  fail_unless (kms_sdp_agent_add_handler_to_group (answerer, gid2, id4));

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id5 = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id5 < 0);

  fail_unless (kms_sdp_agent_add_handler_to_group (answerer, gid2, id5));

  /* Let's negotiate */
  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  o = gst_sdp_message_get_origin (offer);
  v1 = g_ascii_strtoull (o->sess_version, NULL, 10);
  session1 = g_strdup (o->sess_id);

  fail_if (!kms_sdp_agent_set_local_description (offerer, offer, &err));

  /* Check that medias are orderer and supported */
  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (!check_if_in_bundle_group (offer, "video0"));
  fail_if (!check_if_in_bundle_group (offer, "audio0"));
  fail_if (!check_if_in_bundle_group (offer, "application0"));

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (check_if_media_is_removed (offer, 1));
  fail_if (check_if_media_is_removed (offer, 2));

  sdp_utils_for_each_media (offer, check_mid_attr, &check_bundle);

  /* We set our local description for further renegotiations */
  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  /* Check that medias are orderer and supported */
  media = gst_sdp_message_get_media (answer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (answer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (answer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (!check_if_in_bundle_group (answer, "video0"));
  fail_if (check_if_in_bundle_group (answer, "audio0"));
  fail_if (!check_if_in_bundle_group (answer, "application0"));

  fail_if (check_if_media_is_removed (answer, 0));
  fail_if (!check_if_media_is_removed (answer, 1));
  fail_if (check_if_media_is_removed (answer, 2));

  o = gst_sdp_message_get_origin (answer);
  session2 = g_strdup (o->sess_id);
  v2 = g_ascii_strtoull (o->sess_version, NULL, 10);

  GST_DEBUG ("Offerer session: %s, version %" G_GUINT64_FORMAT, session1, v1);
  GST_DEBUG ("Answerer session: %s, version %" G_GUINT64_FORMAT, session2, v2);

  fail_if (!kms_sdp_agent_set_remote_description (offerer, answer, &err));
  fail_if (!kms_sdp_agent_set_local_description (answerer, answer, &err));

  /* Lets create another offer with the offerer */
  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Next Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  o = gst_sdp_message_get_origin (offer);
  tmp = g_ascii_strtoull (o->sess_version, NULL, 10);

  /* The SDP must be the same */
  fail_unless (g_strcmp0 (session1, o->sess_id) == 0 && v1 == tmp);

  fail_if (!kms_sdp_agent_set_local_description (offerer, offer, &err));

  /* Check that medias are orderer and supported */
  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (!check_if_in_bundle_group (offer, "video0"));
  fail_if (check_if_in_bundle_group (offer, "audio0"));
  fail_if (!check_if_in_bundle_group (offer, "application0"));

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (!check_if_media_is_removed (offer, 1));
  fail_if (check_if_media_is_removed (offer, 2));

  sdp_utils_for_each_media (offer, check_mid_attr, &check_bundle);

  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));

  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  /* Check that medias are orderer and supported */
  media = gst_sdp_message_get_media (answer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (answer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (answer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (!check_if_in_bundle_group (answer, "video0"));
  fail_if (check_if_in_bundle_group (answer, "audio0"));
  fail_if (!check_if_in_bundle_group (answer, "application0"));

  fail_if (check_if_media_is_removed (answer, 0));
  fail_if (!check_if_media_is_removed (answer, 1));
  fail_if (check_if_media_is_removed (answer, 2));

  fail_if (!kms_sdp_agent_set_remote_description (offerer, answer, &err));
  fail_if (!kms_sdp_agent_set_local_description (answerer, answer, &err));

  o = gst_sdp_message_get_origin (answer);
  tmp = g_ascii_strtoull (o->sess_version, NULL, 10);

  /* The SDP must be the same */
  fail_unless (g_strcmp0 (session2, o->sess_id) == 0 && v2 == tmp);

  /* Lets create another offer using the answerer */
  offer = kms_sdp_agent_create_offer (answerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer from answerer:\n%s", (sdp_str =
          gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  o = gst_sdp_message_get_origin (offer);
  tmp = g_ascii_strtoull (o->sess_version, NULL, 10);

  /* The SDP must be the same */
  fail_unless (g_strcmp0 (session2, o->sess_id) == 0 && v2 == tmp);

  fail_if (!kms_sdp_agent_set_local_description (answerer, offer, &err));

  /* Check that medias are orderer and supported */
  media = gst_sdp_message_get_media (answer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (answer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (answer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (!check_if_in_bundle_group (answer, "video0"));
  fail_if (check_if_in_bundle_group (answer, "audio0"));
  fail_if (!check_if_in_bundle_group (answer, "application0"));

  fail_if (check_if_media_is_removed (answer, 0));
  fail_if (!check_if_media_is_removed (answer, 1));
  fail_if (check_if_media_is_removed (answer, 2));

  sdp_utils_for_each_media (offer, check_mid_attr, &check_bundle);

  fail_if (!kms_sdp_agent_set_remote_description (offerer, offer, &err));

  ctx = kms_sdp_agent_create_answer (offerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  o = gst_sdp_message_get_origin (answer);
  tmp = g_ascii_strtoull (o->sess_version, NULL, 10);

  /* The SDP must be the same */
  fail_unless (g_strcmp0 (session1, o->sess_id) == 0 && v1 == tmp);

  GST_DEBUG ("Answer from offerer:\n%s", (sdp_str =
          gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  media = gst_sdp_message_get_media (answer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (answer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (answer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (!check_if_in_bundle_group (answer, "video0"));
  fail_if (check_if_in_bundle_group (answer, "audio0"));
  fail_if (!check_if_in_bundle_group (answer, "application0"));

  fail_if (check_if_media_is_removed (answer, 0));
  fail_if (!check_if_media_is_removed (answer, 1));
  fail_if (check_if_media_is_removed (answer, 2));

  sdp_utils_for_each_media (offer, check_mid_attr, &check_bundle);

  fail_if (!kms_sdp_agent_set_local_description (offerer, answer, &err));
  fail_if (!kms_sdp_agent_set_remote_description (answerer, answer, &err));

  /* Lets create an audio handler for the answerer. The handler is AVP that */
  /* must be supported by the remote side that uses AVPF                    */

  GST_DEBUG
      ("Add a AVP audio handler in the answerer and generate a new offer");

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_avp_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id6 = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id6 < 0);

  fail_unless (kms_sdp_agent_add_handler_to_group (answerer, gid2, id6));

  /* Create an offer using the answerer again */
  offer = kms_sdp_agent_create_offer (answerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer from answerer:\n%s", (sdp_str =
          gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  o = gst_sdp_message_get_origin (offer);
  tmp = g_ascii_strtoull (o->sess_version, NULL, 10);

  /* New media should fill the unsupportd audio media instead of creating */
  /* a new netry in the SDP.                                              */
  fail_unless (gst_sdp_message_medias_len (offer) == 3);

  /* The SDP must have changed so we added a new media */
  fail_unless (g_strcmp0 (session2, o->sess_id) == 0 && v2 + 1 == tmp);

  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (!check_if_in_bundle_group (offer, "video0"));
  fail_if (!check_if_in_bundle_group (offer, "audio1"));
  fail_if (!check_if_in_bundle_group (offer, "application0"));

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (check_if_media_is_removed (offer, 1));
  fail_if (check_if_media_is_removed (offer, 2));

  sdp_utils_for_each_media (offer, check_mid_attr, &check_bundle);

  fail_if (!kms_sdp_agent_set_local_description (answerer, offer, &err));
  fail_if (!kms_sdp_agent_set_remote_description (offerer, offer, &err));

  ctx = kms_sdp_agent_create_answer (offerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer from offerer:\n%s", (sdp_str =
          gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  /* New media should fill the unsupportd audio media instead of creating */
  /* a new netry in the SDP.                                              */
  fail_unless (gst_sdp_message_medias_len (answer) == 3);

  media = gst_sdp_message_get_media (answer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (answer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (answer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (!check_if_in_bundle_group (answer, "video0"));
  fail_if (!check_if_in_bundle_group (answer, "audio1"));
  fail_if (!check_if_in_bundle_group (answer, "application0"));

  fail_if (check_if_media_is_removed (answer, 0));
  fail_if (check_if_media_is_removed (answer, 1));
  fail_if (check_if_media_is_removed (answer, 2));

  sdp_utils_for_each_media (offer, check_mid_attr, &check_bundle);

  fail_if (!kms_sdp_agent_set_local_description (offerer, answer, &err));
  fail_if (!kms_sdp_agent_set_remote_description (answerer, answer, &err));

  GST_DEBUG ("Answerer removes video and application handlers");

  fail_if (!kms_sdp_agent_remove_proto_handler (answerer, id5));
  fail_if (!kms_sdp_agent_remove_proto_handler (answerer, id4));

  offer = kms_sdp_agent_create_offer (answerer, &err);
  fail_if (err != NULL);

  /* Create an offer using the answerer again */
  GST_DEBUG ("Offer from answerer:\n%s", (sdp_str =
          gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  o = gst_sdp_message_get_origin (offer);
  tmp = g_ascii_strtoull (o->sess_version, NULL, 10);

  fail_unless (gst_sdp_message_medias_len (offer) == 3);

  /* The SDP must have changed so we added a new media */
  fail_unless (g_strcmp0 (session2, o->sess_id) == 0 && v2 + 2 == tmp);

  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (check_if_in_bundle_group (offer, "video0"));
  fail_if (!check_if_in_bundle_group (offer, "audio1"));
  fail_if (check_if_in_bundle_group (offer, "application0"));

  fail_if (!check_if_media_is_removed (offer, 0));
  fail_if (check_if_media_is_removed (offer, 1));
  fail_if (!check_if_media_is_removed (offer, 2));

  sdp_utils_for_each_media (offer, check_mid_attr, &check_bundle);

  fail_if (!kms_sdp_agent_set_local_description (answerer, offer, &err));
  fail_if (!kms_sdp_agent_set_remote_description (offerer, offer, &err));

  ctx = kms_sdp_agent_create_answer (offerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer from offerer:\n%s", (sdp_str =
          gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_unless (gst_sdp_message_medias_len (answer) == 3);

  media = gst_sdp_message_get_media (answer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  media = gst_sdp_message_get_media (answer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (answer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "application") != 0);

  fail_if (check_if_in_bundle_group (answer, "video0"));
  fail_if (!check_if_in_bundle_group (answer, "audio1"));
  fail_if (check_if_in_bundle_group (answer, "application0"));

  fail_if (!check_if_media_is_removed (answer, 0));
  fail_if (check_if_media_is_removed (answer, 1));
  fail_if (!check_if_media_is_removed (answer, 2));

  sdp_utils_for_each_media (offer, check_mid_attr, &check_bundle);

  fail_if (!kms_sdp_agent_set_local_description (offerer, answer, &err));
  fail_if (!kms_sdp_agent_set_remote_description (answerer, answer, &err));

  /* Lets create two more handlers to fill the removed slots */
  GST_DEBUG ("Answerer adds a new audio handlers");
  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id7 = kms_sdp_agent_add_proto_handler (answerer, "audio", handler);
  fail_if (id7 < 0);

  GST_DEBUG ("Answerer adds a new video handler");
  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id8 = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id8 < 0);

  GST_DEBUG ("Answerer adds the new video handler to the BUNDLE group");

  fail_unless (kms_sdp_agent_add_handler_to_group (answerer, gid2, id8));

  /* Let's negotiate using the answerer again */
  offer = kms_sdp_agent_create_offer (answerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer from answerer:\n%s", (sdp_str =
          gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  o = gst_sdp_message_get_origin (offer);
  tmp = g_ascii_strtoull (o->sess_version, NULL, 10);

  /* New media should fill the unsupportd audio media instead of creating */
  /* a new netry in the SDP.                                              */
  fail_unless (gst_sdp_message_medias_len (offer) == 3);

  /* The SDP must have changed so we added a new media */
  fail_unless (g_strcmp0 (session2, o->sess_id) == 0 && v2 + 3 == tmp);

  media = gst_sdp_message_get_media (offer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (offer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (offer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  fail_if (check_if_in_bundle_group (offer, "video0"));
  fail_if (!check_if_in_bundle_group (offer, "audio1"));
  fail_if (check_if_in_bundle_group (offer, "application0"));
  fail_if (!check_if_in_bundle_group (offer, "video1"));

  fail_if (check_if_media_is_removed (offer, 0));
  fail_if (check_if_media_is_removed (offer, 1));
  fail_if (check_if_media_is_removed (offer, 2));

  sdp_utils_for_each_media (offer, check_mid_attr, &check_bundle);

  fail_if (!kms_sdp_agent_set_local_description (answerer, offer, &err));
  fail_if (!kms_sdp_agent_set_remote_description (offerer, offer, &err));

  ctx = kms_sdp_agent_create_answer (offerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer from offerer:\n%s", (sdp_str =
          gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_unless (gst_sdp_message_medias_len (answer) == 3);

  media = gst_sdp_message_get_media (answer, 0);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (answer, 1);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "audio") != 0);

  media = gst_sdp_message_get_media (answer, 2);
  fail_if (g_strcmp0 (gst_sdp_media_get_media (media), "video") != 0);

  fail_if (check_if_in_bundle_group (answer, "video0"));
  fail_if (check_if_in_bundle_group (answer, "audio0"));
  fail_if (check_if_in_bundle_group (answer, "application0"));
  fail_if (!check_if_in_bundle_group (answer, "audio1"));
  fail_if (!check_if_in_bundle_group (answer, "video1"));

  fail_if (!check_if_media_is_removed (answer, 0));
  fail_if (check_if_media_is_removed (answer, 1));
  fail_if (check_if_media_is_removed (answer, 2));

  sdp_utils_for_each_media (offer, check_mid_attr, &check_bundle);

  fail_if (!kms_sdp_agent_set_local_description (offerer, answer, &err));
  fail_if (!kms_sdp_agent_set_remote_description (answerer, answer, &err));

  g_free (session1);
  g_free (session2);

  g_object_unref (answerer);
  g_object_unref (offerer);
}

GST_END_TEST;

GST_START_TEST (sdp_agent_groups)
{
  KmsSdpMediaHandler *handler;
  KmsSdpAgent *offerer, *answerer;
  gint gid, id;
  gchar *sdp_str = NULL;
  SdpMessageContext *ctx;
  GError *err = NULL;
  GstSDPMessage *offer, *answer;

  offerer = kms_sdp_agent_new ();

  gid = kms_sdp_agent_create_group (offerer, KMS_TYPE_SDP_BUNDLE_GROUP, NULL);
  fail_if (gid < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (offerer, "video", handler);
  fail_if (id < 0);

  kms_sdp_agent_group_add (offerer, gid, id);

  answerer = kms_sdp_agent_new ();

  fail_if (kms_sdp_agent_create_group (answerer, KMS_TYPE_SDP_BUNDLE_GROUP,
          NULL) < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  id = kms_sdp_agent_add_proto_handler (answerer, "video", handler);
  fail_if (id < 0);

  /* Make negotiation */
  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_local_description (offerer, offer, &err));
  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));

  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_local_description (answerer, answer, &err));
  fail_if (!kms_sdp_agent_set_remote_description (offerer, answer, &err));

  /* Renegotiate */
  offer = kms_sdp_agent_create_offer (answerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  gst_sdp_message_free (offer);

  g_object_unref (offerer);
  g_object_unref (answerer);
}

GST_END_TEST;

static gchar *offered_ips[] = {
  "223.123.123.43",
  "4.5.234.12",
  "212.123.123.23",
  "2aaa:aaaa:aaaa:aaaa:9b9:300:7d6a:faa9/64",
  "2aaa:aaaa:aaaa:aaaa:be85:56ff:fe03:128b/64"
};

static gchar *answered_ips[] = {
  "123.123.123.123",
  "4.4.4.4",
  "6.6.6.6",
  "2aaa:bbbb:bbbb:bbbb:9b9:300:7d6a:faa9/64",
  "2aaa:bbbb:bbbb:bbbb:be85:56ff:fe03:128b/64"
};

static gboolean
sdp_agent_test_connection_check_ips (const GArray * ips, gchar ** array,
    guint len)
{
  guint i;

  for (i = 0; i < len; i++) {
    const GstStructure *addr;
    gchar *address;
    GValue *val;

    val = &g_array_index (ips, GValue, i);

    if (!GST_VALUE_HOLDS_STRUCTURE (val)) {
      return FALSE;
    }

    addr = gst_value_get_structure (val);

    if (!gst_structure_get (addr, "address", G_TYPE_STRING, &address, NULL)) {
      return FALSE;
    }

    if (g_strcmp0 (address, array[i]) != 0) {
      g_free (address);
      return FALSE;
    }

    g_free (address);
  }

  return TRUE;
}

static void
sdp_agent_test_connection_add_ips (GArray * ips, gchar ** array, guint len)
{
  guint i;

  for (i = 0; i < len; i++) {
    GValue val = G_VALUE_INIT;
    GstStructure *addr;
    gchar *addrype;

    if (i < 3) {
      addrype = "IP4";
    } else {
      addrype = "IP6";
    }

    addr = gst_structure_new ("sdp-connection", "nettype", G_TYPE_STRING,
        "IN", "addrtype", G_TYPE_STRING, addrype, "address", G_TYPE_STRING,
        array[i], "ttl", G_TYPE_UINT, 0, "addrnumber", G_TYPE_UINT, 0, NULL);

    g_value_init (&val, GST_TYPE_STRUCTURE);
    gst_value_set_structure (&val, addr);
    gst_structure_free (addr);

    g_array_append_val (ips, val);
  }
}

static void
sdp_agent_test_connection_on_offer_ips (KmsConnectionExt * ext, GArray * ips)
{
  sdp_agent_test_connection_add_ips (ips, offered_ips,
      G_N_ELEMENTS (offered_ips));
}

static void
sdp_agent_test_connection_on_answered_ips (KmsConnectionExt * ext,
    const GArray * ips)
{
  fail_if (!sdp_agent_test_connection_check_ips (ips, answered_ips,
          G_N_ELEMENTS (answered_ips)));
}

static void
sdp_agent_test_connection_on_answer_ips (KmsConnectionExt * ext,
    const GArray * ips_offered, GArray * ips_answered)
{
  fail_if (!sdp_agent_test_connection_check_ips (ips_offered, offered_ips,
          G_N_ELEMENTS (offered_ips)));
  sdp_agent_test_connection_add_ips (ips_answered, answered_ips,
      G_N_ELEMENTS (answered_ips));
}

GST_START_TEST (sdp_agent_test_connection_ext)
{
  KmsSdpAgent *offerer, *answerer;
  KmsSdpMediaHandler *handler;
  SdpMessageContext *ctx;
  GstSDPMessage *offer, *answer;
  KmsConnectionExt *ext1, *ext2;
  GError *err = NULL;
  gchar *sdp_str = NULL;

  offerer = kms_sdp_agent_new ();
  fail_if (offerer == NULL);

  answerer = kms_sdp_agent_new ();
  fail_if (answerer == NULL);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  ext1 = kms_connection_ext_new ();
  fail_if (!kms_sdp_media_handler_add_media_extension (handler,
          KMS_I_SDP_MEDIA_EXTENSION (ext1)));

  g_signal_connect (ext1, "on-offer-ips",
      G_CALLBACK (sdp_agent_test_connection_on_offer_ips), NULL);
  g_signal_connect (ext1, "on-answered-ips",
      G_CALLBACK (sdp_agent_test_connection_on_answered_ips), NULL);

  fail_if (kms_sdp_agent_add_proto_handler (offerer, "video", handler) < 0);

  handler = KMS_SDP_MEDIA_HANDLER (kms_sdp_rtp_savpf_media_handler_new ());
  fail_if (handler == NULL);

  set_default_codecs (KMS_SDP_RTP_AVP_MEDIA_HANDLER (handler), audio_codecs,
      G_N_ELEMENTS (audio_codecs), video_codecs, G_N_ELEMENTS (video_codecs));

  ext2 = kms_connection_ext_new ();
  fail_if (!kms_sdp_media_handler_add_media_extension (handler,
          KMS_I_SDP_MEDIA_EXTENSION (ext2)));

  g_signal_connect (ext2, "on-answer-ips",
      G_CALLBACK (sdp_agent_test_connection_on_answer_ips), NULL);

  fail_if (kms_sdp_agent_add_proto_handler (answerer, "video", handler) < 0);

  offer = kms_sdp_agent_create_offer (offerer, &err);
  fail_if (err != NULL);

  GST_DEBUG ("Offer:\n%s", (sdp_str = gst_sdp_message_as_text (offer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_local_description (offerer, offer, &err));
  fail_if (!kms_sdp_agent_set_remote_description (answerer, offer, &err));
  ctx = kms_sdp_agent_create_answer (answerer, &err);
  fail_if (err != NULL);

  answer = kms_sdp_message_context_pack (ctx, &err);
  fail_if (err != NULL);
  kms_sdp_message_context_unref (ctx);

  GST_DEBUG ("Answer:\n%s", (sdp_str = gst_sdp_message_as_text (answer)));
  g_free (sdp_str);

  fail_if (!kms_sdp_agent_set_local_description (answerer, answer, &err));
  fail_if (!kms_sdp_agent_set_remote_description (offerer, answer, &err));

  g_object_unref (offerer);
  g_object_unref (answerer);
}

GST_END_TEST;

static Suite *
sdp_agent_suite (void)
{
  Suite *s = suite_create ("kmssdpagent");
  TCase *tc_chain = tcase_create ("SdpAgent");

  suite_add_tcase (s, tc_chain);

  tcase_add_test (tc_chain, sdp_agent_check_state_machine);
  tcase_add_test (tc_chain, sdp_agent_test_create_offer);
  tcase_add_test (tc_chain, sdp_agent_test_add_proto_handler);
  tcase_add_test (tc_chain, sdp_agent_test_rejected_negotiation);
  tcase_add_test (tc_chain, sdp_agent_test_rejected_unsupported_media);
  tcase_add_test (tc_chain, sdp_agent_test_sctp_negotiation);
  tcase_add_test (tc_chain, sdp_agent_test_rtp_avp_negotiation);
  tcase_add_test (tc_chain, sdp_agent_test_rtp_avpf_negotiation);
  tcase_add_test (tc_chain, sdp_agent_test_rtp_savp_negotiation);
  tcase_add_test (tc_chain, sdp_agent_test_rtp_savpf_negotiation);
  tcase_add_test (tc_chain, sdp_agent_avp_generic_payload_negotiation);
  tcase_add_test (tc_chain, sdp_agent_avp_avpf_negotiation);
  tcase_add_test (tc_chain, sdp_agent_test_bundle_group);
  tcase_add_test (tc_chain, sdp_agent_test_fb_messages);
  tcase_add_test (tc_chain, sdp_agent_test_rtcp_mux);
  tcase_add_test (tc_chain, sdp_agent_test_multi_m_lines);
  tcase_add_test (tc_chain, sdp_agent_test_filter_unknown_attr);
  tcase_add_test (tc_chain, sdp_agent_test_supported_attrs);
  tcase_add_test (tc_chain, sdp_agent_test_bandwidtth_attrs);
  tcase_add_test (tc_chain, sdp_agent_test_extmap_attrs);
  tcase_add_test (tc_chain, sdp_agent_test_dynamic_pts);
  tcase_add_test (tc_chain, sdp_agent_test_optional_enc_parameters);
  tcase_add_test (tc_chain, sdp_agent_regression_tests);
  tcase_add_test (tc_chain, sdp_agent_udp_tls_rtp_savpf_negotiation);
  tcase_add_test (tc_chain, sdp_agent_sdes_negotiation);
  tcase_add_test (tc_chain, sdp_agent_test_connection_ext);

  tcase_add_test (tc_chain, sdp_context_from_first_media_inactive);

  tcase_add_test (tc_chain, sdp_agent_renegotiation_offer_new_media);
  tcase_add_test (tc_chain, sdp_agent_renegotiation_offer_remove_media);
  tcase_add_test (tc_chain, sdp_agent_renegotiation_offer_remove_bundle_media);
  tcase_add_test (tc_chain, sdp_agent_renegotiation_disordered_media_handlers);
  tcase_add_test (tc_chain, sdp_agent_renegotiation_complex_case);

  tcase_add_test (tc_chain, sdp_agent_groups);

  return s;
}

GST_CHECK_MAIN (sdp_agent)
