#include "Play.hpp"
#include <fstream>
#include <iostream>
#include "KeyCapture.hpp"
#include <filesystem>
#include <algorithm>

namespace Play {

	namespace fs = std::filesystem;

	Player::Player(const std::string& folderPath) {
		ma_result result = ma_engine_init(NULL, &engine);
		if (result != MA_SUCCESS) {
			std::cerr << "Engine Init Failed: " << result << std::endl;
			return;
		}
		auto fullPath = (std::filesystem::current_path() / folderPath / "sound.flac").make_preferred();
		std::string pathString = fullPath.string();
		result = ma_sound_init_from_file(&engine, pathString.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &atlas);
		if (result != MA_SUCCESS) {
			std::cerr << "Atlas Load Failed (Error " << result << "): " << pathString << std::endl;
			return;
		}

		ma_engine_start(&engine);

		// 3. Initialize voices only if atlas is valid
		for (int i = 0; i < MAX_VOICES; i++) {
			ma_sound_init_copy(&engine, &atlas, 0, NULL, &voices[i]);
		}

		loadConfig((std::filesystem::current_path() / folderPath / "config.json").string());
	}


	Player::~Player() {
		ma_engine_stop(&engine);
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

	void Player::playKey(unsigned short uihook_code, float currentVolume) {
		if (atlas.pResourceManagerDataSource == nullptr) return;

		std::string keyName = KeyCapture::get_key_name(uihook_code);
		if (timings.count(keyName)) {
			KeyTiming timing = timings[keyName];
			ma_uint32 sampleRate = ma_engine_get_sample_rate(&engine);
			ma_uint64 startFrame = (timing.start_ms * sampleRate) / 1000;
			ma_uint64 durationFrames = (timing.duration_ms * sampleRate) / 1000;
			for (int i = 0; i < MAX_VOICES; i++) {
				if (!ma_sound_is_playing(&voices[i])) {
					// Apply volume from the atomic config
					ma_sound_set_volume(&voices[i], currentVolume / 100.0f);
					ma_sound_seek_to_pcm_frame(&voices[i], startFrame);

					// Use the engine's current time to schedule the stop
					ma_uint64 engineTime = ma_engine_get_time(&engine);
					ma_sound_set_stop_time_in_pcm_frames(&voices[i], engineTime + durationFrames);

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
		std::string oggPath = (std::filesystem::current_path() / folderPath / "sound.flac").string();
		std::string jsonPath = (std::filesystem::current_path() / folderPath / "config.json").string();
		ma_sound_init_from_file(&engine, oggPath.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &atlas);
		loadConfig(jsonPath);
		for (int i = 0; i < MAX_VOICES; i++) {
			ma_sound_init_copy(&engine, &atlas, 0, NULL, &voices[i]);
		}
		ma_engine_start(&engine);
	}
}