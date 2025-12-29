#include <thread>
#include <iostream>
#include "KeyCapture.hpp"
#define MINIAUDIO_IMPLEMENTATION
#include "audio.h"
#include <vector>
// TUI
#include <functional>  
#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"

struct AudioConfig {
    std::atomic<int> volume;
    std::atomic<int> sound_index;
};

int main() {


    std::setvbuf(stdout, NULL, _IONBF, 0);
    hook_set_dispatch_proc(&KeyCapture::dispatch_proc);
    std::thread hook_thread([]() {
        int res = hook_run();
        if (res != UIOHOOK_SUCCESS) {
            std::cerr << "Hook failed with error: " << res << std::endl;
        }
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  
    using namespace ftxui;
    AudioConfig config;
    config.volume = 50;
    config.sound_index = 0;

    std::vector<std::string> sounds = {
        "Cherry MX Blue",
        "Cherry MX Brown",
        "Topre",
        "IBM Model M"
    };

    int selected = 0;
    int volume = 50;

    MenuOption menu_option;
    auto sound_menu = Menu(&sounds, &selected, menu_option);
    SliderOption<int> slider_option;
    auto volume_slider = Slider("Volume", &volume, 0, 100, 1);

    auto ui = Container::Vertical({
        sound_menu,
        volume_slider,
        });

    auto renderer = Renderer(ui, [&] {
        // sync UI → shared state
        config.sound_index.store(selected);
        config.volume.store(volume);

        return vbox({
            text("CyEcho – Mechanical Keyboard Sound") | bold,
            separator(),
            text("Sound profile:"),
            sound_menu->Render(),
            separator(),
            volume_slider->Render(),
            separator(),
            text("Press q to quit"),
            }) | border;
        });

    auto screen = ScreenInteractive::FitComponent();

    renderer = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Character('q')) {
            hook_stop();
            screen.Exit();
            return true;
        }
        return false;
        });

    screen.Loop(renderer);
    if (hook_thread.joinable()) hook_thread.join();

    hook_stop();
  

    return 0;
}


