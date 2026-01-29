#pragma once
#include "../Library/Object2D.h"

class Player : public Object2D {
public:
	Player();
	Player(VECTOR2 pos);
	~Player();
	void Update() override;
	void Draw() override;
private:
	float velocityY;
	bool onGround;
	bool prevPushed;

	float Gravity;
	float JumpHeight;
	float JumpV0;
	float moveSpeed;
	int animTimer;

	int hSound;

	//Input‚ÉˆÚ‚µ‚½
	//int Volume_2 = 128; // 255 ‚Ì–ñ 50%
	//int Volume_4 = 64; // 255 ‚Ì–ñ 25%
};