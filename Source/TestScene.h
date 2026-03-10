#pragma once
#include "../Library/SceneBase.h"

///<summary>
///TestƒV[ƒ“
///
//
///</summary>
class TestScene : public SceneBase
{
public:
	TestScene();
	~TestScene();
	void Update() override;
	void Draw() override;
private:
	int tImage;
	int tLogo;
	float alpha;
	float fadeSpeed;
	float ChangeTimer;
	int tSound;

	////input‚É‚¤‚Â‚µ‚½
	//int Volume_2 = 128; //255 ‚Ì–ñ 50%
	//int Volume_4 = 64; //255 ‚Ì–ñ 25%

	bool screenPattern;
};
