#include "protoTools.hpp"

#include <base64.h>
#include <picosha2.h>

/**
 * Generate JSON for a version request
 */
std::string _getVersionRequestJSONString() {
    rapidjson::Document d;
    d.SetObject();
    rapidjson::Document::AllocatorType& allocation = d.GetAllocator();
    d.AddMember("request-type", _GetVersionString, allocation);
    d.AddMember("message-id", _GetVersionString, allocation);
    rapidjson::StringBuffer message;
    rapidjson::Writer<rapidjson::StringBuffer> writer(message);
    d.Accept(writer);
    return std::string(message.GetString());
}

/**
 * Generate JSON for an auth required check
 */
std::string _getAuthRequiredJSONString() {
    rapidjson::Document d;
    d.SetObject();
    rapidjson::Document::AllocatorType& allocation = d.GetAllocator();
    d.AddMember("request-type", _AuthRequiredString, allocation);
    d.AddMember("message-id", _AuthRequiredString, allocation);
    rapidjson::StringBuffer message;
    rapidjson::Writer<rapidjson::StringBuffer> writer(message);
    d.Accept(writer);
    return std::string(message.GetString());
}

/**
 * base64(sha256(A+B))
 */
std::string ___encHash(const char* A, const char* B) {
    std::string concat(std::string(A) + std::string(B));
    std::vector<unsigned char> hash(picosha2::k_digest_size);
    picosha2::hash256(concat.begin(), concat.end(), hash.begin(), hash.end());
    return base64_encode(std::string(hash.begin(), hash.end()));
}

/**
 * base64(sha256(base64(sha256(password + salt)) + challenge)) 
 */
std::string __generateAuthResponse(const char* password, const char* challenge, const char* salt) {
    return ___encHash(___encHash(password, salt).c_str(), challenge);
}

/**
 * Generate JSON for an auth request
 */
std::string _generateAuthJSONString(const char* password, const char* challenge, const char* salt) {
    rapidjson::Document d;
    d.SetObject();
    rapidjson::Document::AllocatorType& allocation = d.GetAllocator();
    d.AddMember("auth", __generateAuthResponse(password, challenge, salt), allocation);
    d.AddMember("message-id", _AuthenticateString, allocation);
    d.AddMember("request-type", _AuthenticateString, allocation);
    rapidjson::StringBuffer message;
    rapidjson::Writer<rapidjson::StringBuffer> writer(message);
    d.Accept(writer);
    return std::string(message.GetString());
}