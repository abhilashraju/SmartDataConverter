#pragma once
#include <nlohmann/json.hpp>
namespace redfish{
    namespace messages{
        void setInternalError(nlohmann::json& j)
        {    
            
            auto js=R"({"error":"internal error occured"})"_json;
            
            j.merge_patch(js);
            
        }
    }
}