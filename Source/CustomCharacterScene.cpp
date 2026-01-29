#include "DxLib.h"
#include <math.h>
#include <vector>

// たんこぶの構造体
struct Bump {
    float lx, ly;    // ボール中心からの相対位置
    int life = 0;    // 残り時間
    bool active = false;
};

struct Ball {
    float x, y, vx, vy;
    float radius = 25.0f;
    int painTimer = 0;
    Bump bump;
    unsigned int color;
};

// 足場の構造体
struct Platform {
    int x1, y1, x2, y2;
};

// プロトタイプ宣言
void UpdatePhysics(Ball& a, Ball& b, const std::vector<Platform>& platforms);
void DrawBall(Ball& b, bool isPlayer);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ChangeWindowMode(TRUE);
    SetGraphMode(800, 600, 32);
    if (DxLib_Init() == -1) return -1;
    SetDrawScreen(DX_SCREEN_BACK);

    // 初期設定
    Ball a = { 100, 400, 0, 0, 25.0f, 0, {}, GetColor(255, 200, 0) }; // 操作側（黄）
    Ball b = { 160, 400, 0, 0, 25.0f, 0, {}, GetColor(100, 200, 255) }; // 追従側（青）

    // 足場の設置（タワー状）
    std::vector<Platform> platforms = {
        { 0, 550, 800, 600 },   // 地面
        { 300, 450, 500, 470 }, // 段差1
        { 500, 350, 700, 370 }, // 段差2
        { 200, 250, 400, 270 }, // 段差3
        { 400, 150, 600, 170 }  // 段差4
    };

    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0) {
        ClearDrawScreen();

        // 1. 操作（Ball A）
        if (CheckHitKey(KEY_INPUT_LEFT))  a.vx -= 0.6f;
        if (CheckHitKey(KEY_INPUT_RIGHT)) a.vx += 0.6f;
        // ジャンプ判定は物理更新の中で簡易的に行う（y速度がほぼ0ならジャンプ可）
        if (CheckHitKey(KEY_INPUT_SPACE) && abs(a.vy) < 0.1f) a.vy = -12.0f;

        // 2. 物理演算
        UpdatePhysics(a, b, platforms);

        // 3. 描画
        // 足場
        for (auto& p : platforms) DrawBox(p.x1, p.y1, p.x2, p.y2, GetColor(150, 150, 150), TRUE);

        // ひも
        DrawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y, GetColor(255, 255, 255), 2);

        // キャラ
        DrawBall(a, true);
        DrawBall(b, false);

        ScreenFlip();
    }

    DxLib_End();
    return 0;
}

void UpdatePhysics(Ball& a, Ball& b, const std::vector<Platform>& platforms) {
    const float G = 0.5f;          // 重力
    const float SPRING_L = 80.0f;  // ひもの長さ
    const float K = 0.08f;         // ひもの強さ

    // 重力と抵抗
    a.vy += G; b.vy += G;
    a.vx *= 0.9f; b.vx *= 0.9f;

    // ひもの拘束力（Ball BをAの方へ、AをBの方へ少し引く）
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist > SPRING_L) {
        float f = (dist - SPRING_L) * K;
        a.vx -= (dx / dist) * f * 0.5f;
        a.vy -= (dy / dist) * f * 0.5f;
        b.vx += (dx / dist) * f;
        b.vy += (dy / dist) * f;
    }

    // 移動適用
    a.x += a.vx; a.y += a.vy;
    b.x += b.vx; b.y += b.vy;

    // 足場・壁判定（簡易）
    Ball* balls[2] = { &a, &b };
    for (int i = 0; i < 2; i++) {
        Ball& ball = *balls[i];

        // 床判定
        for (auto& p : platforms) {
            if (ball.x > p.x1 && ball.x < p.x2 && ball.y + ball.radius > p.y1 && ball.y < p.y2) {
                ball.y = p.y1 - ball.radius;
                ball.vy = 0;
            }
        }

        // 壁判定（左右画面端）とたんこぶ発生
        if (ball.x < ball.radius || ball.x > 800 - ball.radius) {
            if (abs(ball.vx) > 5.0f) {
                ball.painTimer = 60;
                ball.bump.active = true;
                ball.bump.life = 120;
                ball.bump.lx = (ball.x < ball.radius) ? -15 : 15;
                ball.bump.ly = -20;
            }
            ball.x = (ball.x < ball.radius) ? ball.radius : 800 - ball.radius;
            ball.vx *= -0.5f;
        }

        // たんこぶと表情タイマー更新
        if (ball.painTimer > 0) ball.painTimer--;
        if (ball.bump.active) {
            ball.bump.life--;
            if (ball.bump.life <= 0) ball.bump.active = false;
        }
    }
}

void DrawBall(Ball& b, bool isPlayer) {
    // 本体
    DrawCircle((int)b.x, (int)b.y, (int)b.radius, b.color, TRUE);

    // 目（表情切り替え）
    if (b.painTimer > 0) {
        // 痛い顔（＞＜）
        DrawLine((int)b.x - 10, (int)b.y - 5, (int)b.x - 2, (int)b.y, GetColor(0, 0, 0), 2);
        DrawLine((int)b.x - 10, (int)b.y + 5, (int)b.x - 2, (int)b.y, GetColor(0, 0, 0), 2);
        DrawLine((int)b.x + 10, (int)b.y - 5, (int)b.x + 2, (int)b.y, GetColor(0, 0, 0), 2);
        DrawLine((int)b.x + 10, (int)b.y + 5, (int)b.x + 2, (int)b.y, GetColor(0, 0, 0), 2);
    }
    else {
        // 通常の目
        DrawCircle((int)b.x - 8, (int)b.y - 5, 3, GetColor(0, 0, 0), TRUE);
        DrawCircle((int)b.x + 8, (int)b.y - 5, 3, GetColor(0, 0, 0), TRUE);
    }

    // たんこぶ（徐々に小さくなる）
    if (b.bump.active) {
        float scale = b.bump.life / 120.0f;
        DrawCircle((int)(b.x + b.bump.lx), (int)(b.y + b.bump.ly), (int)(12 * scale), GetColor(255, 100, 100), TRUE);
    }
}