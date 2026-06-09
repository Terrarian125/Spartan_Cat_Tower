#pragma once
#include "../Library/SceneManager.h" 
#include "../Library/GameObject.h"
#include "../Library/GuiButton.h"
#include "../Library/SceneBase.h"
#include <vector>
#include <string>
#include <memory>
#include <stack>
#include <fstream>


//タイル情報を保持する構造体
struct BuilderTile {
    int id = 0;             //0は空
    int imageHandle = -1;
    std::string function;
};

//コマンドパターンの基底クラス
class BuilderCommand {
public:
    virtual ~BuilderCommand() = default;
    virtual void Undo(std::vector<std::vector<int>>& grid) = 0;
    virtual void Redo(std::vector<std::vector<int>>& grid) = 0;
};

class StageBuilderScene : public SceneBase {
private:
    //グリッド設定 (40x22マス、1マス32ピクセル)
    static const int GRID_COLS = 40;
    static const int GRID_ROWS = 22;
    static const int TILE_SIZE = 32;
    int gridStartX = 200; //左側のパレット領域を空ける
    int gridStartY = 80;  //上部のツールバー領域を空ける

    std::vector<std::vector<int>> gridData; //ステージの配置データ

    //パレット用の利用可能タイルリスト (Unlock == 1 のもの)
    std::vector<BuilderTile> availableTiles;
    int selectedTileId = 2; //現在ブラシで選択されているタイルID

    //エディターの現在のモード
    enum class EditMode { BRUSH, ERASER, SELECT, FILL, MOVE };
    EditMode currentMode = EditMode::BRUSH;

    //Undo / Redo スタック
    std::stack<std::shared_ptr<BuilderCommand>> undoStack;
    std::stack<std::shared_ptr<BuilderCommand>> redoStack;

    //UIボタン用リスト
    std::vector<GuiButton*> toolbarButtons;
    std::vector<GuiButton*> paletteButtons;

    //範囲選択用の座標
    int selectStartX = -1, selectStartY = -1;
    int selectEndX = -1, selectEndY = -1;
    bool isSelecting = false;

    //マウスドラッグ中の1手を記録するための一時バッファ
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> currentStrokeChanges;

public:
    StageBuilderScene();
    ~StageBuilderScene() override;
    void Update() override;
    void Draw() override;

private:
    void LoadTileConfig();
    void ExecuteCommand(std::shared_ptr<BuilderCommand> cmd);
    void PushCurrentStrokeAsCommand();
    void FloodFill(int startX, int startY, int targetId, int replacementId);
    void SaveToCSV(const std::string& filepath);
    void LoadFromCSV(const std::string& filepath);
    void CopyToClipboard();
    void PasteFromClipboard();
};