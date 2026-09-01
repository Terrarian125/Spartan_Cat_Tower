#include "StageGimmick.h"

#include "Stage.h"
#include "Ball2D.h"
#include "CoyoteTime.h"

#include "../Library/SceneManager.h"

#include <math.h>
#include <string>
#include <DxLib.h>

float StageGimmick::physicsTime = 0.0f;
float StageGimmick::horizontalTime = 0.0f;
float StageGimmick::verticalTime = 0.0f;
float StageGimmick::spikeTime = 0.0f;

StageGimmick::StageGimmick() {
    isGoalStarted = false;
    fadeAlpha = 0.0f;
}

void StageGimmick::UpdatePhysics(VECTOR2& pos, VECTOR2& vel, float radius, bool isPlayer, bool isDownPressed, float moveInput, int voiceHandle, Ball2D* pBall, CoyoteTime& coyoteTime) {
    int physicsStart = GetNowCount();

    Stage* stage = FindGameObject<Stage>();
    if (!stage) return;

    //ゴール演出中の処理
    if (isGoalStarted) {
        fadeAlpha += fadeSpeed;

        if (fadeAlpha >= 255.0f) {
            fadeAlpha = 255.0f;
            SceneManager::ChangeScene("CLEAR");
        }
    }

    //縦方向の速度移動を適用
    pos.y += vel.y;

    //入力、重力、氷の摩擦計算
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

    if (!isPlayer) {
        vel.x *= 0.95f;
    }

    //横方向移動
    int horizontalStart = GetNowCount();

    float oldX = pos.x;
    pos.x += vel.x;

    float frontX = pos.x + (vel.x > 0 ? radius : -radius);
    float frontY = pos.y + radius - 5.0f;

    std::string frontAttr = stage->GetTileFunction(frontX, frontY);

    //横方向のBARRIER判定
    if (frontAttr == "BARRIER") {
        pos.x = oldX;

        //右側の壁に接触した
        if (vel.x > 0) {
            vel.x = -15.0f;
        }
        //左側の壁に接触した
        else if (vel.x < 0) {
            vel.x = 15.0f;
        }

        //壁では接地扱いにしない
        if (isPlayer) {
            coyoteTime.SetGrounded(false);
        }
    }
    //横方向のSOLIDとSPRING判定
    else if (frontAttr == "SOLID" || frontAttr == "SPRING") {
        pos.x = oldX;
        vel.x = 0;
    }

    horizontalTime =
        (float)(GetNowCount() - horizontalStart);

    //縦方向移動
    int verticalStart = GetNowCount();

    pos.y += vel.y;

    //足元確認
    float footX = pos.x;
    float footY = pos.y + radius;

    std::string attr =
        stage->GetTileFunction(
            footX,
            footY
        );

    std::string centerAttr =
        stage->GetTileFunction(
            pos.x,
            pos.y
        );

    //頭上確認
    float headX = pos.x;
    float headY = pos.y - radius;

    //ゴール判定
    if (centerAttr == "GOAL" && !isGoalStarted) {
        if (pBall && isPlayer) {

            Ball2D* partner =
                pBall->GetPartner();

            if (partner) {
                Ball2D::lastTotalDamage =
                    partner->GetDamageCount();
            }

            isGoalStarted = true;

            physicsTime =
                (float)(GetNowCount() - physicsStart);

            return;
        }
    }

    verticalTime =
        (float)(GetNowCount() - verticalStart);

    //トゲ処理
    int spikeStart = GetNowCount();

    if (stage->GetTileFunction(pos.x, pos.y + radius) == "SPIKE" ||
        stage->GetTileFunction(pos.x, pos.y - radius) == "SPIKE" ||
        stage->GetTileFunction(pos.x + radius, pos.y) == "SPIKE" ||
        stage->GetTileFunction(pos.x - radius, pos.y) == "SPIKE") {

        if (pBall) {

            pBall->OnDamage();

            vel.y = -10.0f;

            if (stage->GetTileFunction(pos.x + radius, pos.y) == "SPIKE") {
                vel.x = -15.0f;
            }

            if (stage->GetTileFunction(pos.x - radius, pos.y) == "SPIKE") {
                vel.x = 15.0f;
            }

            pos.y -= 5.0f;

            spikeTime =
                (float)(GetNowCount() - spikeStart);

            physicsTime =
                (float)(GetNowCount() - physicsStart);

            return;
        }
    }

    spikeTime =
        (float)(GetNowCount() - spikeStart);

    //斜面処理
    if (attr == "SLOPE_R" || attr == "SLOPE_L") {

        float localX =
            fmod(footX, TILE_SIZE);

        if (localX < 0) {
            localX += TILE_SIZE;
        }

        float tx =
            localX / TILE_SIZE;

        float ty =
            (attr == "SLOPE_R")
            ? (1.0f - tx)
            : tx;

        float tileBaseY =
            floor(footY / TILE_SIZE) *
            TILE_SIZE;

        float targetY =
            tileBaseY +
            (ty * TILE_SIZE) -
            radius;

        if (vel.y >= 0 ||
            pos.y > targetY - 15.0f) {

            pos.y = targetY;
            vel.y = 0;

            //接地したのでコヨーテタイムを開始
            if (isPlayer) {
                coyoteTime.SetGrounded(true);
            }
        }
    }
    //通常ブロック処理
    else if (attr == "SOLID" ||
        attr == "SPRING" ||
        attr == "ICE" ||
        attr == "ONE_WAY" ||
        attr == "SPIKE" ||
        attr == "BARRIER") {

        bool isLanding = false;

        float tileTopY =
            floor(footY / TILE_SIZE) *
            TILE_SIZE;

        //BARRIERは上から乗らず、ノックバックする
        if (attr == "BARRIER") {

            if (vel.y > 0) {

                pos.y =
                    tileTopY - radius;

                vel.y = -15.0f;
            }
            else if (vel.y < 0) {

                pos.y =
                    tileTopY +
                    TILE_SIZE +
                    radius;

                vel.y = 15.0f;
            }

            if (isPlayer) {
                coyoteTime.SetGrounded(false);
            }
        }
        //ONE_WAYは上からのみ着地
        else if (attr == "ONE_WAY") {

            if (!isDownPressed &&
                vel.y > 0 &&
                footY >= tileTopY &&
                footY <= tileTopY + vel.y + 10.0f) {

                isLanding = true;
            }
        }
        //通常ブロック
        else if (vel.y > 0) {
            isLanding = true;
        }

        if (isLanding) {

            pos.y =
                tileTopY - radius;

            if (attr == "SPRING") {

                vel.y =
                    JUMP * 1.5f;

                if (voiceHandle != -1) {
                    PlaySoundMem(
                        voiceHandle,
                        DX_PLAYTYPE_BACK
                    );
                }
            }
            else {
                vel.y = 0;
            }

            //SPRING以外は接地
            if (isPlayer &&
                attr != "SPRING") {

                coyoteTime.SetGrounded(true);
            }
        }

        //上昇中にブロック下面へ衝突
        if (vel.y < 0 &&
            (attr == "SOLID" ||
                attr == "ICE" ||
                attr == "SPRING" ||
                attr == "SPIKE")) {

            float tileBottomY =
                floor(headY / TILE_SIZE) *
                TILE_SIZE +
                TILE_SIZE;

            if (headY <= tileBottomY &&
                stage->GetTileFunction(
                    headX,
                    tileBottomY
                ) == attr) {

                pos.y =
                    tileBottomY +
                    radius;

                vel.y = 0;

                //下面への衝突では接地扱いにしない
                if (isPlayer) {
                    coyoteTime.SetGrounded(false);
                }
            }
        }
    }

    physicsTime =
        (float)(GetNowCount() - physicsStart);
}

float StageGimmick::GetPhysicsTime() {
    return physicsTime;
}

float StageGimmick::GetHorizontalTime() {
    return horizontalTime;
}

float StageGimmick::GetVerticalTime() {
    return verticalTime;
}

float StageGimmick::GetSpikeTime() {
    return spikeTime;
}

void StageGimmick::DrawFade() {

    if (fadeAlpha > 0.0f) {

        SetDrawBlendMode(
            DX_BLENDMODE_ALPHA,
            (int)fadeAlpha
        );

        //画面全体を黒く塗る
        DrawFillBox(
            0,
            0,
            1280,
            720,
            GetColor(0, 0, 0)
        );

        SetDrawBlendMode(
            DX_BLENDMODE_NOBLEND,
            0
        );
    }
}