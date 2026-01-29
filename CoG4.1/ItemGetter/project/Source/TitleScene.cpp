#include "TitleScene.h"
#include <DxLib.h>
#include "Score.h"

TitleScene::TitleScene()
{
	if (FindGameObject<Score>() == nullptr)
		new Score();
}

TitleScene::~TitleScene()
{
}

void TitleScene::Update()
{
	if (CheckHitKey(KEY_INPUT_P)) {
		SceneManager::ChangeScene("PLAY");
	}
	if (CheckHitKey(KEY_INPUT_ESCAPE)) {
		SceneManager::Exit();
	}
}


void TitleScene::Draw()
{
	DrawString(0, 0, "TITLE SCENE", GetColor(255,255,255));
	DrawString(100, 400, "Push [P]Key To Play", GetColor(255, 255, 255));

	Score* sc = FindGameObject<Score>();
	DrawFormatString(600, 30, GetColor(255, 255, 255), "%d", sc->GetScore());
}
