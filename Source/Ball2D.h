#pragma once
#include "../Library/Object2D.h"
#include <string>

class Ball2D : public Object2D {
public:
    Ball2D(unsigned int _color, bool _isPlayer);
    virtual ~Ball2D() {}

    void Update() override;
    void Draw() override;

	void LoadParam(std::string path);// パラメータ読み込み
	void OnDamage();// ダメージを受けたときの処理

    void SetPosition(VECTOR2 pos) { position = pos; }
	void AddForce(VECTOR2 force) { velocity = velocity + force; }
	void SetPartner(Ball2D* _partner) { partner = _partner; }// パートナーを設定

    float mass;

private:
    bool isPlayer;
    unsigned int color;
    Ball2D* partner = nullptr;

    VECTOR2 velocity;

    float RADIUS;
	float G;// 重力
	float SPEED;// 移動速度
	float JUMP;//ジャンプ力
	float SPRING_L;// スプリング自然長
	float K;// スプリング定数
    float BUMP_MAX_LIFE;

    int dmgImgHandle;   // ダメージ時の画像
    int voiceHandle;    // ボイス
    int painTimer = 0;  // ダメージ演出用タイマー

    struct Bump {
		bool active = false;// バンプエフェクトが有効かどうか
		float life = 0;// バンプエフェクトの残り寿命
		VECTOR2 localPos;// バンプエフェクトのローカル位置
    } bump;
};