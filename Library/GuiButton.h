#pragma once
#include "GameObject.h"
#include <functional>
#include <string>

class GuiButton : public GameObject {
public:
    std::function<void()> onClick;

    GuiButton(int x, int y, int w, int h, std::string text);
    void Update() override;
    void Draw() override;

	void SetFocus(bool f) { isFocused = f; }// フォーカスされているかどうかを設定する関数
	void SetActive(bool a) { active = a; }// アクティブかどうかを設定する関数
	bool IsActive() const { return active; }// アクティブかどうかを取得する関数
	bool IsMouseOver();// マウスオーバーしているかを判定する関数

	void SetDescription(std::string desc) { description = desc; }// 説明文を設定するための関数
	std::string GetDescription() const { return description; }// 説明文を取得するための関数
private:
    int x, y, width, height;
    unsigned int color, hoverCol;
    std::string label;
    bool isFocused;
    bool active;
	std::string description;// 説明文用
};