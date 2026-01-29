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

    //  初回実行時に初期位置を確定 (ESCリセット用) 
    static bool isFirstUpdate = true;
    if (isFirstUpdate) {
        startPosition = position;
        isFirstUpdate = false;
    }

    //  ESCキーで初期位置にリセット 
    if (Input::IsKeyDown(KEY_INPUT_ESCAPE)) {
        ResetPosition();
        if (partner) partner->ResetPosition();
        return;
    }

    //  1. 入力・重力・摩擦処理 
    if (isPlayer) {
        float moveX = 0;
        if (Input::IsKeepKeyDown(KEY_INPUT_A)) moveX -= SPEED;
        if (Input::IsKeepKeyDown(KEY_INPUT_D)) moveX += SPEED;

        // 足元のタイルが ICE (滑る床) かどうかで加速と摩擦を変える
        if (stage->GetTileFunction(position.x, position.y + RADIUS + 1) == "ICE") {
            velocity.x += moveX * 0.2f; // 加速しにくい
            velocity.x *= 0.98f;        // 止まりにくい
        }
        else {
            velocity.x += moveX;
            velocity.x *= 0.90f;        // 通常の摩擦
        }

        // ジャンプ処理 (地面にほぼ接している時のみ)
        if (Input::IsKeyDown(KEY_INPUT_SPACE) && abs(velocity.y) < 1.0f) {
            velocity.y = JUMP;
        }
    }
    velocity.y += G; // 重力加算
    if (!isPlayer) velocity.x *= 0.95f; // パートナーは常に少し摩擦がある

    //  2. スプリング(ひも)の計算 
    if (isPlayer && partner) {
        VECTOR2 diff = position - partner->GetPosition();
        float dist = VSize(diff);
        if (dist > SPRING_L && dist > 0.1f) {
            float f = (dist - SPRING_L) * K;
            VECTOR2 norm = VECTOR2(diff.x / dist, diff.y / dist);
            partner->AddForce(VECTOR2(norm.x * f / partner->mass, norm.y * f / partner->mass));
        }
    }

    //  3. 水平移動と壁判定 
    position.x += velocity.x;
    float stepHeight = 10.0f; // 段差乗り越え許容値
    float checkYs[] = { -RADIUS, 0.0f, RADIUS - stepHeight };

    for (float offY : checkYs) {
        float sideX = (velocity.x > 0) ? (position.x + RADIUS) : (position.x - RADIUS);
        std::string attr = stage->GetTileFunction(sideX, position.y + offY);

        // SOLID(壁) の場合のみ水平方向の押し戻しを行う
        if (attr == "SOLID") {
            if (velocity.x > 0) position.x = floor(sideX / stage->TILE_SIZE) * stage->TILE_SIZE - RADIUS - 0.01f;
            else position.x = ceil(sideX / stage->TILE_SIZE) * stage->TILE_SIZE + RADIUS + 0.01f;

            if (!isPlayer && abs(velocity.x) > 5.0f) {
                OnDamage();
                if (voiceHandle != -1) PlaySoundMem(voiceHandle, DX_PLAYTYPE_BACK);
            }
            velocity.x = 0;
            break;
        }
    }

    //  4. 垂直移動と床・坂道・ゴール判定 
    position.y += velocity.y;
    float footX = position.x;
    float footY = position.y + RADIUS;
    std::string footAttr = stage->GetTileFunction(footX, footY);

    // [GOAL] ゴール判定
    if (footAttr == "GOAL") {
        SceneManager::ChangeScene("CLEAR");
        return;
    }

    // [SLOPE] 坂道判定
    if (footAttr == "SLOPE_R" || footAttr == "SLOPE_L") {
        float tx = fmod(footX, (float)stage->TILE_SIZE) / (float)stage->TILE_SIZE;
        if (tx < 0) tx += 1.0f;
        float ty = (footAttr == "SLOPE_R") ? (1.0f - tx) : tx;
        float targetY = floor(footY / stage->TILE_SIZE) * stage->TILE_SIZE + (ty * stage->TILE_SIZE) - RADIUS;

        // 下降中、または坂道にわずかにめり込んでいる時に吸着
        if (velocity.y >= 0 || position.y > targetY - 10.0f) {
            position.y = targetY;
            velocity.y = 0;
        }
    }
    // [FLOOR] 通常床・バネ・滑る床・すり抜け床
    else if (footAttr == "SOLID" || footAttr == "SPRING" || footAttr == "ICE" || footAttr == "ONE_WAY") {
        bool isLanding = false;

        if (footAttr == "ONE_WAY") {
            // 下向き移動中で、かつ足元がタイルの上端付近にある時だけ着地
            float tileTopY = floor(footY / stage->TILE_SIZE) * stage->TILE_SIZE;
            if (velocity.y > 0 && footY >= tileTopY && footY <= tileTopY + velocity.y + 5.0f) {
                isLanding = true;
            }
        }
        else {
            // それ以外は落下中なら着地
            if (velocity.y > 0) isLanding = true;
        }

        if (isLanding) {
            position.y = floor(footY / stage->TILE_SIZE) * stage->TILE_SIZE - RADIUS;

            if (footAttr == "SPRING") {
                velocity.y = JUMP * 1.5f;
                if (voiceHandle != -1) PlaySoundMem(voiceHandle, DX_PLAYTYPE_BACK);
            }
            else {
                velocity.y = 0;
            }
        }
    }

    //  5. 演出・タイマー更新 
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