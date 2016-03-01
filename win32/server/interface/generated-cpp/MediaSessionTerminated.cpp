/* Autogenerated with kurento-module-creator */

#include "MediaSessionTerminated.hpp"
#include <jsonrpc/JsonSerializer.hpp>
#include <time.h>
#include <string>

namespace kurento
{

void
MediaSessionTerminated::Serialize (JsonSerializer &s)
{
  Media::Serialize (s);

}

} /* kurento */

namespace kurento
{

void Serialize (std::shared_ptr<kurento::MediaSessionTerminated> &event, JsonSerializer &s)
{
  if (!s.IsWriter && !event) {
    event.reset (new kurento::MediaSessionTerminated() );
  }

  if (event) {
    event->Serialize (s);
  }
}

} /* kurento */