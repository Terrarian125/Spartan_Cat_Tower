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

    //1. Data/keifont.ttf の読み込み（フォント名: "けいふぉんと", サイズ: 28px）
    AddFontResourceEx("Data/keifont.ttf", FR_PRIVATE, NULL);
    fontHandle = CreateFontToHandle("けいふぉんと", 28, 3, DX_FONTTYPE_ANTIALIASING);

    //2. このステージで解禁されるパーツがあればアンロック処理
    int unlockId = Stage::lastUnlockedTileId;
    if (unlockId > 0) {
        UnlockTileInConfig(unlockId);
    }

    //ステージ選択画面へ戻るボタンの設定
    int bx = 50, by = 50, bw = 100, bh = 100;
    int btnImg_StBack = LoadGraph("data/image/btnImg_StBack.png");

    auto bStBack = new GuiButton(bx, by, bw, bh, "ステージ選択へ");
    bStBack->SetImage(btnImg_StBack);
    bStBack->onClick = []() { SceneManager::ChangeScene("STAGE"); };
    buttons.push_back(bStBack);
}

ClearScene::~ClearScene()
{
    //フォントリソースの解放
    if (fontHandle != -1) DeleteFontToHandle(fontHandle);
    RemoveFontResourceEx("Data/keifont.ttf", FR_PRIVATE, NULL);

    if (unlockedTileImg != -1) DeleteGraph(unlockedTileImg);
}

//TileConfig.csv のアンロックフラグ（6列目）を 1 に書き換え＆演出用データの読み込み
void ClearScene::UnlockTileInConfig(int targetId)
{
    CsvReader csv("Data/Stage/TileConfig.csv");
    if (csv.GetLines() <= 0) return;

    std::vector<std::string> lines;
    lines.push_back("id,image,anim,function,description,unlocked");

    for (int i = 1; i < csv.GetLines(); i++) {
        // 列数が足りない壊れた行はスキップ
        if (csv.GetColumns(i) < 4) continue;

        int id = csv.GetInt(i, 0);
        std::string img = csv.GetString(i, 1);
        int anim = csv.GetInt(i, 2);
        std::string func = csv.GetString(i, 3);

        // 5列目（description）、6列目（unlocked）が存在するかチェック
        std::string desc = (csv.GetColumns(i) >= 5) ? csv.GetString(i, 4) : "";
        int unlocked = (csv.GetColumns(i) >= 6) ? csv.GetInt(i, 5) : 1;

        if (id == targetId) {
            unlocked = 1;
            hasNewUnlock = true;
            unlockedTileDesc = desc;
            unlockedTileImg = LoadGraph(("Data/Image/" + img).c_str());
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

    //スコア＆クリアテキスト（けいふぉんと適用）
    DrawStringToHandle(400, 80, "STAGE CLEAR!", colGold, fontHandle);
    DrawFormatStringToHandle(400, 130, colWhite, fontHandle, "受けたダメージ: %d 回", dmg);

    if (dmg == 0) {
        DrawStringToHandle(400, 180, "評価：SSS", colGold, fontHandle);
    }
    else if (dmg < 5) {
        DrawStringToHandle(400, 180, "評価：A", colWhite, fontHandle);
    }
    else {
        DrawStringToHandle(400, 180, "評価：C", colWhite, fontHandle);
    }

    //新パーツ解放時のウィンドウ演出
    if (hasNewUnlock) {
        //背景枠（ゴールドの縁取り付きウィンドウ）
        DrawBox(300, 240, 980, 500, GetColor(30, 30, 45), TRUE);
        DrawBox(300, 240, 980, 500, colGold, FALSE);

        DrawStringToHandle(330, 260, "★ NEW PART UNLOCKED! ★", colGold, fontHandle);

        //パーツ画像の描画（拡大表示）
        if (unlockedTileImg != -1) {
            DrawExtendGraph(340, 320, 340 + 96, 320 + 96, unlockedTileImg, TRUE);
        }

        //説明文の描画
        if (!unlockedTileDesc.empty()) {
            DrawStringToHandle(460, 350, unlockedTileDesc.c_str(), colWhite, fontHandle);
        }
    }
}