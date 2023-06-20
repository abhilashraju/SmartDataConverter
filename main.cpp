#include "dbus_tree_parser.hpp"
#include <iostream>
using namespace dbus::utility;
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
struct Extraction_Handlers : DbusBaseMapper
    {
        
        
        Extraction_Handlers()
        {
           
            addInterfaceHandler("xyz.openbmc_project.License.Entry.LicenseEntry","Name",mapToKey("Abhilash"));
            addInterfaceHandler("xyz.openbmc_project.License.Entry.LicenseEntry",
                "AuthDeviceNumber", mapToKey<uint32_t>("AuthDeviceNumber"));
            addInterfaceHandler("xyz.openbmc_project.License.Entry.LicenseEntry",
                "SerialNumber", mapToKey<std::string>("Abilash/New Key",[](auto s ,MapperResult& mr,MetaData& md){
                     
                    if(mr != MapperResult::Ok){
                        md.res=R"({"error":"some error"})"_json;
                        return std::string();
                    }
                    return s;
                }));
            addInterfaceHandler("xyz.openbmc_project.License.Entry.LicenseEntry",
                "SampleVec", mapToKey<AssociationsValType>("New Key/Old Key",[](std::string_view path, const auto& v){
                    nlohmann::json j;
                     j["New Key"]["Old Key"]=v;
                     return j;
                }));
            

            std::vector<handler_pair> AvailabilityExtractor;
            addInterfaceHandler("xyz.openbmc_project.State.Decorator.Availability",
                "Available",
                mapToKeyOrError<std::vector<uint32_t>,nlohmann::json>("",
                    [](auto val) {
                    nlohmann::json j;
                    j["Availability"] = val;
                    return std::optional(j);
                
                }));
                
            addInterfaceHandler("xyz.openbmc_project.State.Decorator.Availability",
                             "New Prop",mapToKey("prop"));
            addInterfaceHandler("xyz.openbmc_project.Common.OriginatedBy",
                             "OriginatorType",mapToEnumKey<OriginatorTypes>("Type",mapDbusOriginatorTypeToRedfish));
        }
    };

struct CablePropertieMappers : DbusBaseMapper
    {          
        CablePropertieMappers()
        {
            addInterfaceHandler("xyz.openbmc_project.Inventory.Item.Cable",
                "CableTypeDescription", mapToKeyOrError<std::string>("CableType"));
            addInterfaceHandler("xyz.openbmc_project.Inventory.Item.Cable",
                "LengthMeters", mapToKey<double>("LengthMeters")); 
                // ,[](auto length){ 
                //     if (std::isfinite(length) && std::isnan(length))
                //     {
                //         return std::optional(length);   
                //     }
                //     return std::optional<double>();
                // })); 
                
        }
    };
    
int main()
{
    auto entry1 = makeEntries("/xyz/openbmc_project/license/entry1/");
    auto entry2 = makeEntries("/xyz/openbmc_project/license/entry2/");
    ManagedObjectType resp = {entry1, entry2};

    
    Extraction_Handlers extraction_handlers;
    CablePropertieMappers cableMappers;
    
    DbusBaseMapper mapper;
    mapper.addMap("CableTypeDescription", mapToKeyOrError<std::string>("@CableType"))
          .addMap("LengthMeters", mapToKeyOrError<double,std::string>("LengthMeters",[](auto length){ 
                    if (std::isfinite(length) && std::isnan(length))
                    {
                        return std::optional<std::string>();   
                    }
                    return std::optional(std::string("Hello"));
                })); 
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
    
    DbusTreeParser(mapper,true)
    .parse(propMap,[ &result](DbusParserStatus status, const auto& summary) {
            result = summary;
    });
    
    std::cout<<result.dump(4);
   
    
}

