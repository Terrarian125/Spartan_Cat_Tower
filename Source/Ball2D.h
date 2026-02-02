#pragma once
#include "../Library/Object2D.h"
#include "StageGimmick.h"
#include <string>

class Ball2D : public Object2D {
public:
    Ball2D(unsigned int _color, bool _isPlayer);
    virtual ~Ball2D() {}

    void Update() override;
    void Draw() override;

    void LoadParam(std::string path);
    void OnDamage();

    void SetPosition(VECTOR2 pos) { position = pos; }
    void AddForce(VECTOR2 force) { velocity = velocity + force; }
    void SetPartner(Ball2D* _partner) { partner = _partner; }

    // ダメージ回数を取得する関数
    int GetDamageCount() const { return damageCount; }

    float mass;

private:
    StageGimmick gimmick;
    bool isPlayer;
    unsigned int color;
    Ball2D* partner = nullptr;

    VECTOR2 velocity;
    VECTOR2 startPosition;
    void ResetPosition() { position = startPosition; velocity = VECTOR2(0, 0); }

    float RADIUS;
    float G;
    float SPEED;
    float JUMP;
    float SPRING_L;
    float K;
    float BUMP_MAX_LIFE;

    int dmgImgHandle;
    int voiceHandle;
    int painTimer = 0;
    int damageCount = 0; // ダメージを受けた総回数を記録する変数

    struct Bump {
        bool active = false;
        float life = 0;
        VECTOR2 localPos;
    } bump;
};