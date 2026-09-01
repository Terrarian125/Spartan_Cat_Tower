#include "Ball2D.h"
#include "Stage.h"
#include "../Library/Input.h"
#include "../Library/CsvReader.h"
#include <math.h>
#include "StageGimmick.h"
#include <DxLib.h>
#include <windows.h>

int Ball2D::lastTotalDamage = 0;

//デバッグ表示用
bool Ball2D::debugVisible = false;
int Ball2D::debugLastTime = 0;
float Ball2D::debugFPS = 0.0f;
float Ball2D::debugMinFPS = 0.0f;
int Ball2D::debugFPSResetTime = 0;

//物理処理時間
float Ball2D::debugPhysicsTime = 0.0f;

Ball2D::Ball2D(unsigned int _color, bool _isPlayer)
    : color(_color), isPlayer(_isPlayer) {

    SetDrawOrder(50);

    position = VECTOR2(0, 0);
    velocity = VECTOR2(0, 0);

    hImage = -1;
    dmgImgHandle = -1;
    voiceHandle = -1;

    LoadParam("Data/Stage/BallParam.csv");
}

void Ball2D::LoadParam(std::string path) {
    CsvReader csv(path);

    //デフォルト値
    RADIUS = 25.0f;
    G = 0.5f;
    SPEED = 0.6f;
    SPRING_L = 250.0f;
    K = 0.08f;
    mass = 1.0f;
    BUMP_MAX_LIFE = 120.0f;

    float tempHeight = 250.0f;

    if (csv.GetLines() <= 0) return;

    for (int i = 0; i < csv.GetLines(); i++) {
        std::string name = csv.GetString(i, 0);
        std::string fileName = csv.GetString(i, 1);

        //文字列として処理する項目
        if (name == "PlayerImage" || name == "PartnerImage") {
            if ((isPlayer && name == "PlayerImage") ||
                (!isPlayer && name == "PartnerImage")) {

                hImage = LoadGraph(
                    ("Data/Image/" + fileName).c_str()
                );
            }

            continue;
        }

        if (name == "PartnerDamageImage") {
            if (!isPlayer) {
                dmgImgHandle = LoadGraph(
                    ("Data/Image/" + fileName).c_str()
                );
            }

            continue;
        }

        if (name == "PlayerVoice" || name == "PartnerVoice") {
            if ((isPlayer && name == "PlayerVoice") ||
                (!isPlayer && name == "PartnerVoice")) {

                voiceHandle = LoadSoundMem(
                    ("Data/Sound/" + fileName).c_str()
                );
            }

            continue;
        }

        float val = (float)csv.GetFloat(i, 1);

        if (name == "Radius")
            RADIUS = val;
        else if (name == "Gravity")
            G = val;
        else if (name == "MoveSpeed")
            SPEED = val;
        else if (name == "JumpHeight")
            tempHeight = val;
        else if (name == "BumpLife")
            BUMP_MAX_LIFE = val;
        else if (name == "SpringLength")
            SPRING_L = val;
        else if (name == "SpringK")
            K = val;
        else if (isPlayer && name == "MassPlayer")
            mass = val;
        else if (!isPlayer && name == "MassPartner")
            mass = val;
    }

    //重力からジャンプ初速を計算
    if (G > 0)
        JUMP = -sqrtf(2.0f * G * tempHeight);
    else
        JUMP = -10.0f;
}

void Ball2D::Update() {
    Stage* stage = FindGameObject<Stage>();
    if (!stage) return;

    float moveInput = 0;
    bool isDownPressed = false;

    VECTOR2 oldVelocity = velocity;

    //初回の更新時に初期位置を保存
    static bool isFirstUpdate = true;

    if (isFirstUpdate) {
        startPosition = position;
        isFirstUpdate = false;
    }

    //デバッグ表示切り替え
    if (isPlayer && Input::IsKeyDown(KEY_INPUT_F1)) {
        debugVisible = !debugVisible;

        //デバッグ表示開始時にFPS計測をリセット
        if (debugVisible) {
            debugLastTime = GetNowCount();
            debugFPSResetTime = debugLastTime;
            debugFPS = 0.0f;
            debugMinFPS = 0.0f;
            debugPhysicsTime = 0.0f;
        }
    }

    //コヨーテタイム更新
    if (isPlayer) {
        coyoteTime.Update();
    }

    //プレイヤー操作
    if (isPlayer) {
        if (Input::IsKeepKeyDown(KEY_INPUT_A))
            moveInput -= SPEED;

        if (Input::IsKeepKeyDown(KEY_INPUT_D))
            moveInput += SPEED;

        if (Input::IsKeepKeyDown(KEY_INPUT_S))
            isDownPressed = true;

        //ジャンプ可能な場合のみジャンプ
        if (Input::IsKeyDown(KEY_INPUT_SPACE) &&
            coyoteTime.CanJump()) {

            velocity.y = JUMP;

            //ジャンプしたのでコヨーテタイムを消費
            coyoteTime.ConsumeJump();
        }
    }

    //パートナーとの紐の物理計算
    if (isPlayer && partner) {
        VECTOR2 diff = position - partner->GetPosition();
        float dist = VSize(diff);

        if (dist > SPRING_L && dist > 0.1f) {
            float f = (dist - SPRING_L) * K;

            VECTOR2 norm = VECTOR2(
                diff.x / dist,
                diff.y / dist
            );

            partner->AddForce(
                VECTOR2(
                    norm.x * f / partner->mass,
                    norm.y * f / partner->mass
                )
            );

            //パートナー救済判定
            float rescueDistance =
                SPRING_L * RESCUE_DISTANCE;

            if (dist > rescueDistance) {

                //離れ始めた時刻を記録
                if (rescueStartTime < 0) {
                    rescueStartTime = GetNowCount();
                }
                else if (GetNowCount() - rescueStartTime >= RESCUE_TIME) {

                    //パートナーにダメージ
                    partner->OnDamage();

                    //プレイヤーの少し上へ戻す
                    partner->SetPosition(
                        position + VECTOR2(0, -50.0f)
                    );

                    //パートナーの速度をリセット
                    partner->SetVelocity(
                        VECTOR2(0, 0)
                    );

                    //救済タイマーをリセット
                    rescueStartTime = -1;
                }
            }
        }
        else {
            //距離が戻ったのでタイマーをリセット
            rescueStartTime = -1;
        }
    }
    else {
        rescueStartTime = -1;
    }

    //物理更新の開始時刻
    LARGE_INTEGER frequency;
    LARGE_INTEGER physicsStart;
    LARGE_INTEGER physicsEnd;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&physicsStart);

    gimmick.SetParams(G, JUMP);

    gimmick.UpdatePhysics(
        position,
        velocity,
        RADIUS,
        isPlayer,
        isDownPressed,
        moveInput,
        voiceHandle,
        this,
        coyoteTime
    );

    QueryPerformanceCounter(&physicsEnd);

    //物理処理時間をミリ秒で取得
    if (isPlayer) {
        debugPhysicsTime =
            (float)(physicsEnd.QuadPart - physicsStart.QuadPart)
            * 1000.0f
            / (float)frequency.QuadPart;
    }

    //速度制限
    float maxSpeedX = 10.0f;
    float maxSpeedY = 20.0f;

    if (velocity.x > maxSpeedX)
        velocity.x = maxSpeedX;

    if (velocity.x < -maxSpeedX)
        velocity.x = -maxSpeedX;

    if (velocity.y > maxSpeedY)
        velocity.y = maxSpeedY;

    if (velocity.y < -maxSpeedY)
        velocity.y = -maxSpeedY;

    //ダメージ判定
    if (!isPlayer) {

        //壁への激突判定
        if (fabsf(oldVelocity.x) > 15.0f &&
            fabsf(velocity.x) < 1.0f) {

            OnDamage();
        }

        //ひもの限界ダメージ
        if (partner) {
            VECTOR2 diff =
                position - partner->GetPosition();

            float dist = VSize(diff);

            if (dist > SPRING_L * 2.0f) {

                if (painTimer <= 0) {
                    OnDamage();
                }
            }
        }
    }

    //たんこぶ更新
    if (painTimer > 0)
        painTimer--;

    if (bump.active) {
        if (--bump.life <= 0)
            bump.active = false;
    }
}

void Ball2D::OnDamage() {
    if (painTimer <= 0) {

        painTimer = 180;

        if (!isPlayer) {

            damageCount++;

            //ダメージに応じた音量計算
            int volume = 255;

            if (damageCount >= 5) {

                volume =
                    255 - (damageCount - 5) * 9;

                if (volume < 30) {
                    volume = 30;
                }
            }

            if (voiceHandle != -1) {

                ChangeVolumeSoundMem(
                    volume,
                    voiceHandle
                );

                PlaySoundMem(
                    voiceHandle,
                    DX_PLAYTYPE_BACK
                );
            }
        }

        //たんこぶ出現
        bump.active = true;
        bump.life = 120;
    }
}

void Ball2D::Draw() {
    Stage* st = FindGameObject<Stage>();

    float sx = st ? st->ScrollX() : 0;
    float sy = st ? st->ScrollY() : 0;

    int dx = (int)(position.x - sx);
    int dy = (int)(position.y - sy);

    //FPS計測
    if (isPlayer) {

        int currentTime = GetNowCount();

        if (debugLastTime != 0) {

            int deltaTime =
                currentTime - debugLastTime;

            if (deltaTime > 0) {

                debugFPS =
                    1000.0f / (float)deltaTime;

                //最低FPSを記録
                if (debugMinFPS <= 0.0f ||
                    debugFPS < debugMinFPS) {

                    debugMinFPS = debugFPS;
                }
            }
        }

        debugLastTime = currentTime;

        //1秒ごとに最低FPSをリセット
        if (debugFPSResetTime != 0 &&
            currentTime - debugFPSResetTime >= 1000) {

            debugFPSResetTime = currentTime;
            debugMinFPS = debugFPS;
        }
    }

    //プレイヤーからパートナーへの紐
    if (isPlayer && partner) {

        VECTOR2 pPos =
            partner->GetPosition();

        DrawLine(
            dx,
            dy,
            (int)(pPos.x - sx),
            (int)(pPos.y - sy),
            GetColor(255, 255, 255),
            2
        );
    }

    //ダメージに応じた黒ずみ
    int brightness = 255;

    if (!isPlayer && damageCount >= 5) {

        brightness =
            255 - (damageCount - 5) * 9;

        if (brightness < 30) {
            brightness = 30;
        }
    }

    SetDrawBright(
        brightness,
        brightness,
        brightness
    );

    //ダメージ中なら専用画像
    int currentImg =
        (painTimer > 0 && dmgImgHandle != -1)
        ? dmgImgHandle
        : hImage;

    if (currentImg != -1) {

        DrawExtendGraph(
            dx - (int)RADIUS,
            dy - (int)RADIUS,
            dx + (int)RADIUS,
            dy + (int)RADIUS,
            currentImg,
            TRUE
        );
    }
    else {

        DrawCircle(
            dx,
            dy,
            (int)RADIUS,
            color,
            TRUE
        );
    }

    //輝度をリセット
    SetDrawBright(
        255,
        255,
        255
    );

    //デバッグ表示
    if (isPlayer && debugVisible) {

        int textColor =
            GetColor(255, 255, 255);

        //背景
        DrawBox(
            5,
            5,
            350,
            180,
            GetColor(0, 0, 0),
            TRUE
        );

        DrawFormatString(
            15,
            15,
            textColor,
            "[DEBUG]"
        );

        DrawFormatString(
            15,
            35,
            textColor,
            "FPS: %.1f",
            debugFPS
        );

        DrawFormatString(
            15,
            55,
            textColor,
            "MIN FPS: %.1f",
            debugMinFPS
        );

        DrawFormatString(
            15,
            75,
            textColor,
            "Player X: %.1f  Y: %.1f",
            position.x,
            position.y
        );

        if (partner) {

            VECTOR2 pPos =
                partner->GetPosition();

            DrawFormatString(
                15,
                95,
                textColor,
                "Partner X: %.1f  Y: %.1f",
                pPos.x,
                pPos.y
            );

            DrawFormatString(
                15,
                115,
                textColor,
                "Damage: %d",
                partner->GetDamageCount()
            );
        }
        else {

            DrawFormatString(
                15,
                95,
                textColor,
                "Partner: NONE"
            );

            DrawFormatString(
                15,
                115,
                textColor,
                "Damage: 0"
            );
        }

        DrawFormatString(
            15,
            135,
            textColor,
            "State: %s",
            velocity.y < 0.0f
            ? "JUMP"
            : "FALL/GROUND"
        );

        DrawFormatString(
            15,
            155,
            textColor,
            "Physics: %.3f ms",
            debugPhysicsTime
        );
    }

    if (isPlayer) {
        gimmick.DrawFade();
    }
}