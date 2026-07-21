#pragma once
#include "../Library/SceneBase.h"
#include "../Library/GuiButton.h"
#include <string>
#include <vector>

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
    int unlockedTileImg = -1;
    std::string unlockedTileDesc = "";
    bool hasNewUnlock = false;

    std::vector<GuiButton*> buttons;

    void UnlockTileInConfig(int targetId);
};