#pragma once

#include <string>

struct EngineEvent {
    int eventType{0};
    std::string type;
    int payloadInt{0};
    bool payloadBool{false};
    float payloadFloat{0.0f};
};
