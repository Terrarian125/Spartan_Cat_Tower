#include "../ImGui/imgui.h"
#include "Goal.h"
#include "Stage.h"
#include "Player.h"
#include <assert.h>
#include "../Library/Input.h"
//Goal::Goal()
//{
//}

Goal::Goal(VECTOR2 pos)
{
	hImage = LoadGraph("data/image/parts.png");
	assert(hImage > 0);
	imageSize = VECTOR2(64, 64);
	position = pos;

	GoalSound = LoadSoundMem("Data/Music/Goal.mp3");
	ChangeVolumeSoundMem(Input::Volume_2, GoalSound);

	anim = 3;
	animY = 0;
	score = 0;
}

Goal::~Goal()
{

}

void Goal::Update()
{
	Player* pl = FindGameObject<Player>();
	VECTOR2 pPos = pl->GetPosition();

	if (VSize(pPos - position) <= 60.0f)
	{
		//PlaySoundMem(GoalSound, DX_PLAYTYPE_BACK);
		score += 100;
		DestroyMe();
		//ResultScene‚ÉˆÚ“®
		SceneManager::ChangeScene("TEST");

	}
}

int Goal::GetScore()
{
	return score;
}
