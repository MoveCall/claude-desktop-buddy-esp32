#pragma once

#include "tama_state.h"
#include "persona.h"
#include "../storage/buddy_nvs.h"

class BuddyApp {
public:
    static BuddyApp& GetInstance();
    void Initialize();
    void Run();

    TamaState& GetState() { return state_; }
    PersonaState GetPersona() const { return active_state_; }
    BuddyStats& GetStats() { return stats_; }
    BuddySettings& GetSettings() { return settings_; }

    void Approve();
    void Deny();
    void NotifyActivity();
    bool IsScreenOff() const;

private:
    void OnButtonClick();
    void OnButtonLongPress();
    void UpdatePersona();

    TamaState state_ = {};
    BuddyStats stats_ = {};
    BuddySettings settings_ = {};
    PersonaState base_state_ = PersonaState::IDLE;
    PersonaState active_state_ = PersonaState::IDLE;
    uint32_t one_shot_until_ = 0;
    PersonaState one_shot_state_ = PersonaState::IDLE;
    bool approval_sent_ = false;
    uint32_t prompt_start_ms_ = 0;
    uint32_t last_activity_ms_ = 0;
};
