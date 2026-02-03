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

		if (name == "Radius")           RADIUS = val;// 半径
		else if (name == "Gravity")     G = val;// 重力加速度
		else if (name == "MoveSpeed")   SPEED = val;// 移動速度
		else if (name == "JumpHeight")  tempHeight = val;// ジャンプの高さ（初速計算用）
		else if (name == "BumpLife")    BUMP_MAX_LIFE = val;// バンプエフェクトの最大寿命
		else if (name == "SpringLength") SPRING_L = val;// 紐の自然長
		else if (name == "SpringK")      K = val;// バネ定数
		else if (isPlayer && name == "MassPlayer")    mass = val;// プレイヤー用の質量
		else if (!isPlayer && name == "MassPartner")  mass = val;// パートナー用の質量
    }

    // 重力からジャンプ初速を計算
    if (G > 0) JUMP = -sqrtf(2.0f * G * tempHeight);
    else JUMP = -10.0f;
}

void Ball2D::Update() {
    Stage* stage = FindGameObject<Stage>();
    if (!stage) return;

    // 初回の更新時にリセット用の初期位置を保存します
    static bool isFirstUpdate = true;
    if (isFirstUpdate) {
        startPosition = position;
        isFirstUpdate = false;
    }

    // エスケープキーが押されたら自分とパートナーの位置を初期状態に戻します
    if (Input::IsKeyDown(KEY_INPUT_ESCAPE)) {
        ResetPosition();
        if (partner) partner->ResetPosition();
        return;
    }

    float moveInput = 0;
    bool isDownPressed = false;

    // プレイヤー操作時の移動入力とジャンプの判定
    if (isPlayer) {
        if (Input::IsKeepKeyDown(KEY_INPUT_A)) moveInput -= SPEED;
        if (Input::IsKeepKeyDown(KEY_INPUT_D)) moveInput += SPEED;
        if (Input::IsKeepKeyDown(KEY_INPUT_S)) isDownPressed = true;

        // 接地に近い状態であればスペースキーでジャンプ初速を与えます
        if (Input::IsKeyDown(KEY_INPUT_SPACE) && abs(velocity.y) < 1.0f) {
            velocity.y = JUMP;
        }
    }

    // パートナーとの間にある紐の伸縮による物理計算
    if (isPlayer && partner) {
        VECTOR2 diff = position - partner->GetPosition();
        float dist = VSize(diff);
        if (dist > SPRING_L && dist > 0.1f) {
            float f = (dist - SPRING_L) * K;
            VECTOR2 norm = VECTOR2(diff.x / dist, diff.y / dist);
            // 相手側へ引っ張る力を加えます
            partner->AddForce(VECTOR2(norm.x * f / partner->mass, norm.y * f / partner->mass));
        }
    }

    // 重力、摩擦、移動、およびトゲ床などのギミック判定をステージギミッククラスに委託
    // 最後に this を渡すことでギミック側から OnDamage を呼び出せるようにしています
    gimmick.SetParams(G, JUMP);
    gimmick.UpdatePhysics(position, velocity, RADIUS, isPlayer, isDownPressed, moveInput, voiceHandle, this);

    // ダメージ演出用のタイマーとバンプエフェクトの残り時間を更新します
    if (painTimer > 0) painTimer--;
    if (bump.active) {
        if (--bump.life <= 0) bump.active = false;
    }
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