#pragma once
#include "../Library/Object2D.h"
#include "StageGimmick.h"
#include "CoyoteTime.h"
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
    void SetVelocity(VECTOR2 vel) { velocity = vel; }
    void AddForce(VECTOR2 force) { velocity = velocity + force; }
    void SetPartner(Ball2D* _partner) { partner = _partner; }

    //ダメージ回数を取得
    int GetDamageCount() const { return damageCount; }

    static int lastTotalDamage;

    Ball2D* GetPartner() const { return partner; }

    float mass;

private:
    StageGimmick gimmick;
    CoyoteTime coyoteTime;

    bool isPlayer;
    unsigned int color;
    Ball2D* partner = nullptr;

    VECTOR2 velocity;
    VECTOR2 startPosition;

    void ResetPosition() {
        position = startPosition;
        velocity = VECTOR2(0, 0);
    }

    float RADIUS;
    float G;
    float SPEED;
    float JUMP;
    float SPRING_L;
    float K;
    float BUMP_MAX_LIFE;

    //パートナー救済用
    static constexpr int RESCUE_TIME = 3000;
    static constexpr float RESCUE_DISTANCE = 2.0f;
    int rescueStartTime = -1;

    int dmgImgHandle;
    int voiceHandle;
    int painTimer = 0;
    int damageCount = 0;

    //デバッグ表示
    static bool debugVisible;
    static int debugLastTime;
    static float debugFPS;
    static float debugMinFPS;
    static int debugFPSResetTime;

    //物理処理時間
    static float debugPhysicsTime;

    struct Bump {
        bool active = false;
        float life = 0;
        VECTOR2 localPos;
    } bump;
};