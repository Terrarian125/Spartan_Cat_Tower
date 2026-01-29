#pragma once

#include <string>
#include <ctime>
#include <DxLib.h>

struct GameInfo {
    // リストに表示・管理するための情報
    std::string commandName;      // 内部識別用コマンド名 (例: "game_a")
    std::string displayName;      // 表示用ゲーム名
    std::string execPath;         // 実行ファイルパス または URL

    // UI表示用
    int iconHandle = -1;          // DxLib用アイコン画像ハンドル

    // ソート・管理用
    time_t registrationDate = 0;  // 登録日 (エポック秒)
    time_t lastPlayedDate = 0;    // 最後に遊んだ日 (エポック秒)
};