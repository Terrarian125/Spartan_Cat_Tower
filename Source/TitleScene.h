#pragma once
#include "../Library/SceneBase.h"
#include "../Library/GuiButton.h"
#include "SettingPanel.h"

///<summary>
///タイトルシーン
///
///タイトルを表示して、キーを押したらプレイシーンに移行する。
///</summary>
class TitleScene : public SceneBase
{
public:
	TitleScene();
	~TitleScene();
	void Update() override;
	void Draw() override;
	//int hImage;
	int LogoBg;
	int Logo;
	int Op_Music;

	///フェード用
	float alpha;
	float fadeSpeed;
	float ChangeTimer;
	int hSound;

	int selectIdx = 0;
	std::vector<GuiButton*> buttons;

	////inputにうつした
	//int Volume_2 = 128; //255 の約 50%
	//int Volume_4 = 64; //255 の約 25%

	bool screenPattern;//画面パターン選択用

	bool isExitDialogVisible = false;
	std::vector<GuiButton*> exitButtons;

private:
	std::vector<GuiButton*> menuButtons;
	int currentSelect = 0; //キーボード選択用インデックス
	SettingPanel* mySettingPanel;
};