#include "SettingPanel.h"
#include "../Library/GameSetting.h"
#include "../Library/Input.h"
#include <DxLib.h>

SettingPanel::SettingPanel() : visible(false), currentSelect(0) {
    int bx = 850, bw = 300, bh = 60, startY = 250;

    auto bUp = new GuiButton(bx, startY, bw, bh, "Volume UP (+)");
    bUp->onClick = []() {
        if (GameSetting::MasterVolumeLevel < 5) {
            GameSetting::MasterVolumeLevel++;
            GameSetting::Save();
        }
        };
    buttons.push_back(bUp);

    auto bDown = new GuiButton(bx, startY + 80, bw, bh, "Volume DOWN (-)");
    bDown->onClick = []() {
        if (GameSetting::MasterVolumeLevel > 0) {
            GameSetting::MasterVolumeLevel--;
            GameSetting::Save();
        }
        };
    buttons.push_back(bDown);

    auto bClose = new GuiButton(bx, startY + 200, bw, bh, "CLOSE");
    bClose->onClick = [this]() {
        this->SetVisible(false);
        };
    buttons.push_back(bClose);

    SetVisible(false);
}

void SettingPanel::SetVisible(bool v) {
    visible = v;
    for (auto b : buttons) b->SetActive(v);
    currentSelect = 0;
}

void SettingPanel::Update() {
    if (!visible) return;

    if (Input::IsKeyDown(KEY_INPUT_DOWN)) currentSelect = (currentSelect + 1) % buttons.size();
    if (Input::IsKeyDown(KEY_INPUT_UP)) currentSelect = (currentSelect - 1 + (int)buttons.size()) % (int)buttons.size();

    for (int i = 0; i < (int)buttons.size(); i++) {
        buttons[i]->SetFocus(i == currentSelect);
        if (buttons[i]->IsMouseOver()) currentSelect = i;
    }
}

void SettingPanel::Draw() {
    if (!visible) return;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    DrawBox(800, 0, 1280, 720, GetColor(10, 10, 15), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    DrawLine(800, 0, 800, 720, GetColor(255, 255, 255));

    DrawFormatString(850, 150, GetColor(255, 255, 255), "--- SETTINGS ---");
    std::string volBar = "VOL: ";
    for (int i = 0; i < 5; i++) volBar += (i < GameSetting::MasterVolumeLevel) ? "■" : "□";
    DrawFormatString(850, 400, GetColor(255, 255, 100), volBar.c_str());
}

SettingPanel::~SettingPanel() {

}