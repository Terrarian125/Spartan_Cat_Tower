#pragma once
#include "../Library/SceneBase.h"
#include <vector>
#include <string>
class GuiButton;
class StageSelectScene : public SceneBase {
public:
    StageSelectScene();
    virtual ~StageSelectScene();

    void Update() override;
    void Draw() override;

private:
    int currentCursor = 0;
    std::vector<std::string> stageFiles;
    std::vector<std::string> stageNames;

    int fontHandle;
    int bgHandle;

    //生成したボタンを保持するリスト
    std::vector<GuiButton*> buttons;
};