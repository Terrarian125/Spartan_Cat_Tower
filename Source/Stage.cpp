#include "Stage.h"
#include "Ball2D.h"
#include "../Library/CsvReader.h"
#include "../Source/Screen.h"
#include <math.h>
#include <fstream>
#include <iostream>

//初期値（StageSelectScene等から書き換えられる）
std::string Stage::nextMapPath = "Data/Stage/stage01.csv";

Stage::Stage(std::string configPath, std::string mapPath) {
    SetDrawOrder(100);
    scroll = VECTOR2(0, 0);
    startPos = VECTOR2(100, 100);

    LoadConfig(configPath);
    LoadMap(mapPath); //既存通りのパスをそのまま渡す
}

//設定(.txt)とマップ(.csv)をそれぞれ別ファイルとして保存する
void Stage::SaveMap(std::string path) {
    //パスから ".csv" 拡張子を取り除いたベースパスを作成
    std::string basePath = path;
    size_t extPos = basePath.rfind(".csv");
    if (extPos != std::string::npos) {
        basePath = basePath.substr(0, extPos);
    }

    //設定ファイル (.txt) の保存
    std::ofstream txtOfs(basePath + ".txt");
    if (txtOfs) {
        txtOfs << "BG=" << currentBgPath << std::endl;
        txtOfs.close();
    }

    //マップデータ (.csv) の保存（純粋な数値データのみ）
    std::ofstream csvOfs(basePath + ".csv");
    if (!csvOfs) return;

    for (const auto& row : mapData) {
        for (int i = 0; i < (int)row.size(); i++) {
            csvOfs << row[i] << (i < (int)row.size() - 1 ? "," : "");
        }
        csvOfs << std::endl;
    }
    csvOfs.close();
}

//TileConfig.csv の読込
void Stage::LoadConfig(std::string path) {
    CsvReader csv(path);
    if (csv.GetLines() <= 0) return;

    for (int i = 1; i < csv.GetLines(); i++) {
        int id = csv.GetInt(i, 0);
        std::string imgPath = "Data/Image/" + csv.GetString(i, 1);
        int anim = csv.GetInt(i, 2);
        std::string functionName = csv.GetString(i, 3);

        TileTypeData data;
        data.animCount = (anim <= 0) ? 1 : anim;
        data.func = functionName;

        for (int j = 0; j < 16; j++) data.handles[j] = -1;

        if (data.animCount <= 1) {
            data.handles[0] = LoadGraph(imgPath.c_str());
        }
        else {
            LoadDivGraph(imgPath.c_str(), data.animCount, data.animCount, 1, (int)TILE_SIZE, (int)TILE_SIZE, data.handles);
        }
        catalog[id] = data;
    }
}

//.txt と .csv から背景とマップデータを綺麗に分離して読み込む
void Stage::LoadMap(std::string path) {
    currentMapPath = path;
    mapData.clear();

    //パスから ".csv" 拡張子を取り除いたベースパスを作成
    std::string basePath = path;
    size_t extPos = basePath.rfind(".csv");
    if (extPos != std::string::npos) {
        basePath = basePath.substr(0, extPos);
    }

    //同名の設定ファイル (.txt) から背景情報を読み込む
    std::string txtPath = basePath + ".txt";
    std::ifstream ifs(txtPath);
    if (ifs) {
        std::string line;
        while (std::getline(ifs, line)) {
            auto pos = line.find('=');
            if (pos == std::string::npos) continue;

            std::string key = line.substr(0, pos);
            std::string val = line.substr(pos + 1);

            if (key == "BG") {
                currentBgPath = val;
                std::string bgPath = "Data/Image/" + currentBgPath;
                bgHandle = LoadGraph(bgPath.c_str());
            }
        }
        ifs.close();
    }

    //マップ配置データの読み込み
    std::string csvPath = basePath + ".csv";
    CsvReader csv(csvPath);
    if (csv.GetLines() <= 0) return;

    //1行目から純粋な数値データとして処理
    for (int i = 0; i < csv.GetLines(); i++) {
        std::vector<int> row;
        for (int j = 0; j < csv.GetColumns(i); j++) {
            int val = csv.GetInt(i, j);
            row.push_back(val);

            //プレイヤーのスタート位置
            if (val == 1) {
                startPos = VECTOR2(j * TILE_SIZE + TILE_SIZE / 2.0f, i * TILE_SIZE + TILE_SIZE / 2.0f);
            }
        }
        mapData.push_back(row);
    }
}

//指定座標にあるタイルの「機能名」を返す
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
    Ball2D* player = FindGameObject<Ball2D>();
    if (player) {
        float targetX = player->GetPosition().x - (Screen::WIDTH / 2.0f);
        float targetY = player->GetPosition().y - (Screen::HEIGHT / 2.0f);

        float lerpSpeed = 0.1f;
        scroll.x += (targetX - scroll.x) * lerpSpeed;
        scroll.y += (targetY - scroll.y) * lerpSpeed;
    }
}

void Stage::Draw() {
    if (bgHandle != -1) {
        DrawGraph(0, 0, bgHandle, FALSE);
    }

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