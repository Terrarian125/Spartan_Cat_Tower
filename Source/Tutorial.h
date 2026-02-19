#pragma once
#include "../Library/SceneBase.h"
#include "../Library/GuiButton.h"
#include "SettingPanel.h"

class TutorialScene : public SceneBase
{
public:
	TutorialScene();
	~TutorialScene();
	void Update() override;
	void Draw() override;
};