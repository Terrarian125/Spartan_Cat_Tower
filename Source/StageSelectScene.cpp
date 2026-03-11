#include "StageSelectScene.h"
#include "../Library/Input.h"
#include "../Library/SceneManager.h"
#include "../Library/ObjectManager.h"
#include "Stage.h"
#include "../Library/GuiButton.h"

StageSelectScene::StageSelectScene() {
    // 背景とフォントのロード
    bgHandle = LoadGraph("Data/Image/bg_select.png");
    fontHandle = CreateFontToHandle("メイリオ", 32, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);

    // ボタンの配置設定
    int bx = 200;  // ボタンのX座標
    int by = 200;  // ボタンの開始Y座標
    int bw = 800;  // ボタンの横幅
    int bh = 120;  // ボタンの高さ
    int bi = 150;  // ボタンの間隔

    // ステージ情報の定義
    struct StageData {
        std::string name;
        std::string file;
        std::string movie; // 動画パスを追加
    };
    std::vector<StageData> stages = {
        { "STAGE 1: ", "Data/Stage/stage01.csv", "Data/Movie/stage01.mp4" },
        { "STAGE 2: ", "Data/Stage/stage02.csv", "Data/Movie/stage02.mp4" },
        { "STAGE 3: ", "Data/Stage/stage03.csv", "Data/Movie/stage03.mp4" }
    };

    // 各ステージに対応するボタンを生成してリストに
    for (int i = 0; i < (int)stages.size(); i++) {
        auto btn = new GuiButton(bx, by + (i * bi), bw, bh, stages[i].name);
        buttons.push_back(btn);

        // 動画のロード
        int movieHandle = LoadGraph(stages[i].movie.c_str());
        if (movieHandle != -1) {
            btn->SetImage(movieHandle);
            btn->SetIsMovie(true); // GuiButton側で動画として扱うフラグ


            PlayMovieToGraph(movieHandle, DX_PLAYTYPE_LOOP);//ループ再生
            PauseMovieToGraph(movieHandle);  // 最初は停止させておく

        }

        // 読み込むべきステージパスを変数に保持
        std::string path = stages[i].file;

        // ボタンがクリックされた時の処理
        btn->onClick = [path]() {
            Stage::nextMapPath = path;
            SceneManager::ChangeScene("PLAY");
            };

        // ボタンをシーンのオブジェクトとして追加
        ObjectManager::Push(btn);
    }
}

StageSelectScene::~StageSelectScene() {
    if (bgHandle != -1) {
        DeleteGraph(bgHandle);
        bgHandle = -1;
    }
    if (fontHandle != -1) {
        DeleteFontToHandle(fontHandle);
        fontHandle = -1;
    }

    for (auto btn : buttons) {
        if (btn != nullptr) {
        }
    }
    buttons.clear();
    ObjectManager::DeleteAllGameObject();
}

void StageSelectScene::Update() {
    // シーン全体に関わる処理
    if (Input::IsKeyDown(KEY_INPUT_ESCAPE)) {
        SceneManager::ChangeScene("TITLE");
    }
}

void StageSelectScene::Draw() {
    // 背景の描画
    if (bgHandle != -1) DrawGraph(0, 0, bgHandle, FALSE);

    // タイトルの描画
    DrawStringToHandle(100, 50, "ステージを選択してください", GetColor(255, 255, 255), fontHandle);
}