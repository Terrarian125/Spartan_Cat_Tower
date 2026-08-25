#include "CoyoteTime.h"
#include <DxLib.h>

CoyoteTime::CoyoteTime() {
    timer = 0.0f;
    isGrounded = false;
    lastTime = GetNowCount();
}

void CoyoteTime::Update() {
    int currentTime = GetNowCount();
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    if (isGrounded) {
        timer = 0.0f;
        return;
    }

    if (timer > 0.0f) {
        timer -= deltaTime;

        if (timer < 0.0f) {
            timer = 0.0f;
        }
    }
}

void CoyoteTime::Start() {
    timer = COYOTE_TIME;
}

void CoyoteTime::SetGrounded(bool grounded) {
    isGrounded = grounded;

    if (grounded) {
        timer = 0.0f;
    }
}

bool CoyoteTime::CanJump() const {
    return isGrounded || timer > 0.0f;
}