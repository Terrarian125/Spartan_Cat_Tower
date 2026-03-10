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

class StageEditor;

class Stage : public GameObject {
    friend class StageEditor;
public:
    //コンストラクタの引数は既存の構成を維持
    Stage(std::string configPath, std::string mapPath);
    virtual ~Stage() {}

    virtual void Update() override;
    virtual void Draw() override;

    //Ball2Dが使用する判定関数
    std::string GetTileFunction(float px, float py);

    //各クラスが参照するゲッター関数を維持
    VECTOR2 GetStartPosition() const { return startPos; }
    float ScrollX() const { return scroll.x; }
    float ScrollY() const { return scroll.y; }

    //次に読み込むべきマップのパスを保持する静的変数
    static std::string nextMapPath;

    const float TILE_SIZE = 64.0f;

    int GetMapWidth() { return mapData.empty() ? 0 : (int)mapData[0].size(); }
    int GetMapHeight() { return (int)mapData.size(); }

    void SaveMap(std::string path);//保存関数
private:
    void LoadConfig(std::string path);
    void LoadMap(std::string path);

    std::map<int, TileTypeData> catalog;
    std::vector<std::vector<int>> mapData;
    VECTOR2 scroll;
    VECTOR2 startPos;
    int bgHandle = -1;

    std::string currentMapPath; //現在のマップパスを記憶
    std::string currentBgPath;  //読み込んだ背景画像パスを記憶
};