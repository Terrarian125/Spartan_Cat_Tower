#pragma once
#include "../Library/SceneBase.h"

class LoadScene : public SceneBase
{
public:
	LoadScene();
	~LoadScene();
	void Update() override;
	void Draw() override;
private:
	int fontHandle;
	int bgHandle;
	float alpha;
	float fadeSpeed;
	float ChangeTimer;

	int LoadBg;
	int LoadMovie;
	int hSound;
};