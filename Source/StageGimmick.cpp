#include "StageGimmick.h"
#include "Stage.h"
#include "Ball2D.h"
#include "../Library/SceneManager.h"
#include <math.h>
#include <string>
#include <DxLib.h> // SetDrawBlendMode 等のために必要

StageGimmick::StageGimmick() {
    isGoalStarted = false;
    fadeAlpha = 0.0f;
}

void StageGimmick::UpdatePhysics(VECTOR2& pos, VECTOR2& vel, float radius, bool isPlayer, bool isDownPressed, float moveInput, int voiceHandle, Ball2D* pBall) {
    Stage* stage = FindGameObject<Stage>();
    if (!stage) return;

    // --- ゴール演出中の処理 ---
    if (isGoalStarted) {
        fadeAlpha += fadeSpeed;
        if (fadeAlpha >= 255.0f) {
            fadeAlpha = 255.0f;
            SceneManager::ChangeScene("CLEAR");
        }
        // フェード中は物理演算を止める、または移動を制限したい場合はここで return しても良い
    }

    // 入力、重力、氷の摩擦計算
    if (isPlayer) {
        if (stage->GetTileFunction(pos.x, pos.y + radius + 1) == "ICE") {
            vel.x += moveInput * 0.2f;
            vel.x *= 0.98f;
        }
        else {
            vel.x += moveInput;
            vel.x *= 0.90f;
        }
    }

    vel.y += G;
    if (!isPlayer) vel.x *= 0.95f;

    // 水平移動
    float oldX = pos.x;
    pos.x += vel.x;

    std::string frontAttr = stage->GetTileFunction(pos.x + (vel.x > 0 ? radius : -radius), pos.y + radius - 5.0f);
    if (frontAttr == "SOLID") {
        if (stage->GetTileFunction(pos.x, pos.y - radius) == "NONE") {
            pos.y -= 8.0f;
        }
        else {
            pos.x = oldX;
            vel.x = 0;
        }
    }

    // 垂直移動
    pos.y += vel.y;

    // 属性確認
    float footX = pos.x;
    float footY = pos.y + radius;
    std::string attr = stage->GetTileFunction(footX, footY);
    std::string centerAttr = stage->GetTileFunction(pos.x, pos.y);

    // --- ゴール判定の修正 ---
    if (centerAttr == "GOAL" && !isGoalStarted) {
        if (pBall && isPlayer) {
            Ball2D* partner = pBall->GetPartner();
            if (partner) {
                Ball2D::lastTotalDamage = partner->GetDamageCount();
            }
            isGoalStarted = true; // 即座にChangeSceneせず、フラグを立てる
            return;
        }
    }

    // トゲ床（SPIKE）の処理
    if (stage->GetTileFunction(pos.x, pos.y + radius) == "SPIKE" ||
        stage->GetTileFunction(pos.x, pos.y - radius) == "SPIKE" ||
        stage->GetTileFunction(pos.x + radius, pos.y) == "SPIKE" ||
        stage->GetTileFunction(pos.x - radius, pos.y) == "SPIKE")
    {
        if (pBall) {
            pBall->OnDamage();
            vel.y = -10.0f;
            if (stage->GetTileFunction(pos.x + radius, pos.y) == "SPIKE") vel.x = -15.0f;
            if (stage->GetTileFunction(pos.x - radius, pos.y) == "SPIKE") vel.x = 15.0f;
            pos.y -= 5.0f;
            return;
        }
    }

    // 坂道吸着の処理
    if (attr == "SLOPE_R" || attr == "SLOPE_L") {
        float localX = fmod(footX, TILE_SIZE);
        if (localX < 0) localX += TILE_SIZE;
        float tx = localX / TILE_SIZE;
        float ty = (attr == "SLOPE_R") ? (1.0f - tx) : tx;
        float tileBaseY = floor(footY / TILE_SIZE) * TILE_SIZE;
        float targetY = tileBaseY + (ty * TILE_SIZE) - radius;

        if (vel.y >= 0 || pos.y > targetY - 15.0f) {
            pos.y = targetY;
            vel.y = 0;
        }
    }
    // 着地判定
    else if (attr == "SOLID" || attr == "SPRING" || attr == "ICE" || attr == "ONE_WAY" || attr == "SPIKE") {
        bool isLanding = false;
        float tileTopY = floor(footY / TILE_SIZE) * TILE_SIZE;

        if (attr == "ONE_WAY") {
            if (!isDownPressed && vel.y > 0 && footY >= tileTopY && footY <= tileTopY + vel.y + 10.0f) {
                isLanding = true;
            }
        }
        else if (vel.y > 0) {
            isLanding = true;
        }

        if (isLanding) {
            pos.y = tileTopY - radius;
            if (attr == "SPRING") {
                vel.y = JUMP * 1.5f;
                if (voiceHandle != -1) PlaySoundMem(voiceHandle, DX_PLAYTYPE_BACK);
            }
            else {
                vel.y = 0;
            }
        }
    }
}

// フェード描画の実装
void StageGimmick::DrawFade() {
    if (fadeAlpha > 0.0f) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)fadeAlpha);
        // 画面全体を黒く塗る（サイズは環境に合わせて1280, 720等に変更してください）
        DrawFillBox(0, 0, 1280, 720, GetColor(0, 0, 0));
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}