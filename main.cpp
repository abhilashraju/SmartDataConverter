#include "dbus_tree_parser.hpp"
#include <iostream>
using namespace dbus::utility;
/***************Mock for dbus data begin********************************/
enum class OriginatorTypes{
    Invalid,
    Client,
    Internal,
    SupportingService,
};
NLOHMANN_JSON_SERIALIZE_ENUM(OriginatorTypes, {
    {OriginatorTypes::Invalid, "Invalid"},
    {OriginatorTypes::Client, "Client"},
    {OriginatorTypes::Internal, "Internal"},
    {OriginatorTypes::SupportingService, "SupportingService"},
});
inline OriginatorTypes
    mapDbusOriginatorTypeToRedfish(const std::string& originatorType)
{
    if (originatorType ==
        "xyz.openbmc_project.Common.OriginatedBy.OriginatorTypes.Client")
    {
        return OriginatorTypes::Client;
    }
    if (originatorType ==
        "xyz.openbmc_project.Common.OriginatedBy.OriginatorTypes.Internal")
    {
        return OriginatorTypes::Internal;
    }
    if (originatorType ==
        "xyz.openbmc_project.Common.OriginatedBy.OriginatorTypes.SupportingService")
    {
        return OriginatorTypes::SupportingService;
    }
    return OriginatorTypes::Invalid;
}
auto makeEntries(auto path)
{
    DBusPropertiesMap licence_entry = {
        DBusPropertiesMap::value_type{"Name",
                                          DbusVariantType("some name")},
        {"AuthDeviceNumber", uint32_t(32)},
        {"ExpirationTime", uint64_t(3434324234)},
        {"SampleVec",
         AssociationsValType{{"he", "ll", "o"}, {"hi", "by", "e"}}},
        {"SampleBool", true},
        // {"SerialNumber", "3434324234"}
        };
    DBusPropertiesMap log_entry = {
        DBusPropertiesMap::value_type{"OriginatorType",
                                          DbusVariantType("xyz.openbmc_project.Common.OriginatedBy.OriginatorTypes.Internal")}};
    DBusPropertiesMap availability_entry = {
        DBusPropertiesMap::value_type{
            "Available", DbusVariantType(std::vector<uint32_t>{1, 2})},
            DBusPropertiesMap::value_type{
            "New Prop", DbusVariantType(std::vector<uint32_t>{1, 2})}};
    DBusPropertiesMap cableEntry = {
        DBusPropertiesMap::value_type{
            "CableTypeDescription", DbusVariantType(std::string("Super Cable"))},
            {"LengthMeters",DbusVariantType(3.45)},
            {"Available", DbusVariantType(std::vector<uint32_t>{1, 2})}};
    
    return ManagedObjectType::value_type{
        sdbusplus::message::object_path(path),
        {{"xyz.openbmc_project.License.Entry.LicenseEntry", licence_entry},
         {"xyz.openbmc_project.State.Decorator.Availability",
          availability_entry},
          {"xyz.openbmc_project.Common.OriginatedBy",log_entry},
          {"xyz.openbmc_project.Inventory.Item.Cable",cableEntry},
         {"xyz.openbmc_project.State.Decorator.Availability2",
          availability_entry}}};
}
/*****************************mock for dbus data ends***************************/
    
int main()
{
    auto entry1 = makeEntries("/xyz/openbmc_project/license/entry1/");
    auto entry2 = makeEntries("/xyz/openbmc_project/license/entry2/");
    ManagedObjectType resp = {entry1, entry2};

    /*create a mapper object and configure it with required mappings*/
    DbusBaseMapper mapper;
    /* simple mapping from debus string property to redfish attribute
    This will be the mejority cases*/
    mapper.addMap("CableTypeDescription", mapToKeyOrError<std::string>("@CableType"));
    /* a non trivial mapping where some transformation is required before 
    added as redfish attributes. In case of errors due missing dbus property or 
    type missmatch or value bound issues redfish json with be added with error attributes
    automatically with details of the erros as description*/
    mapper.addMap("LengthMeters", mapToKeyOrError<double,std::string>("LengthMeters",[](auto length){ 
                    if (std::isfinite(length) && std::isnan(length))
                    {
                        return std::optional<std::string>();   
                    }
                    return std::optional(std::string("Hello"));
                })); 
    /* As above but returing a json sub structure which evenautally merge with final json*/
    mapper.addMap("Available",
                mapToKeyOrError<std::vector<uint32_t>,nlohmann::json>("",
                    [](auto val) {
                    nlohmann::json j;
                    j["Availability"] = val;
                    return std::optional<nlohmann::json>(j);
                }));
    nlohmann::json result;
    auto obj=resp[0].second;
    auto propMap = obj[3].second;
    
    /*The parser that take above mapper object and dbus data then  parses the dbus data 
     based on supplied mapper . The results will be given in a callback as summary json*/
    DbusTreeParser(mapper,true)
    .parse(propMap,[ &result](DbusParserStatus status, const auto& summary) {
            result = summary;
    });
    
    std::cout<<result.dump(4);
   
    
}

