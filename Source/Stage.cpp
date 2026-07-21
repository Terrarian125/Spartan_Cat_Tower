#include "Stage.h"
#include "Ball2D.h"
#include "../Library/CsvReader.h"
#include "../Source/Screen.h"
#include <math.h>
#include <fstream>
#include <iostream>
#include <sstream>

std::string Stage::nextMapPath = "Data/Stage/stage01.csv";
std::vector<int> Stage::lastUnlockedTileIds;

Stage::Stage(std::string configPath, std::string mapPath) {
    SetDrawOrder(100);
    scroll = VECTOR2(0, 0);
    startPos = VECTOR2(100, 100);

    LoadConfig(configPath);
    LoadMap(mapPath);
}

//マップ保存：1行目に CONFIG,画像名,アンロックID群 を書き出す
void Stage::SaveMap(std::string path) {
    std::ofstream ofs(path);
    if (!ofs) return;

    std::string bg = currentBgPath.empty() ? "bg_stage01.png" : currentBgPath;

    //アンロックIDリストをスペース区切り文字列にする
    std::string unlockStr = "";
    for (size_t i = 0; i < unlockTileIds.size(); i++) {
        unlockStr += std::to_string(unlockTileIds[i]) + (i < unlockTileIds.size() - 1 ? " " : "");
    }

    ofs << "CONFIG," << bg << "," << unlockStr << std::endl;

    //2行目以降にマップデータを書き出す
    for (const auto& row : mapData) {
        for (int i = 0; i < (int)row.size(); i++) {
            ofs << row[i] << (i < (int)row.size() - 1 ? "," : "");
        }
        ofs << std::endl;
    }
    ofs.close();
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

//マップ読み込み：1行目の CONFIG/BG を解析
void Stage::LoadMap(std::string path) {
    currentMapPath = path;
    mapData.clear();
    unlockTileIds.clear();

    CsvReader csv(path);
    if (csv.GetLines() <= 0) return;

    int startLine = 0;
    std::string firstTag = csv.GetString(0, 0);

    if (firstTag == "BG" || firstTag == "CONFIG") {
        currentBgPath = csv.GetString(0, 1);

        if (!currentBgPath.empty()) {
            std::string bgPath = "Data/Image/" + currentBgPath;
            bgHandle = LoadGraph(bgPath.c_str());
        }

        //3列目：スペース区切りのアンロックIDリストを取得（例: "5 8 12"）
        if (csv.GetColumns(0) >= 3) {
            std::string unlockStr = csv.GetString(0, 2);
            if (!unlockStr.empty()) {
                std::stringstream ss(unlockStr);
                int id;
                while (ss >> id) {
                    unlockTileIds.push_back(id);
                }
            }
        }

        Stage::lastUnlockedTileIds = unlockTileIds; //静的変数へ保持
        startLine = 1;
    }
    else {
        Stage::lastUnlockedTileIds.clear();
    }

    //マップ配置データの読み込み（安全ガード付き）
    for (int i = startLine; i < csv.GetLines(); i++) {
        std::vector<int> row;
        for (int j = 0; j < csv.GetColumns(i); j++) {
            std::string str = csv.GetString(i, j);

            int val = 0;
            if (!str.empty()) {
                try {
                    val = std::stoi(str);
                }
                catch (...) {
                    val = 0;
                }
            }

            row.push_back(val);

            if (val == 1) {
                startPos = VECTOR2(j * TILE_SIZE + TILE_SIZE / 2.0f, (i - startLine) * TILE_SIZE + TILE_SIZE / 2.0f);
            }
        }
        mapData.push_back(row);
    }
}

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