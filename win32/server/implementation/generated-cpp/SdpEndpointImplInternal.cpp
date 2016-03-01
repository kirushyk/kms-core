/* Autogenerated with kurento-module-creator */

#include <gst/gst.h>
#include "SdpEndpointImpl.hpp"
#include "SdpEndpointImplFactory.hpp"
#include "SdpEndpointInternal.hpp"
#include <jsonrpc/JsonSerializer.hpp>
#include <KurentoException.hpp>

using kurento::KurentoException;

namespace kurento
{

void
SdpEndpointImpl::invoke (std::shared_ptr<MediaObjectImpl> obj, const std::string &methodName, const Json::Value &params, Json::Value &response)
{
  if (methodName == "generateOffer") {
    kurento::JsonSerializer s (false);
    SdpEndpointMethodGenerateOffer method;
    JsonSerializer responseSerializer (true);
    std::string ret;

    s.JsonValue = params;
    method.Serialize (s);

    ret = method.invoke (std::dynamic_pointer_cast<SdpEndpoint> (obj) );
    responseSerializer.SerializeNVP (ret);
    response = responseSerializer.JsonValue["ret"];
    return;
  }

  if (methodName == "processOffer") {
    kurento::JsonSerializer s (false);
    SdpEndpointMethodProcessOffer method;
    JsonSerializer responseSerializer (true);
    std::string ret;

    s.JsonValue = params;
    method.Serialize (s);

    ret = method.invoke (std::dynamic_pointer_cast<SdpEndpoint> (obj) );
    responseSerializer.SerializeNVP (ret);
    response = responseSerializer.JsonValue["ret"];
    return;
  }

  if (methodName == "processAnswer") {
    kurento::JsonSerializer s (false);
    SdpEndpointMethodProcessAnswer method;
    JsonSerializer responseSerializer (true);
    std::string ret;

    s.JsonValue = params;
    method.Serialize (s);

    ret = method.invoke (std::dynamic_pointer_cast<SdpEndpoint> (obj) );
    responseSerializer.SerializeNVP (ret);
    response = responseSerializer.JsonValue["ret"];
    return;
  }

  if (methodName == "getLocalSessionDescriptor") {
    kurento::JsonSerializer s (false);
    SdpEndpointMethodGetLocalSessionDescriptor method;
    JsonSerializer responseSerializer (true);
    std::string ret;

    s.JsonValue = params;
    method.Serialize (s);

    ret = method.invoke (std::dynamic_pointer_cast<SdpEndpoint> (obj) );
    responseSerializer.SerializeNVP (ret);
    response = responseSerializer.JsonValue["ret"];
    return;
  }

  if (methodName == "getRemoteSessionDescriptor") {
    kurento::JsonSerializer s (false);
    SdpEndpointMethodGetRemoteSessionDescriptor method;
    JsonSerializer responseSerializer (true);
    std::string ret;

    s.JsonValue = params;
    method.Serialize (s);

    ret = method.invoke (std::dynamic_pointer_cast<SdpEndpoint> (obj) );
    responseSerializer.SerializeNVP (ret);
    response = responseSerializer.JsonValue["ret"];
    return;
  }

  if (methodName == "getMaxVideoRecvBandwidth") {
    int ret;
    JsonSerializer responseSerializer (true);

    ret = std::dynamic_pointer_cast<SdpEndpoint> (obj)->getMaxVideoRecvBandwidth ();
    responseSerializer.SerializeNVP (ret);
    response = responseSerializer.JsonValue["ret"];
    return;
  }

  if (methodName == "setMaxVideoRecvBandwidth") {
    kurento::JsonSerializer s (false);
    int maxVideoRecvBandwidth;
    s.JsonValue = params;

    if (!s.JsonValue.isMember ("maxVideoRecvBandwidth") || !s.JsonValue["maxVideoRecvBandwidth"].isConvertibleTo (Json::ValueType::intValue) ) {
      throw KurentoException (MARSHALL_ERROR,
                              "'maxVideoRecvBandwidth' parameter should be a integer");
    }

    if (!s.IsWriter) {
      s.SerializeNVP (maxVideoRecvBandwidth);
      std::dynamic_pointer_cast<SdpEndpoint> (obj)->setMaxVideoRecvBandwidth (maxVideoRecvBandwidth);
    }
    return;
  }

  if (methodName == "getMaxAudioRecvBandwidth") {
    int ret;
    JsonSerializer responseSerializer (true);

    ret = std::dynamic_pointer_cast<SdpEndpoint> (obj)->getMaxAudioRecvBandwidth ();
    responseSerializer.SerializeNVP (ret);
    response = responseSerializer.JsonValue["ret"];
    return;
  }

  if (methodName == "setMaxAudioRecvBandwidth") {
    kurento::JsonSerializer s (false);
    int maxAudioRecvBandwidth;
    s.JsonValue = params;

    if (!s.JsonValue.isMember ("maxAudioRecvBandwidth") || !s.JsonValue["maxAudioRecvBandwidth"].isConvertibleTo (Json::ValueType::intValue) ) {
      throw KurentoException (MARSHALL_ERROR,
                              "'maxAudioRecvBandwidth' parameter should be a integer");
    }

    if (!s.IsWriter) {
      s.SerializeNVP (maxAudioRecvBandwidth);
      std::dynamic_pointer_cast<SdpEndpoint> (obj)->setMaxAudioRecvBandwidth (maxAudioRecvBandwidth);
    }
    return;
  }

  SessionEndpointImpl::invoke (obj, methodName, params, response);
}

bool
SdpEndpointImpl::connect (const std::string &eventType, std::shared_ptr<EventHandler> handler)
{

  return SessionEndpointImpl::connect (eventType, handler);
}

void
SdpEndpointImpl::Serialize (JsonSerializer &serializer)
{
  if (serializer.IsWriter) {
    try {
      Json::Value v (getId() );

      serializer.JsonValue = v;
    } catch (std::bad_cast &e) {
    }
  } else {
    throw KurentoException (MARSHALL_ERROR,
                            "'SdpEndpointImpl' cannot be deserialized as an object");
  }
}
} /* kurento */

namespace kurento
{

void
Serialize (std::shared_ptr<kurento::SdpEndpointImpl> &object, JsonSerializer &serializer)
{
  if (serializer.IsWriter) {
    if (object) {
      object->Serialize (serializer);
    }
  } else {
    std::shared_ptr<kurento::MediaObjectImpl> aux;
    aux = kurento::SdpEndpointImplFactory::getObject (JsonFixes::getString(serializer.JsonValue) );
    object = std::dynamic_pointer_cast<kurento::SdpEndpointImpl> (aux);
  }
}

void
Serialize (std::shared_ptr<kurento::SdpEndpoint> &object, JsonSerializer &serializer)
{
  std::shared_ptr<kurento::SdpEndpointImpl> aux = std::dynamic_pointer_cast<kurento::SdpEndpointImpl> (object);

  Serialize (aux, serializer);
  object = std::dynamic_pointer_cast <kurento::SdpEndpoint> (aux);
}

} /* kurento */