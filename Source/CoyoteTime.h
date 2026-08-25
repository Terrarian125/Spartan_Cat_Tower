#pragma once

class CoyoteTime {
public:
    CoyoteTime();

    void Update();
    void Start();
    void SetGrounded(bool grounded);

    bool CanJump() const;

private:
    static constexpr float COYOTE_TIME = 0.3f;

    float timer;
    bool isGrounded;
    int lastTime;
};