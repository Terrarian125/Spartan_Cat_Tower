#pragma once
#include "../Library/GameObject.h"

class Coin : public GameObject {
public:
	Coin();
	Coin(int x, int y);
	~Coin();
	void Update();
	void Draw();
private:
	int hImage;
	float positionX, positionY;
};