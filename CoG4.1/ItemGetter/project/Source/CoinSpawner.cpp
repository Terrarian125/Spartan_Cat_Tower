#include "CoinSpawner.h"
#include "Screen.h"
#include "Coin.h"

static const int SPAWN_TIME = 5 * 60;
CoinSpawner::CoinSpawner()
{
	frameCounter = SPAWN_TIME;
}

CoinSpawner::~CoinSpawner()
{
}

void CoinSpawner::Update()
{
	frameCounter--;
	if (frameCounter <= 0) {
		frameCounter = SPAWN_TIME;
		int x = rand() * (Screen::WIDTH - 64) / RAND_MAX;
		int y = rand() * (Screen::HEIGHT - 64) / RAND_MAX;
		new Coin(x, y);
	}
}
