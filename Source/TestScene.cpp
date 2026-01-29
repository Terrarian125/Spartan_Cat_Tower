#include "TestScene.h"
#include <DxLib.h>
#include "Screen.h"
#include <time.h>
#include <assert.h>
#include "../Library/Input.h"

TestScene::TestScene()
{
	tImage = LoadGraph("data/image/tImage.png");
	tLogo = LoadGraph("data/image/UI/tLogo.png");

	screenPattern = 0;
	alpha = 0;
	fadeSpeed = 0.6f;
	ChangeTimer = 0;
	tSound = LoadSoundMem("Data/Music/tMusic.mp3");
	assert(tSound > 0);
	ChangeVolumeSoundMem(Input::Volume_2, tSound);
	PlaySoundMem(tSound, DX_PLAYTYPE_NORMAL);
}

TestScene::~TestScene()
{
	//再生中の音楽を停止させる
	StopSoundMem(tSound);
	//メモリに確保していたサウンドデータと画像データを解放する
	DeleteSoundMem(tSound);
	DeleteGraph(tImage);
	DeleteGraph(tLogo);
}

void TestScene::Update()
{
	if (Input::IsKeyUP(KEY_INPUT_SPACE)) {
		StopSoundMem(tSound);
		SceneManager::ChangeScene("TITLE");
	}
	
	if (alpha < 255)
	{
		// フェードイン処理 (screenPattern = 0 の状態)
		alpha += fadeSpeed;
		if (alpha > 255)
		{
			alpha = 255;
			// alphaが255に達したら次の処理へ
			screenPattern = 1;
		}
	}

	else if (screenPattern == 1) // フェードイン完了後の状態
	{
		// サウンドを停止し、TITLEシーンへ遷移
		StopSoundMem(tSound);
		SceneManager::ChangeScene("TITLE");
	}

	if (CheckHitKey(KEY_INPUT_ESCAPE)) {
		SceneManager::Exit();
	}
}


void TestScene::Draw()
{
	int wPos = (Screen::WIDTH - 700) / 2.0f;
	int hPos = (Screen::HEIGHT - 700) / 2.0f;
	if (screenPattern == 0)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawGraph(0, 0, tImage, TRUE);
		DrawGraph(400, 300, tLogo, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}
