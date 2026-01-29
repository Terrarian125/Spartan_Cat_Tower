#pragma once
#include <DxLib.h>

class Camera {
public:
    Camera();
    void Update(); // 入力による移動などを書く
    void Set();    // 実際にDXLibのカメラを適用する
    VECTOR m_Eye;    // 位置
    VECTOR m_Target; // 注視点
    float  m_Angle;  // 回転角度（ラジアン）
private:
};