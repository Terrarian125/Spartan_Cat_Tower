#include "PlayScene.h"
#include <DxLib.h>
#include "Player.h"
#include "CoinSpawner.h"
#include "Score.h"

PlayScene::PlayScene()
{
	new Player();
	new CoinSpawner();
	Score* sc = FindGameObject<Score>();
	sc->Reset();
}

PlayScene::~PlayScene()
{
}

void PlayScene::Update()
{
	if (CheckHitKey(KEY_INPUT_T)) {
		SceneManager::ChangeScene("TITLE");
	}
}

void PlayScene::Draw()
{
	DrawString(0, 0, "PLAY SCENE", GetColor(255, 255, 255));
	DrawString(100, 400, "Push [T]Key To Title", GetColor(255, 255, 255));

	Score* sc = FindGameObject<Score>();
	DrawFormatString(600, 30, GetColor(255, 255, 255), "%d", sc->GetScore());
}
