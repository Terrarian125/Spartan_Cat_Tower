#include "SettingScene.h"
#include "../Library/GuiButton.h"
#include "../Library/GameSetting.h"
#include "../Library/SceneManager.h"
#include "../Library/Input.h"
#include <DxLib.h>
#include "../Library/GameSetting.h"

SettingScene::SettingScene() {
    int bx = 280, bw = 240, bh = 45;

    // 音量を上げる
    auto bUp = new GuiButton(bx, 200, bw, bh, "Volume UP (+)");
    bUp->onClick = []() {
        if (GameSetting::MasterVolumeLevel < 5) {
            GameSetting::MasterVolumeLevel++;
            GameSetting::Save(); // ファイルに保存
            // ※BGMへの反映は各シーン側で行うか、SoundManagerがあればそこで行う
        }
        };
    buttons.push_back(bUp);

    // 音量を下げる
    auto bDown = new GuiButton(bx, 260, bw, bh, "Volume DOWN (-)");
    bDown->onClick = []() {
        if (GameSetting::MasterVolumeLevel > 0) {
            GameSetting::MasterVolumeLevel--;
            GameSetting::Save(); // ファイルに保存
        }
        };
    buttons.push_back(bDown);

    // 戻る
    auto bBack = new GuiButton(bx, 400, bw, bh, "Back to Title");
    bBack->onClick = []() {
        SceneManager::ChangeScene("TITLE");
        };
    buttons.push_back(bBack);
}

SettingScene::~SettingScene() {}

void SettingScene::Update() {
    // キーボード操作
    if (Input::IsKeyDown(KEY_INPUT_DOWN)) currentSelect = (currentSelect + 1) % buttons.size();
    if (Input::IsKeyDown(KEY_INPUT_UP)) currentSelect = (currentSelect - 1 + (int)buttons.size()) % (int)buttons.size();

    for (int i = 0; i < (int)buttons.size(); i++) {
        buttons[i]->SetFocus(i == currentSelect);
        if (buttons[i]->IsMouseOver()) currentSelect = i;
    }
}

void SettingScene::Draw() {
    DrawString(280, 150, "--- SYSTEM SETTINGS ---", GetColor(255, 255, 255));

    // 現在の音量レベルを視覚化（★部分）
    std::string volBar = "Volume: ";
    for (int i = 0; i < 5; i++) volBar += (i < GameSetting::MasterVolumeLevel) ? "■" : "□";
    DrawString(320, 330, volBar.c_str(), GetColor(255, 255, 100));
}