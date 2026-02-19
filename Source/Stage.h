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
    // コンストラクタの引数は既存の構成を維持します
    Stage(std::string configPath, std::string mapPath);
    virtual ~Stage() {}

    virtual void Update() override;
    virtual void Draw() override;

    // Ball2Dが使用する判定関数です
    std::string GetTileFunction(float px, float py);

    // 各クラスが参照するゲッター関数を維持します
    VECTOR2 GetStartPosition() const { return startPos; }
    float ScrollX() const { return scroll.x; }
    float ScrollY() const { return scroll.y; }

    // 次に読み込むべきマップのパスを保持する静的変数です
    static std::string nextMapPath;

    const float TILE_SIZE = 64.0f;
private:
    void LoadConfig(std::string path);
    void LoadMap(std::string path);

    std::map<int, TileTypeData> catalog;
    std::vector<std::vector<int>> mapData;
    VECTOR2 scroll;
    VECTOR2 startPos;
    int bgHandle = -1;
};