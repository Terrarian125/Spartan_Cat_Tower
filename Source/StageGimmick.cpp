#include "StageGimmick.h"
#include "Stage.h"
#include "Ball2D.h"
#include "../Library/SceneManager.h"
#include <math.h>

StageGimmick::StageGimmick() {}

void StageGimmick::UpdatePhysics(VECTOR2& pos, VECTOR2& vel, float radius, bool isPlayer, bool isDownPressed, float moveInput, int voiceHandle, Ball2D* pBall) {
    Stage* stage = FindGameObject<Stage>();
    if (!stage) return;

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

    // 水平移動と段差乗り越え判定
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

    // 足元の属性確認
    float footX = pos.x;
    float footY = pos.y + radius;
    std::string attr = stage->GetTileFunction(footX, footY);

    if (attr == "GOAL") {
        SceneManager::ChangeScene("CLEAR");
        return;
    }

    // ギミック：トゲ床（NEEDLE）の処理
    // 足元の中心だけでなく、少し左右も含めて接触判定を行う
    if (stage->GetTileFunction(pos.x, footY) == "NEEDLE" ||
        stage->GetTileFunction(pos.x - radius * 0.5f, footY) == "NEEDLE" ||
        stage->GetTileFunction(pos.x + radius * 0.5f, footY) == "NEEDLE") {

        if (pBall) {
            pBall->OnDamage();
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
    else if (attr == "SOLID" || attr == "SPRING" || attr == "ICE" || attr == "ONE_WAY" || attr == "NEEDLE") {
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