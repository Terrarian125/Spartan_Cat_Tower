#include "PlayScene.h"
#include "Stage.h"
#include "Ball2D.h"
#include "../Library/Input.h"

PlayScene::PlayScene() { Init(); }
PlayScene::~PlayScene() {}

void PlayScene::Init() {
    Stage* stage = new Stage("Data/Stage/TileConfig.csv", Stage::nextMapPath);
    ObjectManager::Push(stage);

    Ball2D* player = new Ball2D(GetColor(255, 200, 0), true);
    Ball2D* partner = new Ball2D(GetColor(100, 200, 255), false);

    VECTOR2 start = stage->GetStartPosition();
    player->SetPosition(start);
    partner->SetPosition(VECTOR2(start.x + 100, start.y));
    player->SetPartner(partner);
}

void PlayScene::Update() {
    if (Input::IsKeyDown(KEY_INPUT_ESCAPE)) {
		SceneManager::ChangeScene("STAGE");
        return;
    }
}
void PlayScene::Draw() {
    //DrawString(10, 10, "A D: Move / Space: Jump", GetColor(255, 255, 255));
}