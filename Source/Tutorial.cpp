#include "Tutorial.h"
#include "../Library/Input.h"

TutorialScene::TutorialScene()
{
}

TutorialScene::~TutorialScene()
{
}

void TutorialScene::Update()
{
	if (Input::IsKeyDown(KEY_INPUT_ESCAPE)) {
		SceneManager::ChangeScene("TITLE");
		return;
	}
}

void TutorialScene::Draw()
{
}
