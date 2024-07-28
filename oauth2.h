#pragma once
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

class OAuth2
{
public:
    OAuth2(const std::string& clientId, const std::string& clientSecret, const std::string& redirectUri);
    std::string getAuthorizationUrl();
    std::string getAccessToken(const std::string& authorizationCode);

private:
    std::string clientId;
    std::string clientSecret;
    std::string redirectUri;
    std::string tokenEndpoint = "https://oauth2.googleapis.com/token";
};
