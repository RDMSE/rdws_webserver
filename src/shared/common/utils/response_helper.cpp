#include "response_helper.h"

#include <ctime>

namespace rdws::utils {

std::string ResponseHelper::returnSuccess(const std::string& message, int statusCode,
                                          const ::rapidjson::Value* data) {
    ::rapidjson::Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();

    doc.AddMember("success", ::rapidjson::Value(true), allocator);
    doc.AddMember("statusCode", ::rapidjson::Value(statusCode), allocator);

    if (!message.empty()) {
        doc.AddMember("message", ::rapidjson::Value(message.c_str(), allocator), allocator);
    }

    if (data) {
        ::rapidjson::Value dataCopy(*data, allocator);
        doc.AddMember("data", dataCopy, allocator);
    }

    addMetadata(doc, allocator);

    return documentToString(doc);
}

std::string ResponseHelper::returnError(const std::string& message, int statusCode,
                                        const ::rapidjson::Value* details) {
    ::rapidjson::Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();

    doc.AddMember("success", ::rapidjson::Value(false), allocator);
    doc.AddMember("statusCode", ::rapidjson::Value(statusCode), allocator);
    doc.AddMember("error", ::rapidjson::Value(message.c_str(), allocator), allocator);

    if (details) {
        ::rapidjson::Value detailsCopy(*details, allocator);
        doc.AddMember("details", detailsCopy, allocator);
    }

    addMetadata(doc, allocator);

    return documentToString(doc);
}

std::string ResponseHelper::returnData(const ::rapidjson::Value& data, const std::string& message,
                                       int statusCode) {
    ::rapidjson::Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();

    doc.AddMember("success", ::rapidjson::Value(true), allocator);
    doc.AddMember("statusCode", ::rapidjson::Value(statusCode), allocator);

    if (!message.empty()) {
        doc.AddMember("message", ::rapidjson::Value(message.c_str(), allocator), allocator);
    }

    ::rapidjson::Value dataCopy(data, allocator);
    doc.AddMember("data", dataCopy, allocator);

    addMetadata(doc, allocator);

    return documentToString(doc);
}

void ResponseHelper::addMetadata(::rapidjson::Document& doc,
                                 ::rapidjson::Document::AllocatorType& allocator,
                                 const std::string& source) {
    std::string src = source.empty() ? "microservice C++ with PostgreSQL" : source;
    doc.AddMember("source", ::rapidjson::Value(src.c_str(), allocator), allocator);
    doc.AddMember("timestamp", ::rapidjson::Value(static_cast<int64_t>(std::time(nullptr))),
                  allocator);
}

std::string ResponseHelper::documentToString(const ::rapidjson::Document& doc) {
    ::rapidjson::StringBuffer buffer;
    ::rapidjson::Writer<::rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}

} // namespace rdws::utils
