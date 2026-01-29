#include "Object2D.h"
#include "../Source/Stage.h"

Object2D::Object2D()
{
	hImage = -1; // ‰æ‘œ‚ðLoad‚µ‚Ä‚È‚¢
	position.x = 0;
	position.y = 0;
	imageSize.x = 1;
	imageSize.y = 1;
	anim = 0;
	animY = 0;
}

Object2D::~Object2D()
{
}

void Object2D::Update()
{
}

void Object2D::Draw()
{
	if (hImage > 0) {
		int x = position.x - imageSize.x / 2.0f;
		int y = position.y - imageSize.y / 2.0f;

		Stage* st = FindGameObject<Stage>();
		if (st != nullptr) {
			x -= st->ScrollX();
		}
		DrawRectGraph(x, y, anim*imageSize.x, animY*imageSize.y, imageSize.x, imageSize.y, hImage, TRUE);
	}
}
