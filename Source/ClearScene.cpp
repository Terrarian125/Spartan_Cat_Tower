#include "ClearScene.h"

ClearScene::ClearScene()
{
	alpha = 0.0f;
	fadeSpeed = 0.01f;
	ChangeTimer = 0.0f;
	hSound = LoadSoundMem("Data/Sound/ClearSound.wav");
	PlaySoundMem(hSound, DX_PLAYTYPE_BACK);
}

ClearScene::~ClearScene()
{
}

void ClearScene::Update()
{
}

void ClearScene::Draw()
{
}
