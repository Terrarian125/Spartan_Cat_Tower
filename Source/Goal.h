#pragma once
#include "../Library/Object2D.h"

class Goal : public Object2D
{
public:
	//Goal();
	Goal(VECTOR2 pos);
	~Goal();
	void Update() override;
	int GetScore();
	int GoalSound;
private:
	int score;
};