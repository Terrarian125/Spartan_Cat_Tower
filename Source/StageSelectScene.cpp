#include "StageSelectScene.h"
#include "../Library/Input.h"
#include "../Library/SceneManager.h"
#include "../Library/ObjectManager.h"
#include "Stage.h"
#include "../Library/GuiButton.h"

StageSelectScene::StageSelectScene() {
    //背景とフォントのロード
    bgHandle = LoadGraph("Data/Image/bg_select.png");
    fontHandle = CreateFontToHandle("メイリオ", 32, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);

    //ボタンの配置設定
    int bx = 200;  //ボタンのX座標
    int by = 200;  //ボタンの開始Y座標
    int bw = 800;  //ボタンの横幅（大きめ）
    int bh = 120;  //ボタンの高さ
    int bi = 150;  //ボタンの間隔

    //ステージ情報の定義
    struct StageData {
        std::string name;
        std::string file;
    };
    std::vector<StageData> stages = {
        { "STAGE 1: ", "Data/Stage/stage01.csv" },
        { "STAGE 2: ", "Data/Stage/stage02.csv" },
        { "STAGE 3: ", "Data/Stage/stage03.csv" }
    };

    //各ステージに対応するボタンを生成
    for (int i = 0; i < (int)stages.size(); i++) {
        //ボタンの生成（GameObjectを継承しているのでSceneManager経由で描画されます）
        auto btn = new GuiButton(bx, by + (i * bi), bw, bh, stages[i].name);

        //読み込むべきステージパスを変数に保持
        std::string path = stages[i].file;

        //ボタンがクリックされた時の処理を登録
        btn->onClick = [path]() {
            //Stageクラスの静的変数にパスをセット
            Stage::nextMapPath = path;
            //プレイシーン（PLAY）へ遷移
            SceneManager::ChangeScene("PLAY");
            };

        //ボタンをシーンのオブジェクトとして追加
        ObjectManager::Push(btn);
    }
}

void StageSelectScene::Update() {
    //GuiButton側でクリック判定を処理するため、ここではシーン全体に関わる処理（戻る等）のみ記述
    if (Input::IsKeyDown(KEY_INPUT_ESCAPE)) {
        SceneManager::ChangeScene("TITLE");
    }
}

void StageSelectScene::Draw() {
    //背景の描画
    if (bgHandle != -1) DrawGraph(0, 0, bgHandle, FALSE);

    //タイトルの描画
    DrawStringToHandle(100, 50, "ステージを選択してください", GetColor(255, 255, 255), fontHandle);
}