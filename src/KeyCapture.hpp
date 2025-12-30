#pragma once
#include <uiohook.h>
#include <string>
#include <memory>
#include "Play.hpp"

struct AudioConfig {
    std::atomic<float> volume{ 50.0f };
    std::atomic<int> sound_index{ 0 };
};

namespace Play { class Player; }
namespace KeyCapture
{
    extern AudioConfig config;
    extern std::unique_ptr<Play::Player> global_player;
    std::string get_key_name(unsigned short code);
    void dispatch_proc(uiohook_event* event);
}