#include "App.h"
#include "SceneManager.h"
#include "ObjectManager.h"
#include <DxLib.h>
//#include "../Source/StageEditor.h"
#include "../Source/Stage.h"
#include <memory> 

//グローバル変数をスマートポインタに変更
//これにより、delete を手動で書かなくても安全に破棄されます
//std::unique_ptr<StageEditor> editor = nullptr;

void AppInit()
{
	ObjectManager::Init();
	SceneManager::Init();
	//editor.reset(); //初期化
}

void AppUpdate()
{
	SceneManager::Update();
	ObjectManager::Update();

	Stage* currentStage = nullptr;
	const auto& allObjects = ObjectManager::GetAllObject();

	for (GameObject* obj : allObjects) {
		if (!obj) continue;

		//Stageを継承しているか、直接の型かをチェック
		Stage* s = dynamic_cast<Stage*>(obj);
		if (s) {
			currentStage = s;
			break;
		}
	}

	//if (currentStage) {
	//	if (!editor) {
	//		editor = std::make_unique<StageEditor>(currentStage);
	//	}
	//	editor->Update(); //Stageが確実にいる時だけUpdate
	//}
	//else {
	//	editor.reset(); //いなければ即座に消す
	//}
	////スマートポインタが有効な場合のみ実行
	//if (editor) {
	//	editor->Update();
	//}
}

void AppDraw()
{
	SceneManager::Draw();
	ObjectManager::Draw();

	////描画時も安全チェック
	//if (editor) {
	//	editor->Draw();
	//}
}

void AppRelease()
{
	//editor.reset(); //明示的な破棄
	ObjectManager::Release();
	SceneManager::Release();
}

bool AppIsExit()
{
	return SceneManager::IsExit();
}