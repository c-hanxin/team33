#pragma once

#include <string>

struct AppEvent {
    std::string type;
    int payloadInt{0};
    bool payloadBool{false};
    float payloadFloat{0.0f};
    std::string payloadString;
};
