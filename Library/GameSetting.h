#pragma once
#include <fstream>
#include <string>
#include <DxLib.h>

class GameSetting {
public:
    static int MasterVolumeLevel;
    static const std::string SaveFilePath;

    static void Save() {
        std::ofstream ofs(SaveFilePath, std::ios::binary);
        if (ofs) {
            ofs.write((char*)&MasterVolumeLevel, sizeof(MasterVolumeLevel));
        }
    }

    static void Load() {
        std::ifstream ifs(SaveFilePath, std::ios::binary);
        if (ifs) {
            ifs.read((char*)&MasterVolumeLevel, sizeof(MasterVolumeLevel));
            // ”O‚Ì‚½‚ß”ÍˆÍƒ`ƒFƒbƒN
            if (MasterVolumeLevel < 0) MasterVolumeLevel = 0;
            if (MasterVolumeLevel > 5) MasterVolumeLevel = 5;
        }
    }

    static void Apply(int handle = -1) {
        // 0`5‚ğ0`255‚É•ÏŠ·
        int vol = (255 * MasterVolumeLevel) / 5;
        if (handle != -1) {
            ChangeVolumeSoundMem(vol, handle);
        }
    }
};

inline int GameSetting::MasterVolumeLevel = 3;
inline const std::string GameSetting::SaveFilePath = "settings.dat";