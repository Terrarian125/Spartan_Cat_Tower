#include "SerialCodeScene.h"
#include <DxLib.h>
#include "../Library/Input.h"

SerialCodeScene::SerialCodeScene()
{
	Bg = LoadGraph("data/image/Bg.png");
	UI_Back = LoadGraph("data/image/UI/UI_Q_back.png");
}

SerialCodeScene::~SerialCodeScene()
{
}

void SerialCodeScene::Update()
{
	if (Input::IsKeyUP(KEY_INPUT_Q)) {
		SceneManager::ChangeScene("BUTTON");
	}
}

void SerialCodeScene::Draw()
{
	DrawGraph(0, 0, Bg, TRUE);
	DrawGraph(800, 550, UI_Back, TRUE);
}
