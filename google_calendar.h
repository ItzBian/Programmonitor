#pragma once
#include <string>

class GoogleCalendar
{
public:
    GoogleCalendar(const std::string& accessToken);
    void createEvent(const std::string& calendarId, const std::string& summary, const std::string& description,
        const std::string& startTime, const std::string& endTime);

private:
    std::string accessToken;
    std::string calendarEndpoint = "https://www.googleapis.com/calendar/v3/calendars";
};
