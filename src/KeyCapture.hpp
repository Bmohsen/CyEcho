#pragma once
#include <uiohook.h>
#include <string>

namespace Play { class Player; }
namespace KeyCapture
{
    extern Play::Player global_player; 
    std::string get_key_name(unsigned short code);
    void dispatch_proc(uiohook_event* event);
}