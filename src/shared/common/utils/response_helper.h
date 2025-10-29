#pragma once

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <string>
#include <vector>

namespace rdws::utils {

class ResponseHelper {
  public:
    static std::string returnSuccess(const std::string& message = "", int statusCode = 200,
                                     const ::rapidjson::Value* data = nullptr);

    static std::string returnError(const std::string& message, int statusCode = 500,
                                   const ::rapidjson::Value* details = nullptr);

    static std::string returnData(const ::rapidjson::Value& data, const std::string& message = "",
                                  int statusCode = 200);

    template <typename T>
    static std::string returnEntity(const T& entity, const std::string& entityName,
                                    const std::string& message = "", int statusCode = 200);

    template <typename T>
    static std::string returnEntities(const std::vector<T>& entities,
                                      const std::string& entitiesName,
                                      const std::string& message = "", int statusCode = 200);

  private:
    static void addMetadata(::rapidjson::Document& doc,
                            ::rapidjson::Document::AllocatorType& allocator,
                            const std::string& source = "");

    static std::string documentToString(const ::rapidjson::Document& doc);
};

// Template implementations must be in header
template <typename T>
std::string ResponseHelper::returnEntity(const T& entity, const std::string& entityName,
                                         const std::string& message, int statusCode) {
    ::rapidjson::Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();

    // Convert entity to JSON (assumes entity has a toJsonValue method)
    ::rapidjson::Value entityObj = entity.toJsonValue(allocator);

    doc.AddMember("success", ::rapidjson::Value(true), allocator);
    doc.AddMember("statusCode", ::rapidjson::Value(statusCode), allocator);

    if (!message.empty()) {
        doc.AddMember("message", ::rapidjson::Value(message.c_str(), allocator), allocator);
    }

    doc.AddMember(::rapidjson::Value(entityName.c_str(), allocator), entityObj, allocator);

    addMetadata(doc, allocator);

    return documentToString(doc);
}

template <typename T>
std::string ResponseHelper::returnEntities(const std::vector<T>& entities,
                                           const std::string& entitiesName,
                                           const std::string& message, int statusCode) {
    ::rapidjson::Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();

    ::rapidjson::Value entitiesArray(::rapidjson::kArrayType);
    for (const auto& entity : entities) {
        // Convert entity to JSON (assumes entity has a toJsonValue method)
        ::rapidjson::Value entityObj = entity.toJsonValue(allocator);
        entitiesArray.PushBack(entityObj, allocator);
    }

    doc.AddMember("success", ::rapidjson::Value(true), allocator);
    doc.AddMember("statusCode", ::rapidjson::Value(statusCode), allocator);

    if (!message.empty()) {
        doc.AddMember("message", ::rapidjson::Value(message.c_str(), allocator), allocator);
    }

    doc.AddMember(::rapidjson::Value(entitiesName.c_str(), allocator), entitiesArray, allocator);
    doc.AddMember("total", ::rapidjson::Value(static_cast<int>(entities.size())), allocator);

    addMetadata(doc, allocator);

    return documentToString(doc);
}

} // namespace rdws::utils
