#include "Player.h"

static const float Speed = 1.5f;

Player::Player()
{
	hImage = LoadGraph("data/chara.png");
	positionX = 100;
	positionY = 100;
	direction = Dir::Front;
	counter = 0;
	SetDrawOrder(-1);
}

Player::~Player()
{
	if (hImage > 0) {
		DeleteGraph(hImage);
		hImage = -1;
	}
}

void Player::Update()
{
	if (CheckHitKey(KEY_INPUT_W)) {
		positionY -= Speed;
		direction = Dir::Back;
	}
	if (CheckHitKey(KEY_INPUT_S)) {
		positionY += Speed;
		direction = Dir::Front;
	}
	if (CheckHitKey(KEY_INPUT_D)) {
		positionX += Speed;
		direction = Dir::Right;
	}
	if (CheckHitKey(KEY_INPUT_A)) {
		positionX -= Speed;
		direction = Dir::Left;
	}
	counter++;
}

void Player::Draw()
{
	int col = (counter / 16) % 4 + 4;
	int row = (int)direction;
	DrawRectGraph((int)positionX, (int)positionY, col*64, row*64, 64, 64, hImage, TRUE);
}
