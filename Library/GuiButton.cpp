#include "GuiButton.h"
#include <DxLib.h>
#include "../Library/Input.h"

GuiButton::GuiButton(int _x, int _y, int _w, int _h, std::string text)
    : x(_x), y(_y), width(_w), height(_h), label(text), active(true), isFocused(false) {
    color = GetColor(60, 60, 60);
    hoverCol = GetColor(120, 120, 120);
    SetDrawOrder(10);
}

void GuiButton::Update() {
    if (!active) return;

    // 前フレームでクリック処理をしたなら、このフレームはもう何もしない
    // (静的変数を使って全ボタンの二重反応を防止)
    static int lastUpdateFrame = -1;
    int currentFrame = GetNowCount(); // DxLibのミリ秒取得

    if (IsMouseOver() || isFocused) {
        bool triggered = false;

        // マウスかキーボード、どちらか片方。かつ「離した瞬間」
        if (Input::IsMouseUP(MOUSE_INPUT_LEFT)) {
            triggered = true;
        }
        else if (Input::IsKeyUP(KEY_INPUT_RETURN)) {
            triggered = true;
        }

        //もし反応したなら、一定時間（例えば200ミリ秒）は再反応させない
		//1クリックで2回増える問題を無理やり防止するためにつけた
        static LONGLONG lastTriggerTime = 0;
        LONGLONG now = GetNowHiPerformanceCount();

        if (triggered && onClick) {
            // 前回から0.1秒以上経っていないと無視
            if (now - lastTriggerTime > 100000) {
                onClick();
                lastTriggerTime = now;
            }
        }
    }
}

void GuiButton::Draw() {
    if (!active) return;
    bool highlight = IsMouseOver() || isFocused;
    unsigned int drawCol = highlight ? hoverCol : color;

    DrawBox(x, y, x + width, y + height, drawCol, TRUE);
    DrawBox(x, y, x + width, y + height, GetColor(255, 255, 255), FALSE);

    int tw = GetDrawStringWidth(label.c_str(), (int)label.length());
    DrawString(x + (width - tw) / 2, y + (height / 2) - 8, label.c_str(), GetColor(255, 255, 255));
}

bool GuiButton::IsMouseOver() {
    if (!active) return false;
    int mx = Input::GetMouseX();
    int my = Input::GetMouseY();
    return (mx >= x && mx <= x + width && my >= y && my <= y + height);
}