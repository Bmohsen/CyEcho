#pragma once
#include "audio.h"
#include "json.hpp"
#include <map>
#include <string>

namespace Play {
	struct KeyTiming {
		uint64_t start_ms;
		uint64_t duration_ms;
	};

	class Player {
	public:
		Player(const std::string& folderPath);
		~Player();
		void playKey(unsigned short uihook_code);
		void loadProfile(const std::string& folderPath);
	private:
		ma_engine engine;
		ma_sound atlas;
		std::map<std::string, KeyTiming> timings;
		void loadConfig(const std::string& path);
		static const int MAX_VOICES = 32;
		ma_sound voices[MAX_VOICES];
		bool voiceActive[MAX_VOICES] = { false };
	};
}