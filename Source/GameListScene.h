#pragma once
#include "../Library/SceneBase.h"
#include "../GameInfo.h" //定義した構造体をインクルード
#include <vector>       //リスト管理に必要

class GameListScene : public SceneBase
{
public:
	int Bg;
	int UI_Back;
	int UI_Wait;
	int UI_Play;
	GameListScene();
	~GameListScene();
	void Update() override;
	void Draw() override;
private:
	std::vector<GameInfo> gameList; //ゲームリスト本体
	int selectedIndex = 0;          //現在選択されているリストのインデックス
	int currentSortMode = 0;        // ソートモード (0:名前順, 1:登録日順, ...)

	bool isWaitingForGameLaunch;// ゲーム起動待機中フラグ（UI＿Waitにつかい4）
};