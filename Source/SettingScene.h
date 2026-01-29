#pragma once
#include "../Library/SceneBase.h"
#include <vector>

class GuiButton;

class SettingScene : public SceneBase {
public:
    SettingScene();
    ~SettingScene();
    void Update() override;
    void Draw() override;

private:
    std::vector<GuiButton*> buttons;
    int currentSelect = 0;
};