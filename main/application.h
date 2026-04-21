#ifndef _APPLICATION_H_
#define _APPLICATION_H_

// Minimal application stub for display layer compatibility in buddy mode
#include "device_state.h"

class Application {
public:
    static Application& GetInstance() {
        static Application instance;
        return instance;
    }

    DeviceState GetDeviceState() const { return kDeviceStateIdle; }
    bool IsVoiceDetected() const { return false; }
    void PlaySound(const char*) {}

private:
    Application() = default;
};

#endif // _APPLICATION_H_
