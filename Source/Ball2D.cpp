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

        // --- 文字列として処理する項目（数値変換をスキップ） ---
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

        // --- ここから数値変換が必要な項目 ---
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

    //移動入力
    if (isPlayer) {
        float moveX = 0;
        if (Input::IsKeepKeyDown(KEY_INPUT_A)) moveX -= SPEED;
        if (Input::IsKeepKeyDown(KEY_INPUT_D)) moveX += SPEED;

        if (stage->GetTileFunction(position.x, position.y + RADIUS + 1) == "ICE") {
            velocity.x += moveX * 0.2f; velocity.x *= 0.98f;
        }
        else {
            velocity.x += moveX; velocity.x *= 0.90f;
        }

        if (Input::IsKeyDown(KEY_INPUT_SPACE) && abs(velocity.y) < 0.5f) {
            velocity.y = JUMP;
            // プレイヤーが自力で飛んだ時もボイスを鳴らすならここ
            // if (voiceHandle != -1) PlaySoundMem(voiceHandle, DX_PLAYTYPE_BACK);
        }
    }

    //重力と摩擦
    velocity.y += G;
    if (!isPlayer) velocity.x *= 0.95f;

    //スプリング
    if (isPlayer && partner) {
        VECTOR2 diff = position - partner->GetPosition();
        float dist = VSize(diff);
        if (dist > SPRING_L && dist > 0.001f) {
            float f = (dist - SPRING_L) * K;
            VECTOR2 norm = VECTOR2(diff.x / dist, diff.y / dist);
            partner->AddForce(VECTOR2(norm.x * f / partner->mass, norm.y * f / partner->mass));
        }
    }

    //更新
    position = position + velocity;

    //地面・バネ(SPRING)判定
    float footX = position.x;
    float footY = position.y + RADIUS;
    std::string attr = stage->GetTileFunction(footX, footY);

    if (attr == "SOLID" || attr == "SPRING") {
        if (velocity.y > 0) {
            position.y = (float)(floor(footY / stage->TILE_SIZE) * stage->TILE_SIZE - RADIUS);
            if (attr == "SPRING") {
                velocity.y = JUMP * 1.5f;
                // バネで跳ねた時にボイス再生
                if (voiceHandle != -1) PlaySoundMem(voiceHandle, DX_PLAYTYPE_BACK);
            }
            else {
                velocity.y = 0;
            }
        }
    }

    //壁判定とダメージ
    float sideY = position.y;
    if (stage->GetTileFunction(position.x - RADIUS, sideY) == "SOLID" ||
        stage->GetTileFunction(position.x + RADIUS, sideY) == "SOLID") {

        if (!isPlayer && abs(velocity.x) > 5.0f) {
            OnDamage();
            // 壁激突時にボイス再生
            if (voiceHandle != -1) PlaySoundMem(voiceHandle, DX_PLAYTYPE_BACK);

            bump.active = true;
            bump.life = BUMP_MAX_LIFE;
            bump.localPos = VECTOR2((velocity.x < 0) ? -15.0f : 15.0f, -20.0f);
        }
        velocity.x *= -0.5f;
    }

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
}