#pragma once

// Minimal lang_config stub for buddy mode
namespace Lang {
    constexpr const char* CODE = "en-US";
    namespace Strings {
        constexpr const char* INITIALIZING = "Starting...";
        constexpr const char* LISTENING = "Listening";
        constexpr const char* STANDBY = "Standby";
        constexpr const char* SPEAKING = "Speaking";
        constexpr const char* ERROR = "Error";
        constexpr const char* BATTERY_NEED_CHARGE = "Low Battery";
    }
    namespace Sounds {
        constexpr const char* OGG_LOW_BATTERY = nullptr;
    }
}
