/* Autogenerated with kurento-module-creator */

#ifndef __MODULE_INFO_HPP__
#define __MODULE_INFO_HPP__

#include <json/json.h>
#include <jsonrpc/JsonRpcException.hpp>
#include <memory>
#include <RegisterParent.hpp>


namespace kurento
{
class ModuleInfo;
} /* kurento */

namespace kurento
{
class JsonSerializer;
void Serialize (std::shared_ptr<kurento::ModuleInfo> &object, JsonSerializer &s);
} /* kurento */

namespace kurento
{

class ModuleInfo : public RegisterParent 
{

public:

  ModuleInfo (const std::string &version, const std::string &name, const std::string &generationTime, const std::vector<std::string> &factories) {
    this->version = version;
    this->name = name;
    this->generationTime = generationTime;
    this->factories = factories;
  };

  void setVersion (const std::string &version) {
    this->version = version;
  };

  std::string getVersion () {
    return version;
  };

  void setName (const std::string &name) {
    this->name = name;
  };

  std::string getName () {
    return name;
  };

  void setGenerationTime (const std::string &generationTime) {
    this->generationTime = generationTime;
  };

  std::string getGenerationTime () {
    return generationTime;
  };

  void setFactories (const std::vector<std::string> &factories) {
    this->factories = factories;
  };

  std::vector<std::string> getFactories () {
    return factories;
  };

  virtual void Serialize (JsonSerializer &s);

  static void registerType () {
    std::function<RegisterParent*(void)> func =
        [] () {

      return new ModuleInfo ();

    };

    RegisterParent::registerType ("kurento.ModuleInfo", func);
  }

protected:

  ModuleInfo() {};

private:

  std::string version;
  std::string name;
  std::string generationTime;
  std::vector<std::string> factories;

  friend void kurento::Serialize (std::shared_ptr<kurento::ModuleInfo> &object, JsonSerializer &s);

};

} /* kurento */

#endif /*  __MODULE_INFO_HPP__ */
