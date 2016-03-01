/* Autogenerated with kurento-module-creator */

#ifndef __PASS_THROUGH_INTERNAL_HPP__
#define __PASS_THROUGH_INTERNAL_HPP__

#include "PassThrough.hpp"

namespace kurento
{
class JsonSerializer;
}

namespace kurento
{
class MediaPipeline;
} /* kurento */

namespace kurento
{

class PassThroughConstructor
{
public:
  PassThroughConstructor () {}
  ~PassThroughConstructor () {}

  void Serialize (JsonSerializer &serializer);

  std::shared_ptr<MediaPipeline> getMediaPipeline ();

  void setMediaPipeline (std::shared_ptr<MediaPipeline> mediaPipeline) {
    this->mediaPipeline = mediaPipeline;
  }

private:
  std::shared_ptr<MediaPipeline> mediaPipeline;
};

} /* kurento */

#endif /*  __PASS_THROUGH_INTERNAL_HPP__ */
