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
            DrawString(400, 350, "評価：SSS (神猫級)", col);
        }
        else if (dmg < 5) {
            DrawString(400, 350, "評価：A (かすり傷だね)", col);
        }
        else {
            DrawString(400, 350, "評価：C (ボロボロだぁ...)", col);
        }
    }
