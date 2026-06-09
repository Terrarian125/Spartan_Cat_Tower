#include "StageSelectScene.h"
#include "../Library/Input.h"
#include "../Library/SceneManager.h"
#include "../Library/ObjectManager.h"
#include "Stage.h"
#include "../Library/GuiButton.h"
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

//スクロール位置と画面縦幅の定義
static int scrollY = 0;
static const int WINDOW_HEIGHT = 720; //画面解像度に合わせて調整してください

//Update側で長押し判定をするために、スクロールボタンのポインタを保持する変数
static GuiButton* btnScrollUp = nullptr;
static GuiButton* btnScrollDown = nullptr;

StageSelectScene::StageSelectScene() {
    //背景とUIタイトルのロード
    bgHandle = LoadGraph("Data/Image/bg_select.png");
    titleUiHandle = LoadGraph("Data/Image/UI/UI_StageSelect.png");

    //ボタンの配置設定
    int bx = 200;  // ボタンのX座標
    int by = 200;  // ボタンの開始Y座標
    int bw = 700;  // ボタンの横幅 (スクロールボタンと被らないように少し縮小)
    int bh = 120;  // ボタンの高さ
    int bi = 150;  // ボタンの間隔

    //ステージ情報の構造体
    struct StageData {
        std::string name;
        std::string file;
        std::string image;
    };
    std::vector<StageData> stages;

    //フォルダ内から "stage" で始まるCSVファイルを自動取得する処理
    std::string stageFolder = "Data/Stage";
    std::vector<std::string> csvFiles;

    if (fs::exists(stageFolder) && fs::is_directory(stageFolder)) {
        for (const auto& entry : fs::directory_iterator(stageFolder)) {
            if (entry.is_regular_file() && entry.path().extension() == ".csv") {
                //元のファイル名
                std::string stemName = entry.path().stem().string();

                // 小文字変換用の変数を作る
                std::string lowerStemName = stemName;
                // 大文字小文字のブレを無くすために、文字列全体を小文字に統一
                std::transform(lowerStemName.begin(), lowerStemName.end(), lowerStemName.begin(), [](unsigned char c) {
                    return std::tolower(c);
                    });

                //全て小文字になった状態で、先頭が "stage" から始まっているか判定
                if (lowerStemName.rfind("stage", 0) == 0) {
                    csvFiles.push_back(entry.path().string());
                }
            }
        }

        //大文字小文字が混ざっていても、内部で小文字化して正しくソートする
        std::sort(csvFiles.begin(), csvFiles.end(), [](const std::string& a, const std::string& b) {
            std::string lowerA = fs::path(a).stem().string();
            std::string lowerB = fs::path(b).stem().string();
            std::transform(lowerA.begin(), lowerA.end(), lowerA.begin(), [](unsigned char c) { return std::tolower(c); });
            std::transform(lowerB.begin(), lowerB.end(), lowerB.begin(), [](unsigned char c) { return std::tolower(c); });
            return lowerA < lowerB;
            });
    }

    //取得したCSVファイル一覧からステージデータを構築
    for (size_t i = 0; i < csvFiles.size(); i++) {
        fs::path csvPath(csvFiles[i]);
        std::string stemName = csvPath.stem().string(); // 元のファイル名（大文字小文字ママ）

        StageData data;
        //画面上の表示名 (例: "STAGE 1: stage01")
        data.name = "STAGE " + std::to_string(i + 1) + ": " + stemName;
        //CSVファイルのフルパス
        data.file = csvFiles[i];
        //Image/stage フォルダから、CSVと同じファイル名のPNG画像を探すパスを自動生成
        data.image = "Data/Image/stage/" + stemName + ".png";

        stages.push_back(data);
    }

    scrollY = 0; // スクロール初期化

    //各ステージに対応するボタンを生成してリストに格納
    for (int i = 0; i < (int)stages.size(); i++) {
        auto btn = new GuiButton(bx, by + (i * bi), bw, bh, stages[i].name);
        buttons.push_back(btn);

        //ステージ名に対応したPNG画像をロードしてボタンに適用
        int imgHandle = LoadGraph(stages[i].image.c_str());
        if (imgHandle != -1) {
            btn->SetImage(imgHandle);
            btn->SetIsMovie(false);
        }

        std::string path = stages[i].file;

        //ボタンがクリックされた時の処理
        btn->onClick = [path]() {
            Stage::nextMapPath = path;
            SceneManager::ChangeScene("PLAY");
            };

        //ボタンをオブジェクトマネージャーに登録
        ObjectManager::Push(btn);
    }

    //スクロール用の補助ボタン配置
    btnScrollUp = new GuiButton(930, 200, 60, 60, "▲");
    ObjectManager::Push(btnScrollUp);

    btnScrollDown = new GuiButton(930, 500, 60, 60, "▼");
    ObjectManager::Push(btnScrollDown);

    //戻るボタン
    auto bBack = new GuiButton(10, 620, 100, 60, "Back");
    bBack->onClick = [this]() { SceneManager::ChangeScene("TITLE"); };
    ObjectManager::Push(bBack);
}

StageSelectScene::~StageSelectScene() {
    if (bgHandle != -1) {
        DeleteGraph(bgHandle);
        bgHandle = -1;
    }
    if (titleUiHandle != -1) {
        DeleteGraph(titleUiHandle); //タイトルUI画像のメモリ解放
        titleUiHandle = -1;
    }

    buttons.clear();
    btnScrollUp = nullptr;
    btnScrollDown = nullptr;
    ObjectManager::DeleteAllGameObject();
}

void StageSelectScene::Update() {
    //ESCキーでタイトルへ戻る
    if (Input::IsKeyDown(KEY_INPUT_ESCAPE)) {
        SceneManager::ChangeScene("TITLE");
        return;
    }

    //マウスホイールによるスクロール処理
    int wheel = Input::GetMouseWheel();
    if (wheel != 0) {
        scrollY -= wheel * 30; // 30はスクロールの速度
    }

    //スクロールボタン長押しによる滑らかなスクロール処理
    if (Input::IsKeepMouseDown(MOUSE_INPUT_LEFT) > 0) {

        //▲ボタンの上にマウスがあって、左クリック長押し中なら上に移動
        if (btnScrollUp != nullptr && btnScrollUp->IsMouseOver()) {
            scrollY -= 8; //長押しスクロール速度
        }

        //▼ボタンの上にマウスがあって、左クリック長押し中なら下に移動
        if (btnScrollDown != nullptr && btnScrollDown->IsMouseOver()) {
            scrollY += 8;
        }
    }

    //スクロール制限の計算
    int buttonStartY = 200; //最初のボタンの初期位置Y
    int buttonInterval = 150; //ボタンの間隔
    int totalHeight = (int)buttons.size() * buttonInterval; //ステージボタンの合計の高さ

    //画面内に収まる範囲を引いた、スクロール可能な最大限界値
    int maxScrollY = totalHeight - (WINDOW_HEIGHT - buttonStartY - 100);

    if (maxScrollY < 0) maxScrollY = 0; //ステージ数が少なくて画面に収まる場合はスクロール上限を0にする
    if (scrollY < 0) scrollY = 0;       //上に行き過ぎないガード
    if (scrollY > maxScrollY) scrollY = maxScrollY; //下に行き過ぎないガード

    //全てのステージボタンの座標をスクロール位置に合わせて同期更新
    for (int i = 0; i < (int)buttons.size(); i++) {
        if (buttons[i] != nullptr) {
            int originalY = buttonStartY + (i * buttonInterval);
            buttons[i]->y = originalY - scrollY;
        }
    }
}

void StageSelectScene::Draw() {
    //背景の描画
    if (bgHandle != -1) DrawGraph(0, 0, bgHandle, FALSE);

    //タイトルUI画像の描画 (透過処理を TRUE に設定)
    if (titleUiHandle != -1) {
        DrawGraph(100, 50, titleUiHandle, TRUE);
    }
}