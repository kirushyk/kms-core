/* Autogenerated with kurento-module-creator */

#ifndef __MEDIA_PIPELINE_INTERNAL_HPP__
#define __MEDIA_PIPELINE_INTERNAL_HPP__

#include "MediaPipeline.hpp"

namespace kurento
{
class JsonSerializer;
}

namespace kurento
{
class GstreamerDotDetails;
} /* kurento */

namespace kurento
{

class MediaPipelineMethodGetGstreamerDot
{
public:
  MediaPipelineMethodGetGstreamerDot() {}
  ~MediaPipelineMethodGetGstreamerDot() {}

  std::string invoke (std::shared_ptr<MediaPipeline> obj);
  void Serialize (JsonSerializer &serializer);

  std::shared_ptr<GstreamerDotDetails> getDetails () {
    return details;
  }

  void setDetails (std::shared_ptr<GstreamerDotDetails> details) {
    this->details = details;
    __isSetDetails = true;
  }

private:
  std::shared_ptr<GstreamerDotDetails> details;
  bool __isSetDetails = false;
};

class MediaPipelineConstructor
{
public:
  MediaPipelineConstructor () {}
  ~MediaPipelineConstructor () {}

  void Serialize (JsonSerializer &serializer);

};

} /* kurento */

#endif /*  __MEDIA_PIPELINE_INTERNAL_HPP__ */
