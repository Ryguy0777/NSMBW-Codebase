#pragma once
#include <new/game_config.h>

#ifdef KOOPATLAS_ENABLED
class dKPMusic_c {
public:
	static bool loadInfo();
	static void play(int id);
	static void execute();
	static void stop();
	static void playStarMusic();
};
#endif