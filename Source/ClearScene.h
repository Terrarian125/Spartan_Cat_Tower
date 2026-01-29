#pragma once
#include "../Library/SceneBase.h"

/// <summary>
/// クリアシーン
/// </summary>

class ClearScene : public SceneBase
{
public:
	ClearScene();
	~ClearScene();
	void Update() override;
	void Draw() override;
private:

};