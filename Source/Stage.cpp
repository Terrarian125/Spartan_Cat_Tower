#include "Stage.h"
#include "Ball2D.h"
#include "../Library/CsvReader.h"
#include "../Source/Screen.h"
#include <math.h>

// コンストラクタ：設定ファイルとマップファイルを読み込む
Stage::Stage(std::string configPath, std::string mapPath) {
    SetDrawOrder(100);
    scroll = VECTOR2(0, 0);
    startPos = VECTOR2(100, 100);

    LoadConfig(configPath);
    LoadMap(mapPath);
}

// TileConfig.csv を読み込んでタイルの「カタログ」を作成する
void Stage::LoadConfig(std::string path) {
    CsvReader csv(path);
    if (csv.GetLines() <= 0) return;

    // 1行目はヘッダーなので i=1 から開始
    for (int i = 1; i < csv.GetLines(); i++) {
        int id = csv.GetInt(i, 0);
        // 画像読み込み場所を Data/Image/ に指定
        std::string imgPath = "Data/Image/" + csv.GetString(i, 1);
        int anim = csv.GetInt(i, 2);
        std::string functionName = csv.GetString(i, 3);

        TileTypeData data;
        data.animCount = (anim <= 0) ? 1 : anim;
        data.func = functionName;

        // ハンドルを初期化
        for (int j = 0; j < 16; j++) data.handles[j] = -1;

        if (data.animCount <= 1) {
            data.handles[0] = LoadGraph(imgPath.c_str());
        }
        else {
            // 横並びのスプライトシートとして分割読み込み
            LoadDivGraph(imgPath.c_str(), data.animCount, data.animCount, 1, (int)TILE_SIZE, (int)TILE_SIZE, data.handles);
        }
        catalog[id] = data;
    }
}

// stageXX.csv を読み込んでマップ配置データを作成する
void Stage::LoadMap(std::string path) {
    CsvReader csv(path);
    mapData.clear();
    if (csv.GetLines() <= 0) return;

    for (int i = 0; i < csv.GetLines(); i++) {
        std::vector<int> row;
        for (int j = 0; j < csv.GetColumns(i); j++) {
            int val = csv.GetInt(i, j);
            row.push_back(val);
            // 01番をプレイヤーのスタート位置として記録
            if (val == 1) {
                startPos = VECTOR2(j * TILE_SIZE + TILE_SIZE / 2.0f, i * TILE_SIZE + TILE_SIZE / 2.0f);
            }
        }
        mapData.push_back(row);
    }
}

// 指定座標にあるタイルの「機能名」を返す（Ball2Dの判定で使用）
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
    // プレイヤーの位置に合わせてスクロール座標を更新
    Ball2D* player = FindGameObject<Ball2D>();
    if (player) {
        scroll.x = player->GetPosition().x - (Screen::WIDTH / 2.0f);
        scroll.y = player->GetPosition().y - (Screen::HEIGHT / 2.0f);
    }
}

void Stage::Draw() {
    // 150ミリ秒ごとにアニメーションのコマを進める
    int animIndex = (GetNowCount() / 150);

    for (int y = 0; y < (int)mapData.size(); y++) {
        for (int x = 0; x < (int)mapData[y].size(); x++) {
            int id = mapData[y][x];

            // 0(空白) または 1(スタート地点) は描画しない
            if (id <= 1) continue;

            int dx = (int)(x * TILE_SIZE - scroll.x);
            int dy = (int)(y * TILE_SIZE - scroll.y);

            // 画面外描画スキップ
            if (dx < -TILE_SIZE || dx > Screen::WIDTH || dy < -TILE_SIZE || dy > Screen::HEIGHT) continue;

            if (catalog.count(id)) {
                TileTypeData& t = catalog[id];
                int h = t.handles[animIndex % t.animCount];

                if (h != -1) {
                    // 画像の描画
                    DrawGraph(dx, dy, h, TRUE);
                }
                else {
                    // 画像がない場合の仮描画（枠とID番号）
                    DrawBox(dx, dy, dx + (int)TILE_SIZE, dy + (int)TILE_SIZE, GetColor(80, 80, 80), FALSE);
                    DrawFormatString(dx + 20, dy + 20, GetColor(150, 150, 150), "%02d", id);
                }
            }
        }
    }
}