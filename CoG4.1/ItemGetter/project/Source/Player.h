#pragma once
#include "../Library/GameObject.h"

class Player : public GameObject {
public:
	Player();
	~Player();
	void Update() override;
	void Draw() override;

	float PositionX() const { return positionX; }
	float PositionY() const { return positionY; }
private:
	int hImage;
	float positionX, positionY;
	enum Dir {
		Front = 0,
		Left,
		Back,
		Right,
	};
	Dir direction;
	int counter;
};