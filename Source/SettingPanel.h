#pragma once
#include <vector>
#include "../Library/GuiButton.h"

class SettingPanel {
public:
    SettingPanel();
    ~SettingPanel();

    void Update();
    void Draw();

    // 表示・非表示を切り替え、中のボタンの有効・無効も同期させる
    void SetVisible(bool v);
    bool IsVisible() const { return visible; }

private:
    std::vector<GuiButton*> buttons;
    bool visible;
    int currentSelect;
};