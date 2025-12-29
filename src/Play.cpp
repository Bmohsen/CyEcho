#include "Play.hpp"
#include <fstream>
#include <iostream>
#include "KeyCapture.hpp"
#include <filesystem>
#include <algorithm>

namespace Play {

	namespace fs = std::filesystem;

	Player::Player(const std::string& folderPath) {
		ma_engine_init(NULL, &engine);
		std::string oggPath = (std::filesystem::current_path() / folderPath / "sound.wav").string();
		ma_sound_init_from_file(&engine, oggPath.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &atlas);

		// Voice Pools 
		for (int i = 0; i < MAX_VOICES; i++) {
			ma_sound_init_copy(&engine, &atlas, 0, NULL, &voices[i]);
		}

		ma_engine_start(&engine);
		loadConfig((std::filesystem::current_path() / folderPath / "config.json").string());
	}

	Player::~Player() {
		for (int i = 0; i < MAX_VOICES; i++) {
			ma_sound_uninit(&voices[i]);
		}
		ma_sound_uninit(&atlas);
		ma_engine_uninit(&engine);
	}

	void Player::loadConfig(const std::string& path) {
		std::ifstream f(path);
		if (!f.is_open()) {
			std::cerr << "Could not open JSON: " << path << std::endl;
			return;
		}
		nlohmann::json data = nlohmann::json::parse(f);
		for (auto& [key, val] : data["defines"].items()) {
			timings[key] = { val[0].get<uint64_t>(), val[1].get<uint64_t>() };
		}
	}

	void Player::playKey(unsigned short uihook_code) {
		std::string keyName = KeyCapture::get_key_name(uihook_code);
		if (timings.count(keyName)) {
			KeyTiming timing = timings[keyName];
			ma_uint32 sampleRate = ma_engine_get_sample_rate(&engine);
			ma_uint64 startFrame = (timing.start_ms * sampleRate) / 1000;
			ma_uint64 durationFrames = (timing.duration_ms * sampleRate) / 1000;
			for (int i = 0; i < MAX_VOICES; i++) {
				if (!ma_sound_is_playing(&voices[i])) {
					ma_sound_set_volume(&voices[i], 1.0f);
					ma_sound_seek_to_pcm_frame(&voices[i], startFrame);
					ma_uint64 engineTime = ma_engine_get_time(&engine);
					ma_uint64 stopTime = engineTime + durationFrames;
					ma_sound_set_stop_time_in_pcm_frames(&voices[i], stopTime);
					//ma_sound_set_fade_in_pcm_frames(&voices[i], 1.0f, 0.0f, durationFrames);
					ma_sound_start(&voices[i]);
					return;
				}
			}
		}
	}

	void Player::loadProfile(const std::string& folderPath) {	
		ma_engine_stop(&engine);
		for (int i = 0; i < MAX_VOICES; i++) {
			ma_sound_uninit(&voices[i]);
		}
		ma_sound_uninit(&atlas);
		timings.clear();
		std::string oggPath = (std::filesystem::current_path() / folderPath / "sound.wav").string();
		std::string jsonPath = (std::filesystem::current_path() / folderPath / "config.json").string();
		ma_sound_init_from_file(&engine, oggPath.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &atlas);
		loadConfig(jsonPath);
		for (int i = 0; i < MAX_VOICES; i++) {
			ma_sound_init_copy(&engine, &atlas, 0, NULL, &voices[i]);
		}
		ma_engine_start(&engine);
	}
}