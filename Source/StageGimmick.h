#pragma once
#include "../Library/Object2D.h"
#include <string>

//前方宣言
class Ball2D;

class StageGimmick {
public:
    StageGimmick();

    //物理移動とギミック判定。Ball2Dのポインタを渡すことで、ギミック側からダメージを通知可能にする
    void UpdatePhysics(VECTOR2& pos, VECTOR2& vel, float radius, bool isPlayer, bool isDownPressed, float moveInput, int voiceHandle, Ball2D* pBall);
	void DrawFade();//ゴール演出のフェード描画
    void SetParams(float gravity, float jump) { G = gravity; JUMP = jump; }

private:
    float G = 0.5f;
    float JUMP = -12.0f;
    const float TILE_SIZE = 64.0f;

    bool isGoalStarted = false;
	float fadeAlpha = 15.0f;
	float fadeSpeed = 1.0f;//フェードの速さ
};