#ifndef _LED_H_
#define _LED_H_

class Led {
public:
    virtual ~Led() = default;
    virtual void OnStateChanged() = 0;
    virtual void StartContinuousBlink(int interval_ms = 500) {}
    virtual void BlinkOnce(int duration_ms = 200) {}
    virtual void Blink(int count = 1, int interval_ms = 200) {}
    virtual void TurnOff() {}
};


class NoLed : public Led {
public:
    virtual void OnStateChanged() override {}
};

#endif // _LED_H_
