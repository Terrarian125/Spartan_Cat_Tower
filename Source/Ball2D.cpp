#include "Ball2D.h"
#include "Stage.h"
#include "../Library/Input.h"
#include "../Library/CsvReader.h"
#include <math.h>
#include "StageGimmick.h"

int Ball2D::lastTotalDamage = 0;

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

    //デフォルト値
    RADIUS = 25.0f; G = 0.5f; SPEED = 0.6f;
    SPRING_L = 250.0f; K = 0.08f; mass = 1.0f;
    BUMP_MAX_LIFE = 120.0f;
    float tempHeight = 250.0f; //CSVのJumpHeightの初期値

    if (csv.GetLines() <= 0) return;

    for (int i = 0; i < csv.GetLines(); i++) {
        std::string name = csv.GetString(i, 0);
        std::string fileName = csv.GetString(i, 1);

        //文字列として処理する項目（数値変換をスキップ） 
        if (name == "PlayerImage" || name == "PartnerImage") {
            if ((isPlayer && name == "PlayerImage") || (!isPlayer && name == "PartnerImage")) {
                hImage = LoadGraph(("Data/Image/" + fileName).c_str());
            }
            continue; //数値変換させない
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
        //GetFloatを呼ぶ前に、値が空でないか、数値っぽいかを確認するのが安全
        float val = (float)csv.GetFloat(i, 1);

		if (name == "Radius")           RADIUS = val;//半径
		else if (name == "Gravity")     G = val;//重力加速度
		else if (name == "MoveSpeed")   SPEED = val;//移動速度
		else if (name == "JumpHeight")  tempHeight = val;//ジャンプの高さ（初速計算用）
		else if (name == "BumpLife")    BUMP_MAX_LIFE = val;//バンプエフェクトの最大寿命
		else if (name == "SpringLength") SPRING_L = val;//紐の自然長
		else if (name == "SpringK")      K = val;//バネ定数
		else if (isPlayer && name == "MassPlayer")    mass = val;//プレイヤー用の質量
		else if (!isPlayer && name == "MassPartner")  mass = val;//パートナー用の質量
    }

    //重力からジャンプ初速を計算
    if (G > 0) JUMP = -sqrtf(2.0f * G * tempHeight);
    else JUMP = -10.0f;
}

void Ball2D::Update() {
    Stage* stage = FindGameObject<Stage>();
    if (!stage) return;

    //判定用の変数を先に宣言・初期化する
    float moveInput = 0;
    bool isDownPressed = false;
    VECTOR2 oldVelocity = velocity; //更新前の速度を保存（激突判定用）

    //初回の更新時にリセット用の初期位置を保存
    static bool isFirstUpdate = true;
    if (isFirstUpdate) {
        startPosition = position;
        isFirstUpdate = false;
    }

    //プレイヤー操作時の入力処理
    if (isPlayer) {
        if (Input::IsKeepKeyDown(KEY_INPUT_A)) moveInput -= SPEED;
        if (Input::IsKeepKeyDown(KEY_INPUT_D)) moveInput += SPEED;
        if (Input::IsKeepKeyDown(KEY_INPUT_S)) isDownPressed = true;

        if (Input::IsKeyDown(KEY_INPUT_SPACE) && abs(velocity.y) < 1.0f) {
            velocity.y = JUMP;
        }
    }

    //パートナーとの間にある紐の物理計算
    if (isPlayer && partner) {
        VECTOR2 diff = position - partner->GetPosition();
        float dist = VSize(diff);
        if (dist > SPRING_L && dist > 0.1f) {
            float f = (dist - SPRING_L) * K;
            VECTOR2 norm = VECTOR2(diff.x / dist, diff.y / dist);
            partner->AddForce(VECTOR2(norm.x * f / partner->mass, norm.y * f / partner->mass));
        }
    }

    //物理更新
    gimmick.SetParams(G, JUMP);
    gimmick.UpdatePhysics(position, velocity, RADIUS, isPlayer, isDownPressed, moveInput, voiceHandle, this);

    //ダメージ判定(落下は平気、壁激突と極端な引っ張りのみ
    if (!isPlayer) {
        //壁への激突判定（横方向の速度が急停止した場合）
        if (fabsf(oldVelocity.x) > 15.0f && fabsf(velocity.x) < 1.0f) {
            OnDamage();
        }

        //ひもの限界ダメージ（SPRING_Lのn倍以上離れたら）
        if (partner) {
            VECTOR2 diff = position - partner->GetPosition();
            float dist = VSize(diff);
			if (dist > SPRING_L * 2.0f) {//ここの2.0fは調整用の係数
                if (painTimer <= 0) OnDamage();
            }
        }
    }

    //たんこぶ（bump）の更新
    if (painTimer > 0) painTimer--;
    if (bump.active) {
        if (--bump.life <= 0) bump.active = false;
    }
}

void Ball2D::OnDamage() {
    if (painTimer <= 0) { //無敵時間中でなければ
        painTimer = 180;  //3秒間ダメージ顔

        if (!isPlayer) {
            damageCount++;

            //ダメージに応じた音量計算
            int volume = 255;
            if (damageCount >= 5) {
                //5回から小さくなり始め、30回で最小になる計算
                //輝度と同じく 255 - (damageCount - 5) * 9
                volume = 255 - (damageCount - 5) * 9;
                if (volume < 30) volume = 30; //完全に消えると寂しいので最低値を設定
            }

            //ボイスの音量を変更
            if (voiceHandle != -1) {
                ChangeVolumeSoundMem(volume, voiceHandle);
                PlaySoundMem(voiceHandle, DX_PLAYTYPE_BACK);
            }
        }

        //たんこぶ出現
        bump.active = true;
        bump.life = 120;
    }
}

void Ball2D::Draw() {
    Stage* st = FindGameObject<Stage>();
    float sx = st ? st->ScrollX() : 0, sy = st ? st->ScrollY() : 0;
    int dx = (int)(position.x - sx), dy = (int)(position.y - sy);

    if (isPlayer && partner) {
        VECTOR2 pPos = partner->GetPosition();
        DrawLine(dx, dy, (int)(pPos.x - sx), (int)(pPos.y - sy), GetColor(255, 255, 255), 2);
    }

    //ダメージに応じた黒ずみ（輝度）計算 
    int brightness = 255;
    if (!isPlayer && damageCount >= 5) {
        //5回から開始して30回でほぼ黒になる計算
        //255 - (現在のダメージ - 5) * 9
        brightness = 255 - (damageCount - 5) * 9;
        if (brightness < 30) brightness = 30; //最低値を30に固定
    }

    //画像描画前に輝度を設定
    SetDrawBright(brightness, brightness, brightness);

    //画像の切り替え（ダメージ中なら dmgImgHandle を優先）
    int currentImg = (painTimer > 0 && dmgImgHandle != -1) ? dmgImgHandle : hImage;

    if (currentImg != -1) {
        DrawExtendGraph(dx - (int)RADIUS, dy - (int)RADIUS, dx + (int)RADIUS, dy + (int)RADIUS, currentImg, TRUE);
    }
    else {
        //画像がない場合の円にも輝度が適用されます
        DrawCircle(dx, dy, (int)RADIUS, color, TRUE);
    }

    //たんこぶやUIを暗くしないために輝度をリセット
    SetDrawBright(255, 255, 255);

    ////たんこぶ出現
    //if (bump.active) {
    //   float s = bump.life / BUMP_MAX_LIFE;
    //   DrawCircle((int)(dx + bump.localPos.x), (int)(dy + bump.localPos.y), (int)(12 * s), GetColor(255, 100, 100), TRUE);
    //}

    //座標のデバッグ表示
    if (isPlayer) {
        DrawFormatString(10, 10, GetColor(255, 255, 255), "X:%.1f Y:%.1f", position.x, position.y);
    }
    //ダメージ回数の表示
    if (!isPlayer) {
        DrawFormatString(10, 30, GetColor(255, 255, 255), "Damage: %d", damageCount);
    }
    if (isPlayer) {
        gimmick.DrawFade();
    }

}