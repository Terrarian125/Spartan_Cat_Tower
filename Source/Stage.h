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
    Stage(std::string configPath, std::string mapPath);
    virtual ~Stage() {}

    virtual void Update() override;
    virtual void Draw() override;

    //Ball2Dが使用する判定関数
    std::string GetTileFunction(float px, float py);

    //ゲッター関数
    VECTOR2 GetStartPosition() const { return startPos; }
    float ScrollX() const { return scroll.x; }
    float ScrollY() const { return scroll.y; }

    int GetMapWidth() { return mapData.empty() ? 0 : (int)mapData[0].size(); }
    int GetMapHeight() { return (int)mapData.size(); }

    void SaveMap(std::string path);

    //静的変数（クリア画面等で共有
    static std::string nextMapPath;
    static int lastUnlockedTileId;  //クリア画面でアンロックするタイルID
    static float lastPlayerSpeed;   //ステージごとのプレイヤー速度（拡張用）

    const float TILE_SIZE = 64.0f;

private:
    void LoadConfig(std::string path);
    void LoadMap(std::string path);

    std::map<int, TileTypeData> catalog;
    std::vector<std::vector<int>> mapData;
    VECTOR2 scroll;
    VECTOR2 startPos;
    int bgHandle = -1;

    std::string currentMapPath; //現在のマップパス
    std::string currentBgPath;  //背景画像パス
    int unlockTileId = -1;      //このステージで解放されるタイルID
};