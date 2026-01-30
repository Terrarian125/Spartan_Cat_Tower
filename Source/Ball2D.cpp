#include "Ball2D.h"
#include "Stage.h"
#include "../Library/Input.h"
#include "../Library/CsvReader.h"
#include <math.h>

Ball2D::Ball2D(unsigned int _color, bool _isPlayer) : color(_color), isPlayer(_isPlayer) {
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

    // デフォルト値
    RADIUS = 25.0f; G = 0.5f; SPEED = 0.6f;
    SPRING_L = 250.0f; K = 0.08f; mass = 1.0f;
    BUMP_MAX_LIFE = 120.0f;
    float tempHeight = 250.0f; // CSVのJumpHeightの初期値

    if (csv.GetLines() <= 0) return;

    for (int i = 0; i < csv.GetLines(); i++) {
        std::string name = csv.GetString(i, 0);
        std::string fileName = csv.GetString(i, 1);

        //文字列として処理する項目（数値変換をスキップ） 
        if (name == "PlayerImage" || name == "PartnerImage") {
            if ((isPlayer && name == "PlayerImage") || (!isPlayer && name == "PartnerImage")) {
                hImage = LoadGraph(("Data/Image/" + fileName).c_str());
            }
            continue; // 数値変換させない
        }
        if (name == "PartnerDamageImage") {
            if (!isPlayer) {
                dmgImgHandle = LoadGraph(("Data/Image/" + fileName).c_str());
            }
            continue;
        }
        if (name == "PlayerVoice" || name == "PartnerVoice") {
            if ((isPlayer && name == "PlayerVoice") || (!isPlayer && name == "PartnerVoice")) {
                voiceHandle = LoadSoundMem(("Data/Sound/" + fileName).c_str());
            }
            continue;
        }

        //ここから数値変換が必要な項目 
        // GetFloatを呼ぶ前に、値が空でないか、数値っぽいかを確認するのが安全
        float val = (float)csv.GetFloat(i, 1);

        if (name == "Radius")           RADIUS = val;
        else if (name == "Gravity")     G = val;
        else if (name == "MoveSpeed")   SPEED = val;
        else if (name == "JumpHeight")  tempHeight = val;
        else if (name == "BumpLife")    BUMP_MAX_LIFE = val;
        else if (name == "SpringLength") SPRING_L = val;
        else if (name == "SpringK")      K = val;
        else if (isPlayer && name == "MassPlayer")    mass = val;
        else if (!isPlayer && name == "MassPartner")  mass = val;
    }

    // 重力からジャンプ初速を計算
    if (G > 0) JUMP = -sqrtf(2.0f * G * tempHeight);
    else JUMP = -10.0f;
}

void Ball2D::Update() {
    Stage* stage = FindGameObject<Stage>();
    if (!stage) return;

    // 初回リセット位置保存
    static bool isFirstUpdate = true;
    if (isFirstUpdate) {
        startPosition = position;
        isFirstUpdate = false;
    }

    // ESCでリセット
    if (Input::IsKeyDown(KEY_INPUT_ESCAPE)) {
        ResetPosition();
        if (partner) partner->ResetPosition();
        return;
    }

    // 入力と重力
    bool isDownPressed = false;
    if (isPlayer) {
        float moveX = 0;
        if (Input::IsKeepKeyDown(KEY_INPUT_A)) moveX -= SPEED;
        if (Input::IsKeepKeyDown(KEY_INPUT_D)) moveX += SPEED;
        if (Input::IsKeepKeyDown(KEY_INPUT_S)) isDownPressed = true;

        if (stage->GetTileFunction(position.x, position.y + RADIUS + 1) == "ICE") {
            velocity.x += moveX * 0.2f; velocity.x *= 0.98f;
        }
        else {
            velocity.x += moveX; velocity.x *= 0.90f;
        }

        if (Input::IsKeyDown(KEY_INPUT_SPACE) && abs(velocity.y) < 1.0f) {
            velocity.y = JUMP;
        }
    }
    velocity.y += G;
    if (!isPlayer) velocity.x *= 0.95f;

    // スプリング
    if (isPlayer && partner) {
        VECTOR2 diff = position - partner->GetPosition();
        float dist = VSize(diff);
        if (dist > SPRING_L && dist > 0.1f) {
            float f = (dist - SPRING_L) * K;
            VECTOR2 norm = VECTOR2(diff.x / dist, diff.y / dist);
            partner->AddForce(VECTOR2(norm.x * f / partner->mass, norm.y * f / partner->mass));
        }
    }

    // --- 水平移動と壁判定 (ガクつき防止版) ---
    float oldX = position.x;
    position.x += velocity.x;

    // 進行方向の壁をチェック
    float sideX = (velocity.x > 0) ? (position.x + RADIUS) : (position.x - RADIUS);
    // 足元よりも少し上（段差許容範囲）をチェック
    std::string wallAttr = stage->GetTileFunction(sideX, position.y + RADIUS - 10.0f);

    if (wallAttr == "SOLID") {
        // 段差として登れるかチェック（頭上が空いているか）
        if (stage->GetTileFunction(position.x, position.y - RADIUS) == "NONE") {
            position.y -= 8.0f; // 以前の「坂道が登れる」魔法の数字
        }
        else {
            // 登れない壁なら、古い座標に戻して速度を殺す（ワープさせない）
            position.x = oldX;
            velocity.x = 0;
        }
    }

    // --- 垂直移動と床・坂道・ゴール判定 ---
    position.y += velocity.y;

    float footX = position.x;
    float footY = position.y + RADIUS;
    std::string attr = stage->GetTileFunction(footX, footY);

    // ゴール
    if (attr == "GOAL") {
        SceneManager::ChangeScene("CLEAR");
        return;
    }

    // 坂道判定
    if (attr == "SLOPE_R" || attr == "SLOPE_L") {
        float localX = fmod(footX, (float)stage->TILE_SIZE);
        if (localX < 0) localX += stage->TILE_SIZE;
        float tx = localX / (float)stage->TILE_SIZE;
        float ty = (attr == "SLOPE_R") ? (1.0f - tx) : tx;

        float tileBaseY = floor(footY / stage->TILE_SIZE) * stage->TILE_SIZE;
        float targetY = tileBaseY + (ty * stage->TILE_SIZE) - RADIUS;

        if (velocity.y >= 0 || position.y > targetY - 15.0f) {
            position.y = targetY;
            velocity.y = 0;
        }
    }
    // 床判定 (SOLIDは垂直方向のみ補正)
    else if (attr == "SOLID" || attr == "SPRING" || attr == "ICE" || attr == "ONE_WAY") {
        bool isLanding = false;
        if (attr == "ONE_WAY") {
            if (!isDownPressed) {
                float tileTopY = floor(footY / stage->TILE_SIZE) * stage->TILE_SIZE;
                if (velocity.y > 0 && footY >= tileTopY && footY <= tileTopY + velocity.y + 10.0f) {
                    isLanding = true;
                }
            }
        }
        else {
            if (velocity.y > 0) isLanding = true;
        }

        if (isLanding) {
            position.y = floor(footY / stage->TILE_SIZE) * stage->TILE_SIZE - RADIUS;
            if (attr == "SPRING") {
                velocity.y = JUMP * 1.5f;
                if (voiceHandle != -1) PlaySoundMem(voiceHandle, DX_PLAYTYPE_BACK);
            }
            else {
                velocity.y = 0;
            }
        }
    }

    // タイマー更新
    if (painTimer > 0) painTimer--;
    if (bump.active) { if (--bump.life <= 0) bump.active = false; }
}

void Ball2D::OnDamage() {
    painTimer = (int)(BUMP_MAX_LIFE * 0.5f);
}

void Ball2D::Draw() {
    Stage* st = FindGameObject<Stage>();
    float sx = st ? st->ScrollX() : 0, sy = st ? st->ScrollY() : 0;
    int dx = (int)(position.x - sx), dy = (int)(position.y - sy);

    if (isPlayer && partner) {
        VECTOR2 pPos = partner->GetPosition();
        DrawLine(dx, dy, (int)(pPos.x - sx), (int)(pPos.y - sy), GetColor(255, 255, 255), 2);
    }

    //画像の切り替え（ダメージ中なら dmgImgHandle を優先
    int currentImg = (painTimer > 0 && dmgImgHandle != -1) ? dmgImgHandle : hImage;

    if (currentImg != -1) {
        DrawExtendGraph(dx - (int)RADIUS, dy - (int)RADIUS, dx + (int)RADIUS, dy + (int)RADIUS, currentImg, TRUE);
    }
    else {
        DrawCircle(dx, dy, (int)RADIUS, color, TRUE);
    }

    if (bump.active) {
        float s = bump.life / BUMP_MAX_LIFE;
        DrawCircle((int)(dx + bump.localPos.x), (int)(dy + bump.localPos.y), (int)(12 * s), GetColor(255, 100, 100), TRUE);
    }

    // 座標のデバッグ表示（オレンジ側だけで表示）
    if (isPlayer) {
        DrawFormatString(10, 10, GetColor(255, 255, 0), "X:%.1f Y:%.1f", position.x, position.y);
    }
}