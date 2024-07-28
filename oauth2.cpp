#include "oauth2.h"
#include "logger.h"
#include <sstream>
#include <iostream>

OAuth2::OAuth2(const std::string& clientId, const std::string& clientSecret, const std::string& redirectUri)
    : clientId(clientId), clientSecret(clientSecret), redirectUri(redirectUri)
{
}

std::string OAuth2::getAuthorizationUrl()
{
    std::stringstream ss;
    ss << "https://accounts.google.com/o/oauth2/auth?"
        << "client_id=" << clientId
        << "&redirect_uri=" << redirectUri
        << "&response_type=code"
        << "&scope=https://www.googleapis.com/auth/calendar.events";

    return ss.str();
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string OAuth2::getAccessToken(const std::string& authorizationCode)
{
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
        std::string readBuffer;
        std::string postFields = "code=" + authorizationCode +
            "&client_id=" + clientId +
            "&client_secret=" + clientSecret +
            "&redirect_uri=" + redirectUri +
            "&grant_type=authorization_code";

        curl_easy_setopt(curl, CURLOPT_URL, tokenEndpoint.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK)
        {
            AddLog("Error: Failed to get access token. CURL error: " + std::string(curl_easy_strerror(res)));
            curl_global_cleanup();
            return "";
        }

        auto jsonResponse = nlohmann::json::parse(readBuffer);
        if (jsonResponse.contains("access_token"))
        {
            curl_global_cleanup();
            return jsonResponse["access_token"];
        }
        else
        {
            std::string error = jsonResponse.contains("error") ? jsonResponse["error"] : "Unknown error";
            std::string errorDescription = jsonResponse.contains("error_description") ? jsonResponse["error_description"] : "";
            AddLog("Error: " + error + ". " + errorDescription);
            curl_global_cleanup();
            return "";
        }
    }

    curl_global_cleanup();
    return "";
}
