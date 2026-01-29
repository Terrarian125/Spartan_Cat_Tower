#include "Score.h"

Score::Score()
{
	StayOnSceneChange();
	score = 0;
}

Score::~Score()
{
}

void Score::Reset()
{
	score = 0;
}

void Score::AddScore(int val)
{
	score += val;
}
