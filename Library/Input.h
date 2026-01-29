#pragma once


namespace Input {
	//キーボード取得関連
	void KeyStateUpdate();
	bool IsKeyUP(int keyCode);   // 離された瞬間
	bool IsKeyDown(int keyCode); // 押された瞬間
	int IsKeepKeyDown(int keyCode); // 押しっぱなしのフレーム数


	// マウス取得関連
	
	void MouseStateUpdate();
	bool IsMouseUP(int mouseCode);   // 離された瞬間
	bool IsMouseDown(int mouseCode); // 押された瞬間
	int IsKeepMouseDown(int mouseCode); // 押しっぱなしのフレーム数

	// マウス座標取得
	int GetMouseX();
	int GetMouseY();

	// マウスホイール取得
	int GetMouseWheel();


	// 音量レベル定義
	const int MAX_VOLUME_VALUE = 255;	// 基準となる最大値を定義

	// ５段階音量レベル
	const int Volume_5 = MAX_VOLUME_VALUE * 2;
	const int Volume_4 = MAX_VOLUME_VALUE + 128;
	const int Volume_3 = MAX_VOLUME_VALUE;
	const int Volume_2 = MAX_VOLUME_VALUE / 2;
	const int Volume_1 = MAX_VOLUME_VALUE / 4;
	const int Volume_0 = 0;
}