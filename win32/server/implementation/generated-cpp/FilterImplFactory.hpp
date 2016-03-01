/* Autogenerated with kurento-module-creator */

#ifndef __FILTER_IMPL_FACTORY_HPP__
#define __FILTER_IMPL_FACTORY_HPP__

#include "FilterImpl.hpp"
#include "MediaElementImplFactory.hpp"
#include <MediaObjectImpl.hpp>
#include <boost/property_tree/ptree.hpp>

namespace kurento
{

class FilterImplFactory : public virtual MediaElementImplFactory
{
public:
  FilterImplFactory () {};

  virtual std::string getName () const {
    return "Filter";
  };

};

} /* kurento */

#endif /*  __FILTER_IMPL_FACTORY_HPP__ */