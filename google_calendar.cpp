#include "google_calendar.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "logger.h"

GoogleCalendar::GoogleCalendar(const std::string& accessToken)
    : accessToken(accessToken)
{
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void GoogleCalendar::createEvent(const std::string& calendarId, const std::string& summary, const std::string& description,
    const std::string& startTime, const std::string& endTime)
{
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
        std::string readBuffer;
        std::string url = calendarEndpoint + "/" + calendarId + "/events";
        nlohmann::json eventJson;
        eventJson["summary"] = summary;
        eventJson["description"] = description;
        eventJson["start"]["dateTime"] = startTime;
        eventJson["end"]["dateTime"] = endTime;

        std::string eventData = eventJson.dump();

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, eventData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK)
        {
            AddLog("Error: Failed to create event. CURL error: " + std::string(curl_easy_strerror(res)));
            return;
        }

        auto jsonResponse = nlohmann::json::parse(readBuffer);
        if (jsonResponse.contains("id"))
        {
            AddLog("Event created successfully. Event ID: " + jsonResponse["id"].get<std::string>());
        }
        else
        {
            std::string error = jsonResponse.contains("error") ? jsonResponse["error"] : "Unknown error";
            std::string errorDescription = jsonResponse.contains("error_description") ? jsonResponse["error_description"] : "";
            AddLog("Error: " + error + ". " + errorDescription);
        }
    }

    curl_global_cleanup();
}
