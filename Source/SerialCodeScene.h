#pragma once
#include "../Library/SceneBase.h"
class SerialCodeScene : public SceneBase
{
public:
	int Bg;
	int UI_Back;
	SerialCodeScene();
	~SerialCodeScene();
	void Update();
	void Draw();
};

