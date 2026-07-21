#pragma once
#include "../Library/SceneBase.h"
#include "../Library/GuiButton.h"
#include <string>
#include <vector>

//解放されたパーツの情報構造体
struct UnlockedItemInfo {
    int imgHandle = -1;
    std::string desc = "";
};

class ClearScene : public SceneBase
{
public:
    ClearScene();
    ~ClearScene();
    void Update() override;
    void Draw() override;

private:
    float alpha;
    float fadeSpeed;
    int hSound;

    //けいふぉんと＆アンロック演出用
    int fontHandle = -1;
    std::vector<UnlockedItemInfo> newUnlocks; //複数パーツの情報リスト

    std::vector<GuiButton*> buttons;

    void UnlockTilesInConfig(const std::vector<int>& targetIds);
};