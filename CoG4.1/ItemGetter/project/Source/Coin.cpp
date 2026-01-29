#include "Coin.h"
#include "Player.h"
#include "Score.h"

Coin::Coin() : Coin(0,0)
{
}

Coin::Coin(int x, int y)
{
	hImage = LoadGraph("data/item.png");

	positionX = (float)x;
	positionY = (float)y;
}

Coin::~Coin()
{
}

void Coin::Update()
{
	Player* player = FindGameObject<Player>();
	if (player != nullptr) {
		float dx = player->PositionX() - positionX;
		float dy = player->PositionY() - positionY;
		if (sqrtf(dx * dx + dy * dy) < 32.0f) {
			Score* sc = FindGameObject<Score>();
			sc->AddScore(100);
			DestroyMe();
		}
	}
}

void Coin::Draw()
{
	DrawRectGraph((int)positionX, (int)positionY, 64, 0, 64, 64, hImage, TRUE);
}
