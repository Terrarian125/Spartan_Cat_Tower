#pragma once
#include "../Library/GameObject.h"

class Score : public GameObject {
public:
	Score();
	~Score();
	void Reset();
	void AddScore(int val);
	int GetScore() const { return score; }
private:
	int score;
};