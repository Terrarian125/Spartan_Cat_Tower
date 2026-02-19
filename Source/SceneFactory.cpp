#include "SceneFactory.h"
#include <windows.h>
#include <assert.h>
#include "BootScene.h"
#include "TitleScene.h"
#include "TestScene.h"
#include "PlayScene.h"
#include "SettingScene.h"
#include "ClearScene.h"
#include "StageSelectScene.h"
#include "Tutorial.h"
#include "LoadScene.h"


SceneBase* SceneFactory::CreateFirst(){
	return new BootScene();
}

SceneBase * SceneFactory::Create(const std::string & name){
	if (name == "TEST"){
		return new TestScene();
	}
	if (name == "TITLE"){
		return new TitleScene();
	}
	if (name == "PLAY"){
		return new PlayScene();
	}
	if (name == "CLEAR") {
		return new ClearScene();
	}
	if (name == "SETTING") {
		return new SettingScene();
	}
	if (name == "STAGE") {
		return new StageSelectScene();
	}
	if (name == "TUTORIAL") {
		return new TutorialScene();
	}
	if (name == "LOAD") {
		return new LoadScene();
	}


	//if (name == ""){
	//	return new ();
	//}
	//if (name == ""){
	//	return new ();
	//}
	//if (name == ""){
	//	return new ();
	//}
	MessageBox(NULL, ("éüÇÃÉVÅ[ÉìÇÕÇ†ÇËÇ‹ÇπÇÒ\n" + name).c_str(), "SceneFactory", MB_ICONERROR | MB_OK);
	assert(false);
	return nullptr;
}
