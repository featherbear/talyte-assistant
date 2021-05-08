// STD STRINGS !!!
#define RAPIDJSON_HAS_STDSTRING 1

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

std::string _getVersionRequestJSONString();
std::string _getAuthRequiredJSONString();
std::string _generateAuthJSONString(const char* password, const char* challenge, const char* salt);

#define _GetVersionString "GetVersion"
#define _AuthRequiredString "GetAuthRequired"