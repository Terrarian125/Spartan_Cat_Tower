#include "BootScene.h"
#include "../Library/SceneManager.h"

BootScene::BootScene()
{
}

BootScene::~BootScene()
{
}

void BootScene::Update()
{
	SceneManager::ChangeScene("TEST");
	//SceneManager::ChangeScene("PLAY");
}

void BootScene::Draw()
{
}
