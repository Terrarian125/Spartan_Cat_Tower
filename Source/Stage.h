#pragma once
#include "../Library/GameObject.h"
#include "../Library/Object2D.h"
#include <vector>
#include <string>
#include <map>

struct TileTypeData {
    int handles[16];
    int animCount = 0;
    std::string func;
};

class Stage : public GameObject {
public:
    // 引数を2つに変更
    Stage(std::string configPath, std::string mapPath);
    virtual void Update() override;
    virtual void Draw() override;

    // Ball2Dが使う新しい判定関数
    std::string GetTileFunction(float px, float py);

    // PlaySceneが使う関数
    VECTOR2 GetStartPosition() const { return startPos; }
    float ScrollX() const { return scroll.x; }
    float ScrollY() const { return scroll.y; }

    const float TILE_SIZE = 64.0f;

private:
    void LoadConfig(std::string path);
    void LoadMap(std::string path); // 構造をシンプルに修正

    std::map<int, TileTypeData> catalog;
    std::vector<std::vector<int>> mapData;
    VECTOR2 scroll;
    VECTOR2 startPos;
};