#pragma once
#include "../Library/SceneBase.h"
#include <vector> 

class GuiButton;

class TutorialScene : public SceneBase
{
public:
	TutorialScene();
	~TutorialScene();
	void Update() override;
	void Draw() override;

private:
	std::vector<int> imageHandles;
	int currentPage;

	GuiButton* btnPrev;
	GuiButton* btnNext;
	GuiButton* btnBack;
};