#include "Input.h"
#include "DxLib.h"

namespace Input {
	//キーボード取得関連
	const int KEY_MAX = 255;
	char keyBuff[KEY_MAX];		//キーバッファ (現フレーム)
	char keyBuffOld[KEY_MAX];	//1つ前のキーバッファ(前フレーム) 
	char key_down[KEY_MAX];		//押された瞬間 
	char key_up[KEY_MAX];		//離された瞬間 
	char Key_Keep[KEY_MAX];		//押しっぱなし

	// マウス取得関連
	const int MOUSE_MAX = 3; // 左、右、中央ボタンを想定
	int mouseBuff;			// マウスボタンの状態 (現フレーム)
	int mouseBuffOld;		// マウスボタンの状態 (前フレーム)
	int mouse_down[MOUSE_MAX];	// 押された瞬間
	int mouse_up[MOUSE_MAX];	// 離された瞬間
	int Mouse_Keep[MOUSE_MAX];	// 押しっぱなしフレーム数

	// マウス座標/ホイール
	int mouseX;
	int mouseY;
	int mouseWheel;
}

void Input::MouseStateUpdate()
{
	// 1. 前フレームの状態を保存
	mouseBuffOld = mouseBuff;

	// 2. 現フレームの状態を取得 (DxLib)
	mouseBuff = GetMouseInput();

	// 3. 座標とホイールの取得 (DxLib)
	GetMousePoint(&mouseX, &mouseY);
	mouseWheel = GetMouseWheelRotVol();

	// 4. ボタンごとの状態を計算 (左(0), 右(1), 中央(2)の順)
	for (int i = 0; i < MOUSE_MAX; i++)
	{
		// DxLibのボタンフラグ (1:左, 2:右, 4:中央)
		int mouseCode = 1 << i;

		// 現フレームと前フレームのボタンの状態をチェック
		bool current_is_on = (mouseBuff & mouseCode) != 0;
		bool old_is_on = (mouseBuffOld & mouseCode) != 0;

		// 押しっぱなしのカウント更新
		if (current_is_on && old_is_on) {
			Mouse_Keep[i]++;
		}

		// 状態の変化をチェック (XORに相当)
		if (current_is_on ^ old_is_on) {
			Mouse_Keep[i] = 0; // 状態が変化したらリセット
		}

		// 押された瞬間
		if (current_is_on && !old_is_on) {
			mouse_down[i] = 1;
		}
		else {
			mouse_down[i] = 0;
		}

		// 離された瞬間
		if (!current_is_on && old_is_on) {
			mouse_up[i] = 1;
		}
		else {
			mouse_up[i] = 0;
		}
	}
}

bool Input::IsKeyUP(int keyCode)
{
	return(key_up[keyCode]);
}

bool Input::IsKeyDown(int keyCode)
{
	return(key_down[keyCode]);
}

int Input::IsKeepKeyDown(int keyCode)
{
	return(Key_Keep[keyCode]);
}

void Input::KeyStateUpdate()
{
	memcpy_s(keyBuffOld, sizeof(char) * KEY_MAX, keyBuff, sizeof(char) * KEY_MAX);

	GetHitKeyStateAll(keyBuff);//全てのキーの状態を取得 

	for (int i = 0; i < KEY_MAX; i++)
	{
		if (keyBuff[i] && keyBuffOld[i]) Key_Keep[i]++;
		int key_xor = keyBuff[i] ^ keyBuffOld[i];	//前フレームと現フレームのxor
		if (key_xor) Key_Keep[i] = 0;
		key_down[i] = key_xor & keyBuff[i];		//押された瞬間 = (現フレームとkey_xorのAND) 
		key_up[i] = key_xor & keyBuffOld[i];	//離された瞬間 = (前フレームとkey_xorのAND) 
	}
}

bool Input::IsMouseUP(int mouseCode)
{
	// mouseCode (1, 2, 4...) から配列インデックス (0, 1, 2...) を計算
	int index = 0;
	// 0ではない && 1ではない (1=左ボタン) の場合、右シフトしてインデックスを探す
	while (mouseCode > 1) {
		mouseCode >>= 1;
		index++;
	}
	if (index >= 0 && index < MOUSE_MAX) {
		return mouse_up[index] != 0;
	}
	return false;
}

bool Input::IsMouseDown(int mouseCode)
{
	// mouseCode から配列インデックスを計算
	int index = 0;
	while (mouseCode > 1) {
		mouseCode >>= 1;
		index++;
	}
	if (index >= 0 && index < MOUSE_MAX) {
		return mouse_down[index] != 0;
	}
	return false;
}

int Input::IsKeepMouseDown(int mouseCode)
{
	// mouseCode から配列インデックスを計算
	int index = 0;
	while (mouseCode > 1) {
		mouseCode >>= 1;
		index++;
	}
	if (index >= 0 && index < MOUSE_MAX) {
		return Mouse_Keep[index];
	}
	return 0;
}

int Input::GetMouseX()
{
	return mouseX;
}

int Input::GetMouseY()
{
	return mouseY;
}

int Input::GetMouseWheel()
{
	return mouseWheel;
}

