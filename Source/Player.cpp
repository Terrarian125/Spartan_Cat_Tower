#include "Player.h"
#include <assert.h>
#include "../Library/Input.h"
#include "Stage.h"
#include "../ImGui/imgui.h"
#include "../Library/CsvReader.h"
#include <stdlib.h> // rand() を使うために必要
#include <time.h>
//#include "Fish.h"

Player::Player() : Player(VECTOR2(100, 200))
{
}

Player::Player(VECTOR2 pos)
{
	// パラメーターを読む
	CsvReader* csv = new CsvReader("Data/Stage/playerParam.csv");
	for (int i = 0; i < csv->GetLines(); i++) {
		std::string tag = csv->GetString(i, 0);
		if (tag == "Gravity") {
			Gravity = csv->GetFloat(i, 1);
		}
		else if (tag == "JumpHeight") {
			JumpHeight = csv->GetFloat(i, 1);
		}
		else if (tag == "MoveSpeed") {
			moveSpeed = csv->GetFloat(i, 1);
		}
	}
	// ジャンプ初速の計算 (v^2 = v0^2 + 2ay より v0 = -sqrt(2 * a * y))
	JumpV0 = -sqrtf(2.0f * Gravity * JumpHeight);

	// 画像とサウンドのロード
	hImage = LoadGraph("Data/Image/Player/4cat.png");
	assert(hImage > 0);

	hSound = LoadSoundMem("Data/Music/jump.mp3");
	ChangeVolumeSoundMem(Input::Volume_1, hSound);

	// 初期設定
	imageSize = VECTOR2(64, 64);
	anim = 0;
	animY = 0;
	animTimer = -1;

	position = pos;
	velocityY = 0.0f;
}

Player::~Player()
{

}

void Player::Update()
{
	animTimer++;
	animY = 0;

	Stage* st = FindGameObject<Stage>(); // ステージオブジェクトを取得

	// 左右移動処理
	if (CheckHitKey(KEY_INPUT_D)) {
		position.x += moveSpeed;
		// 右側のステージ当たり判定と押し戻し処理
		int push = st->CheckRight(position + VECTOR2(24, -31)); // 右上
		position.x -= push;
		push = st->CheckRight(position + VECTOR2(24, 31)); // 右下
		position.x -= push;
		//animTimer++;
		animY = 3; // 右向きアニメーション行
	}
	if (CheckHitKey(KEY_INPUT_A)) {
		position.x -= moveSpeed;
		// 左側のステージ当たり判定と押し戻し処理
		int push = st->CheckLeft(position + VECTOR2(-24, -31)); // 左上
		position.x += push;
		push = st->CheckLeft(position + VECTOR2(-24, 31)); // 左下
		position.x += push;
		//animTimer++;
		animY = 8; // 左向きアニメーション行
	}
	if (CheckHitKey(KEY_INPUT_F)) {
		// 未使用のキー入力
	}

	// アニメーションの更新
	if (animTimer % 10 == 0)
	{
		if (anim > 5)
		{
			anim = 0;
		}
		anim++;
	}

	// ジャンプ処理
	if (onGround) {
		if (Input::IsKeyUP(KEY_INPUT_W)) {
			PlaySoundMem(hSound, DX_PLAYTYPE_BACK);
			if (prevPushed == false) { // キーを押した瞬間かチェック
				velocityY = JumpV0;
			}
			prevPushed = true;
		}
		else {
			prevPushed = false;
		}
		//// 2割の確率で音を鳴らす処理（コメントアウトされている）
		//if (rand() % 10 < 2) { // ランダムに
		//	int soundChoice = rand() % 2; // 0または1をランダムに選ぶ
		//	if (soundChoice == 0) {
		//		PlaySound("1.mp3"); // 1.mp3を再生
		//	}
		//	else {
		//		PlaySound("2.mp3"); // 2.mp3を再生
		//	}
		//}
	}

	// 重力と上下のステージ当たり判定
	{
		position.y += velocityY;
		velocityY += Gravity; // 重力を適用
		onGround = false;
		if (velocityY < 0.0f) { // 上昇中 (頭上の当たり判定)
			int push = st->CheckUp(position + VECTOR2(-24, -31)); // 左上
			if (push > 0) {
				velocityY = 0.0f;
				position.y += push;
			}
			push = st->CheckUp(position + VECTOR2(24, -31)); // 右上
			if (push > 0) {
				velocityY = 0.0f;
				position.y += push;
			}
		}
		else { // 落下中 (足元の当たり判定)
			int push = st->CheckDown(position + VECTOR2(-24, 31 + 1)); // 左下
			if (push > 0) {
				velocityY = 0.0f;
				onGround = true; // 着地
				position.y -= push - 1; // 1は当たり判定の余裕分か
			}
			push = st->CheckDown(position + VECTOR2(24, 31 + 1)); // 右下
			if (push > 0) {
				velocityY = 0.0f;
				onGround = true; // 着地
				position.y -= push - 1;
			}
		}
	}

	// 左右にスクロールする処理
	float drawX = position.x - st->ScrollX(); // これが表示座標
	static const int RightLimit = 400; // 右側のスクロール開始位置
	static const int LeftLimit = 400;  // 左側のスクロール開始位置

	if (drawX > RightLimit) {
		// プレイヤーが右側の限界を超えたら、スクロール位置を更新
		st->SetScrollX(position.x - RightLimit);
	}
	else if (drawX < LeftLimit) {
		// プレイヤーが左側の限界を下回ったら、スクロール位置を更新
		st->SetScrollX(position.x - LeftLimit);
	}

	// プレイヤーが落ちたらリセット
	if (position.y > 800) {
		position = VECTOR2(500, 200);
		st->SetScrollX(0);
	}


	//auto enemies = FindGameObjects<Fish>(); // 敵 (Fish) との当たり判定（コメントアウトされている）
	//for (Fish* e : enemies)
	//{
	//	VECTOR2 ePos = e->GetPosition();
	//	VECTOR2 d = ePos - position;
	//	if (VSize(d) < 30) // プレイヤーと敵の距離が30未満なら
	//	{
	//		e->DestroyMe(); // 敵を破壊
	//	}
	//}

	// //ImGuiデバッグ情報の表示
	//ImGui::Begin("Player");
	//ImGui::Checkbox("onGround", &onGround);
	//ImGui::InputFloat("positionY", &position.y);
	//ImGui::End();
}

// --- 描画処理 ---
void Player::Draw()
{
	if (hImage > 0) {
		int x = position.x - imageSize.x / 2.0f;
		int y = position.y - imageSize.y / 2.0f;

		Stage* st = FindGameObject<Stage>();
		if (st != nullptr) {
			x -= st->ScrollX(); // スクロールを考慮して描画座標を調整
		}
		// アニメーションと向きを指定して矩形描画
		DrawRectGraph(x, y, anim * imageSize.x, animY * imageSize.y, imageSize.x, imageSize.y, hImage, TRUE);
	}
	//Object2D::Draw();
	//// デバッグ用の当たり判定ボックス描画
	//Stage* st = FindGameObject<Stage>();
	//float x = position.x - st->ScrollX();
	///*DrawBox(x - 24, position.y - 32, x + 24, position.y + 32,
	//	GetColor(255, 0, 0), FALSE);*/
}