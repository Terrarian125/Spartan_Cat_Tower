#pragma once
#include "../Library/GameObject.h"

class CoinSpawner : public GameObject {
public:
	CoinSpawner();
	~CoinSpawner();
	void Update() override;
private:
	int frameCounter;
};