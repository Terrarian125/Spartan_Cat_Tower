// StageBuilderScene.h
#pragma once
#include "../Library/SceneManager.h" 
#include "../Library/GameObject.h"
#include "../Library/GuiButton.h"
#include "../Library/SceneBase.h"

#include "BuilderToolBase.h"
#include "BrushTool.h"
#include "EraserTool.h"
#include "SelectTool.h"
#include "FillTool.h"
#include "MoveTool.h"
#include "CopyTool.h"
#include "PasteTool.h"
#include "UndoRedoManager.h"

#include <vector>
#include <string>
#include <memory>

struct BuilderTile {
    int id = 0;
    int imageHandle = -1;
    std::string function;
    int animCount = 1;
};

class StageBuilderScene : public SceneBase {
private:
	//グリッドの定義
    static const int GRID_COLS = 200;
    static const int GRID_ROWS = 200;
    static const int TILE_SIZE = 64;
    int gridStartX = 200;
    int gridStartY = 80;

    //カメラとズーム用の変数
    int cameraX = 0;
    int cameraY = (GRID_ROWS - 22) * TILE_SIZE;//初期画面でグリッドの下側が見えるように
    float scale = 1.0f;

    std::vector<std::vector<int>> gridData;
    std::vector<BuilderTile> availableTiles;
    int selectedTileId = 2;

    std::vector<GuiButton*> toolbarButtons;
    std::vector<GuiButton*> paletteButtons;

    //スクロールボタン用の追加
    std::vector<GuiButton*> scrollButtons;
    GuiButton* btnToggleScroll = nullptr; //トグルボタン保持用
    bool showScrollButtons = true;        //現在の表示状態フラグ

    EditMode currentMode = EditMode::BRUSH;
    std::unique_ptr<BrushTool> brushTool;
    std::unique_ptr<EraserTool> eraserTool;
    std::unique_ptr<SelectTool> selectTool;
    std::unique_ptr<FillTool> fillTool;
    std::unique_ptr<MoveTool> moveTool;
    std::unique_ptr<CopyTool> copyTool;
    std::unique_ptr<PasteTool> pasteTool;
    std::unique_ptr<UndoRedoManager> undoRedo;

    void LoadTileConfig();
    void SaveStage(const std::string& filepath);
    void LoadStage(const std::string& filepath);

public:
    StageBuilderScene();
    ~StageBuilderScene() override;
    void Update() override;
    void Draw() override;
};