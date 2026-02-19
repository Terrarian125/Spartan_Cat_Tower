#include "ClearScene.h"
#include "../Library/Input.h"
#include "Ball2D.h"

ClearScene::ClearScene()
{
	alpha = 0.0f;
	fadeSpeed = 0.01f;
	ChangeTimer = 0.0f;
	hSound = LoadSoundMem("Data/Sound/ClearSound.wav");
	PlaySoundMem(hSound, DX_PLAYTYPE_BACK);

    //メインメニューのボタン配置
    int bx = 50, //ボタンのX座標
        by = 50, //ボタンのY座標
        bw = 100,//ボタンの幅
        bh = 100;  //ボタンの高さ

    int btnImg_StBack = LoadGraph("data/image/btnImg_StBack.png");

    auto bStBack = new GuiButton(bx, by, bw, bh, "ステージ選択へ");
    bStBack->SetImage(btnImg_StBack);//ボタン画像を設定、なければデフォルトの四角形ボタンになる
    bStBack->onClick = []() { SceneManager::ChangeScene("STAGE"); };
    buttons.push_back(bStBack);
}

ClearScene::~ClearScene()
{
}

void ClearScene::Update()
{
	if (Input::IsKeyDown(KEY_INPUT_ESCAPE)) {
		SceneManager::ChangeScene("STAGE");
		return;
	}
}

void ClearScene::Draw()
{
        int dmg = Ball2D::lastTotalDamage;
        int col = GetColor(255, 255, 255);

        DrawFormatString(400, 300, col, "受けたダメージ: %d 回", dmg);

        if (dmg == 0) {
            DrawString(400, 350, "評価：SSS", col);
        }
        else if (dmg < 5) {
            DrawString(400, 350, "評価：A", col);
        }
        else {
            DrawString(400, 350, "評価：C", col);
        }
    }
