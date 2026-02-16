#pragma once
#include "../Library/SceneBase.h"
#include "../Library/GuiButton.h"
#include "SettingPanel.h"

/// <summary>
/// クリアシーン
/// </summary>

class ClearScene : public SceneBase
{
public:
	ClearScene();
	~ClearScene();
	void Update() override;
	void Draw() override;

	///フェード用
	float alpha;
	float fadeSpeed;
	float ChangeTimer;
	int hSound;

	int selectIdx = 0;
	std::vector<GuiButton*> buttons;
private:

};