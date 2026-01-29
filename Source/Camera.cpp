#include "Camera.h"
#include <math.h>
#include "../Library/Input.h"

Camera::Camera() {
    m_Eye = VGet(0.0f, 200.0f, -500.0f);
    m_Target = VGet(0.0f, 0.0f, 0.0f);
    m_Angle = 0.0f;
}

void Camera::Update() {
    //回転処理 (Q, E,↑,↓)
    if (Input::IsKeepKeyDown(KEY_INPUT_Q)) m_Angle -= 0.05f;
    if (Input::IsKeepKeyDown(KEY_INPUT_E)) m_Angle += 0.05f;
    if (Input::IsKeepKeyDown(KEY_INPUT_LEFT)) m_Angle -= 0.05f;
    if (Input::IsKeepKeyDown(KEY_INPUT_RIGHT)) m_Angle += 0.05f;
    //前後・左右移動 (WASD)
    if (!Input::IsKeepKeyDown(KEY_INPUT_LSHIFT)) {
        if (Input::IsKeepKeyDown(KEY_INPUT_W)) m_Eye.z += 5.0f;
        if (Input::IsKeepKeyDown(KEY_INPUT_S)) m_Eye.z -= 5.0f;
    }
    if (Input::IsKeepKeyDown(KEY_INPUT_A)) m_Eye.x -= 5.0f;
    if (Input::IsKeepKeyDown(KEY_INPUT_D)) m_Eye.x += 5.0f;

    //高さ（Y軸）の変更
    float heightSpeed = 5.0f;

    // 1. Shift + W / S で上下
    if (Input::IsKeepKeyDown(KEY_INPUT_LSHIFT)) {
        if (Input::IsKeepKeyDown(KEY_INPUT_W)) m_Eye.y += heightSpeed;
        if (Input::IsKeepKeyDown(KEY_INPUT_S)) m_Eye.y -= heightSpeed;
    }

    // 2. アローキー (UP / DOWN) で上下
    if (Input::IsKeepKeyDown(KEY_INPUT_UP))   m_Eye.y += heightSpeed;
    if (Input::IsKeepKeyDown(KEY_INPUT_DOWN)) m_Eye.y -= heightSpeed;

    // 3. マウスホイールで上下
    // GetMouseWheel() は 奥に回すとプラス、手前に回すとマイナスの値
    int wheel = Input::GetMouseWheel();
    //if (wheel != 0) {
    //    m_Eye.y += (float)wheel * 20.0f; // ホイールは1目盛りで大きな値が入るので調整
    //}

    // --- 注視点の更新 ---
    m_Target.x = m_Eye.x + sinf(m_Angle);
    m_Target.z = m_Eye.z + cosf(m_Angle);
    m_Target.y = m_Eye.y; // カメラと同じ高さを見る（水平）
}

void Camera::Set() {
    SetCameraNearFar(1.0f, 100000.0f);
    SetCameraPositionAndTarget_UpVecY(m_Eye, m_Target);
}