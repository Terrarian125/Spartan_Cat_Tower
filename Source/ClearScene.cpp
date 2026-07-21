#include "ClearScene.h"
#include "../Library/Input.h"
#include "../Library/CsvReader.h"
#include "Ball2D.h"
#include "Stage.h"
#include <fstream>

ClearScene::ClearScene()
{
    alpha = 0.0f;
    fadeSpeed = 0.01f;
    hSound = LoadSoundMem("Data/Sound/ClearSound.wav");
    PlaySoundMem(hSound, DX_PLAYTYPE_BACK);

    //Data/keifont.ttf の読み込み
    AddFontResourceEx("Data/keifont.ttf", FR_PRIVATE, NULL);
    fontHandle = CreateFontToHandle("けいふぉんと", 28, 3, DX_FONTTYPE_ANTIALIASING);

    //解禁対象パーツの一括アンロック処理
    if (!Stage::lastUnlockedTileIds.empty()) {
        UnlockTilesInConfig(Stage::lastUnlockedTileIds);
    }

    //ステージ選択ボタンの設定
    int bx = 50, by = 50, bw = 100, bh = 100;
    int btnImg_StBack = LoadGraph("data/image/btnImg_StBack.png");

    auto bStBack = new GuiButton(bx, by, bw, bh, "ステージ選択へ");
    bStBack->SetImage(btnImg_StBack);
    bStBack->onClick = []() { SceneManager::ChangeScene("STAGE"); };
    buttons.push_back(bStBack);
}

ClearScene::~ClearScene()
{
    if (fontHandle != -1) DeleteFontToHandle(fontHandle);
    RemoveFontResourceEx("Data/keifont.ttf", FR_PRIVATE, NULL);

    for (auto& item : newUnlocks) {
        if (item.imgHandle != -1) DeleteGraph(item.imgHandle);
    }
}

//TileConfig.csv の対象IDのアンロックフラグを 1 に更新
void ClearScene::UnlockTilesInConfig(const std::vector<int>& targetIds)
{
    CsvReader csv("Data/Stage/TileConfig.csv");
    if (csv.GetLines() <= 0) return;

    std::vector<std::string> lines;
    lines.push_back("id,image,anim,function,description,unlocked");

    for (int i = 1; i < csv.GetLines(); i++) {
        if (csv.GetColumns(i) < 4) continue;

        int id = csv.GetInt(i, 0);
        std::string img = csv.GetString(i, 1);
        int anim = csv.GetInt(i, 2);
        std::string func = csv.GetString(i, 3);
        std::string desc = (csv.GetColumns(i) >= 5) ? csv.GetString(i, 4) : "";
        int unlocked = (csv.GetColumns(i) >= 6) ? csv.GetInt(i, 5) : 1;

        //targetIds リストに自分が含まれているか確認
        for (int targetId : targetIds) {
            if (id == targetId) {
                unlocked = 1; //フラグをオンにする

                UnlockedItemInfo info;
                info.desc = desc;
                info.imgHandle = LoadGraph(("Data/Image/" + img).c_str());
                newUnlocks.push_back(info);
                break;
            }
        }

        std::string line = std::to_string(id) + "," + img + "," + std::to_string(anim) + "," +
            func + ",\"" + desc + "\"," + std::to_string(unlocked);
        lines.push_back(line);
    }

    std::ofstream ofs("Data/Stage/TileConfig.csv");
    for (const auto& l : lines) {
        ofs << l << std::endl;
    }
    ofs.close();
}

void ClearScene::Update()
{
    if (Input::IsKeyDown(KEY_INPUT_ESCAPE)) {
        SceneManager::ChangeScene("STAGE");
        return;
    }
}

void ClearScene::Draw()
{
    int dmg = Ball2D::lastTotalDamage;
    int colWhite = GetColor(255, 255, 255);
    int colGold = GetColor(255, 215, 0);

    DrawStringToHandle(400, 60, "STAGE CLEAR!", colGold, fontHandle);
    DrawFormatStringToHandle(400, 110, colWhite, fontHandle, "受けたダメージ: %d 回", dmg);

    if (dmg == 0) {
        DrawStringToHandle(400, 160, "評価：SSS", colGold, fontHandle);
    }
    else if (dmg < 5) {
        DrawStringToHandle(400, 160, "評価：A", colWhite, fontHandle);
    }
    else {
        DrawStringToHandle(400, 160, "評価：C", colWhite, fontHandle);
    }

    //アンロックパーツ描画（複数対応
    if (!newUnlocks.empty()) {
        int baseY = 220;
        int itemHeight = 70;
        int boxHeight = baseY + 60 + (int)newUnlocks.size() * itemHeight;

        //背景枠描画
        DrawBox(280, baseY, 1000, boxHeight, GetColor(30, 30, 45), TRUE);
        DrawBox(280, baseY, 1000, boxHeight, colGold, FALSE);

        DrawStringToHandle(310, baseY + 15, "★ PARTS UNLOCKED! ★", colGold, fontHandle);

        //パーツリストを縦に並べて描画
        for (size_t i = 0; i < newUnlocks.size(); i++) {
            int drawY = baseY + 60 + (int)i * itemHeight;

            if (newUnlocks[i].imgHandle != -1) {
                DrawExtendGraph(320, drawY, 320 + 56, drawY + 56, newUnlocks[i].imgHandle, TRUE);
            }
            if (!newUnlocks[i].desc.empty()) {
                DrawStringToHandle(400, drawY + 12, newUnlocks[i].desc.c_str(), colWhite, fontHandle);
            }
        }
    }
}