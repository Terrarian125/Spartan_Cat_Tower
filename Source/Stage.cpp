#include "Stage.h"
#include "Ball2D.h"
#include "../Library/CsvReader.h"
#include "../Source/Screen.h"
#include <math.h>
#include <fstream>
#include <iostream>

std::string Stage::nextMapPath = "Data/Stage/stage01.csv";

//設定ファイルとマップファイルを読み込む
Stage::Stage(std::string configPath, std::string mapPath) {
    SetDrawOrder(100);
    scroll = VECTOR2(0, 0);
    startPos = VECTOR2(100, 100);

    LoadConfig(configPath);
    LoadMap(mapPath);
}

void Stage::SaveMap(std::string path) {
    std::ofstream ofs(path);
    if (!ofs) return;

    //読み込み時と同じ背景設定を1行目に書き出す
    ofs << "\"" << currentBgPath << "\",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,," << std::endl;

    for (const auto& row : mapData) {
        for (int i = 0; i < (int)row.size(); i++) {
            ofs << row[i] << (i < (int)row.size() - 1 ? "," : "");
        }
        ofs << std::endl;
    }
    ofs.close();
}

//TileConfig.csv を読込
void Stage::LoadConfig(std::string path) {
    CsvReader csv(path);
    if (csv.GetLines() <= 0) return;

    //1行目はヘッダーなので i=1 から開始
    for (int i = 1; i < csv.GetLines(); i++) {
        int id = csv.GetInt(i, 0);
        //画像読み込み場所を Data/Image/ に指定
        std::string imgPath = "Data/Image/" + csv.GetString(i, 1);
        int anim = csv.GetInt(i, 2);
        std::string functionName = csv.GetString(i, 3);

        TileTypeData data;
        data.animCount = (anim <= 0) ? 1 : anim;
        data.func = functionName;

        //ハンドルを初期化
        for (int j = 0; j < 16; j++) data.handles[j] = -1;

        if (data.animCount <= 1) {
            data.handles[0] = LoadGraph(imgPath.c_str());
        }
        else {
            //横並びのスプライトシートとして分割読み込み
            LoadDivGraph(imgPath.c_str(), data.animCount, data.animCount, 1, (int)TILE_SIZE, (int)TILE_SIZE, data.handles);
        }
        catalog[id] = data;
    }
}

//stageXX.csv を読み込んでマップ配置データを作成する
void Stage::LoadMap(std::string path) {
    currentMapPath = path; //パスを記憶
    CsvReader csv(path);
    if (csv.GetLines() <= 0) return;

    //1行目から背景情報を抽出して記憶しておく
    std::string firstCol = csv.GetString(0, 0); //"BG,bg_stage02.png"
    currentBgPath = firstCol;

    int startLine = 0;

    //1行目が背景設定（BG）かどうかを判定
    if (csv.GetString(0, 0) == "BG") {
        std::string bgName = csv.GetString(0, 1);
        std::string bgPath = "Data/Image/" + bgName;
        bgHandle = LoadGraph(bgPath.c_str());
        //2行目からがマップデータなので開始行をずらす
        startLine = 1;
    }

    for (int i = startLine; i < csv.GetLines(); i++) {
        std::vector<int> row;
        for (int j = 0; j < csv.GetColumns(i); j++) {
            int val = csv.GetInt(i, j);
            row.push_back(val);

            //プレイヤーのスタート位置（マップデータの行番号に合わせるため i - startLine）
            if (val == 1) {
                startPos = VECTOR2(j * TILE_SIZE + TILE_SIZE / 2.0f, (i - startLine) * TILE_SIZE + TILE_SIZE / 2.0f);
            }
        }
        mapData.push_back(row);
    }
}

//指定座標にあるタイルの「機能名」を返す（Ball2Dの判定で使用）
std::string Stage::GetTileFunction(float px, float py) {
    int tx = (int)(px / TILE_SIZE);
    int ty = (int)(py / TILE_SIZE);
    if (ty >= 0 && ty < (int)mapData.size() && tx >= 0 && tx < (int)mapData[ty].size()) {
        int id = mapData[ty][tx];
        if (catalog.count(id)) {
            return catalog[id].func;
        }
    }
    return "NONE";
}

void Stage::Update() {
    //プレイヤーの位置に合わせてスクロール座標を更新
    Ball2D* player = FindGameObject<Ball2D>();
    if (player) {
        scroll.x = player->GetPosition().x - (Screen::WIDTH / 2.0f);
        scroll.y = player->GetPosition().y - (Screen::HEIGHT / 2.0f);
    }
}

void Stage::Draw() {
    //最初に背景を描画（一番奥）
    if (bgHandle != -1) {
        //画面全体に描画する場合（スクロールさせない固定背景）
        DrawGraph(0, 0, bgHandle, FALSE);

        //もし背景もスクロールさせたい場合は以下
        //DrawGraph((int)(-scroll.x * 0.5f), (int)(-scroll.y * 0.5f), bgHandle, FALSE);
    }

    //150ミリ秒ごとにアニメーションのコマを進める
    int animIndex = (GetNowCount() / 150);

    for (int y = 0; y < (int)mapData.size(); y++) {
        for (int x = 0; x < (int)mapData[y].size(); x++) {
            int id = mapData[y][x];

            if (id <= 1) continue;

            int dx = (int)(x * TILE_SIZE - scroll.x);
            int dy = (int)(y * TILE_SIZE - scroll.y);

            if (dx < -TILE_SIZE || dx > Screen::WIDTH || dy < -TILE_SIZE || dy > Screen::HEIGHT) continue;

            if (catalog.count(id)) {
                TileTypeData& t = catalog[id];
                int h = t.handles[animIndex % t.animCount];

                if (h != -1) {
                    DrawGraph(dx, dy, h, TRUE);
                }
                else {
                    DrawBox(dx, dy, dx + (int)TILE_SIZE, dy + (int)TILE_SIZE, GetColor(80, 80, 80), FALSE);
                    DrawFormatString(dx + 20, dy + 20, GetColor(150, 150, 150), "%02d", id);
                }
            }
        }
    }
}