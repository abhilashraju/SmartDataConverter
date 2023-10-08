#include "dbus_tree_parser.hpp"

#include "dbus_tree_generator.hpp"
#include <iostream>
using namespace dbus::utility;
/***************Mock for dbus data begin********************************/
enum class OriginatorTypes {
  Invalid,
  Client,
  Internal,
  SupportingService,
};
NLOHMANN_JSON_SERIALIZE_ENUM(OriginatorTypes,
                             {
                                 {OriginatorTypes::Invalid, "Invalid"},
                                 {OriginatorTypes::Client, "Client"},
                                 {OriginatorTypes::Internal, "Internal"},
                                 {OriginatorTypes::SupportingService,
                                  "SupportingService"},
                             });
inline OriginatorTypes
mapDbusOriginatorTypeToRedfish(const std::string &originatorType) {
  if (originatorType ==
      "xyz.openbmc_project.Common.OriginatedBy.OriginatorTypes.Client") {
    return OriginatorTypes::Client;
  }
  if (originatorType ==
      "xyz.openbmc_project.Common.OriginatedBy.OriginatorTypes.Internal") {
    return OriginatorTypes::Internal;
  }
  if (originatorType == "xyz.openbmc_project.Common.OriginatedBy."
                        "OriginatorTypes.SupportingService") {
    return OriginatorTypes::SupportingService;
  }
  return OriginatorTypes::Invalid;
}
auto makeEntries(auto path) {
  DBusPropertiesMap licence_entry = {
      DBusPropertiesMap::value_type{"Name", DbusVariantType("some name")},
      {"AuthDeviceNumber", uint32_t(32)},
      {"ExpirationTime", uint64_t(3434324234)},
      {"SampleVec", AssociationsValType{{"he", "ll", "o"}, {"hi", "by", "e"}}},
      {"SampleBool", true},
      // {"SerialNumber", "3434324234"}
  };
  DBusPropertiesMap log_entry = {DBusPropertiesMap::value_type{
      "OriginatorType",
      DbusVariantType(
          "xyz.openbmc_project.Common.OriginatedBy.OriginatorTypes.Internal")}};
  DBusPropertiesMap availability_entry = {
      DBusPropertiesMap::value_type{
          "Available", DbusVariantType(std::vector<uint32_t>{1, 2})},
      DBusPropertiesMap::value_type{
          "New Prop", DbusVariantType(std::vector<uint32_t>{1, 2})}};
  DBusPropertiesMap cableEntry = {
      DBusPropertiesMap::value_type{
          "CableTypeDescription", DbusVariantType(std::string("Super Cable"))},
      {"LengthMeters", DbusVariantType(3.45)},
      {"OriginatorType",
       DbusVariantType(
           "xyz.openbmc_project.Common.OriginatedBy.OriginatorTypes.Internal")},
      {"Available", DbusVariantType(std::vector<uint32_t>{1, 2})}};

  return ManagedObjectType::value_type{
      sdbusplus::message::object_path(path),
      {{"xyz.openbmc_project.License.Entry.LicenseEntry", licence_entry},
       {"xyz.openbmc_project.State.Decorator.Availability", availability_entry},
       {"xyz.openbmc_project.Common.OriginatedBy", log_entry},
       {"xyz.openbmc_project.Inventory.Item.Cable", cableEntry},
       {"xyz.openbmc_project.State.Decorator.Availability2",
        availability_entry}}};
}
/*****************************mock for dbus data
 * ends***************************/

struct VariantHandler {
  void operator()(auto &var) { std::cout << var << "\n"; }
  void operator()(
      const std::vector<std::tuple<std::string, std::string, std::string>>
          &var) {}
  template <typename T> void operator()(const std::vector<T> &var) {}
  void operator()(const sdbusplus::message::object_path &) {}
  void operator()(
      const std::tuple<unsigned long long,
                       std::vector<std::tuple<std::string, std::string, double,
                                              unsigned long long>>> &) {}
};
int main() {
  auto entry1 = makeEntries("/xyz/openbmc_project/license/entry1/");
  auto entry2 = makeEntries("/xyz/openbmc_project/license/entry2/");
  ManagedObjectType resp = {entry1, entry2};

  DbusBaseMapper mapper;

  mapper.addMap("CableTypeDescription",
                mapToKeyOrError<std::string>("@CableType"));
  mapper.addMap("OriginatorType",
                mapToEnumKey<OriginatorTypes>("OriginatorType",
                                              mapDbusOriginatorTypeToRedfish));

  mapper.addMap(
      "LengthMeters",
      mapToKeyOrError<double, std::string>("LengthMeters", [](auto length) {
        if (std::isfinite(length) && std::isnan(length)) {
          return std::optional<std::string>();
        }
        return std::optional(std::string("Hello"));
      }));
  mapper.addMap("Available",
                mapToKeyOrError<std::vector<uint32_t>, nlohmann::json>(
                    "Cable", [](auto val) {
                      nlohmann::json j;
                      j["Availability"] = val;
                      return std::optional<nlohmann::json>(j);
                    }));

  nlohmann::json result;

  auto propMap =
      getInterface(getObject(resp, "/xyz/openbmc_project/license/entry2/"),
                   "xyz.openbmc_project.Inventory.Item.Cable");
  std::cout << "\n\ntesting Legacy  parser "
               "************************************************\n\n";
  DbusTreeParser(mapper, true)
      .parse(propMap, [&result](DbusParserStatus status, const auto &summary) {
        result = summary;
      });

  std::cout << result.dump(4);
  std::cout << "\n\ntesting Dbus tree generator "
               "************************************************\n\n";
  auto treeGen = reactor::Flux<DbusTreeGenerator::value_type>::generate(
      DbusTreeGenerator(resp));
  treeGen
      .filter([](const DbusTreeGenerator::value_type &v) {
        return std::get<0>(v) == "/xyz/openbmc_project/license/entry2/";
      })
      .map(
          [](const DbusTreeGenerator::value_type &v) { return std::get<3>(v); })
      .filter([](const DbusVariantType &v) {
        return std::holds_alternative<std::string>(v);
      })
      .subscribe(
          [](const DbusVariantType &v) { std::visit(VariantHandler{}, v); });

  std::cout << "\n\ntesting property map generator "
               "************************************************\n\n";
  auto propgen = reactor::Flux<DbusPropertyListGenerator::value_type>::generate(
      DbusPropertyListGenerator{*propMap.value()});
  propgen.subscribe([](const auto &v) {
    std::cout << std::get<0>(v) << " ";
    std::visit(VariantHandler{}, std::get<1>(v));
  });

  std::cout << "\n\ntesting DbusInterfaceList generator "
               "************************************************\n\n";
  auto ifacdList =
      getObject(resp, "/xyz/openbmc_project/license/entry2/").value();
  auto iFaceListGen =
      reactor::Flux<DbusInterfaceListGenerator::value_type>::generate(
          DbusInterfaceListGenerator{*(ifacdList)});
  iFaceListGen.subscribe([](const auto &v) {
    std::cout << std::get<0>(v) << " " << std::get<1>(v) << " ";
    std::visit(VariantHandler{}, std::get<2>(v));
  });
}
