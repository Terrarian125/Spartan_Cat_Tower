#pragma once
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include "../Library/SceneBase.h"
#include "../Library/GuiButton.h"

#include "BuilderToolBase.h"
#include "BrushTool.h"
#include "EraserTool.h"
#include "SelectTool.h"
#include "FillTool.h"
#include "MoveTool.h"
#include "CopyTool.h"
#include "PasteTool.h"
#include "UndoRedoManager.h"

struct BuilderTile {
    int id = 0;
    int imageHandle = -1;
    int animCount = 1;
    std::string function = "";
};

class StageBuilderScene : public SceneBase {
private:
    static const int GRID_COLS = 200;
    static const int GRID_ROWS = 100;
    static const int TILE_SIZE = 32;

    int gridStartX = 200;
    int gridStartY = 80;

    std::vector<std::vector<int>> gridData;
    std::vector<BuilderTile> availableTiles;
    int selectedTileId = 1;

    EditMode currentMode = EditMode::BRUSH;

    std::unique_ptr<BrushTool> brushTool;
    std::unique_ptr<EraserTool> eraserTool;
    std::unique_ptr<SelectTool> selectTool;
    std::unique_ptr<FillTool> fillTool;
    std::unique_ptr<MoveTool> moveTool;
    std::unique_ptr<CopyTool> copyTool;
    std::unique_ptr<PasteTool> pasteTool;
    std::unique_ptr<UndoRedoManager> undoRedo;

    int cameraX = 0;
    int cameraY = 0;
    float scale = 1.0f;

    std::vector<GuiButton*> toolbarButtons;
    std::vector<GuiButton*> paletteButtons;
    std::vector<GuiButton*> scrollButtons;
    GuiButton* btnToggleScroll = nullptr;
    bool showScrollButtons = true;

    int paletteScrollY = 0;

    // 設定ダイアログ用
    bool isSettingOpen = false;
    std::string currentBgPath = "Stage/bg_default.png";
    int bgHandle = -1;
    std::vector<int> unlockTileIds;
    int settingScrollY = 0;

    void LoadTileConfig();
    void SaveStage(const std::string& filepath);
    void LoadStage(const std::string& filepath);
    std::string OpenImageFileDialog();

    // UIの上にマウスがあるかを判定（入力貫通防止用）
    bool IsMouseOverUI() const;

public:
    StageBuilderScene();
    virtual ~StageBuilderScene() override;

    virtual void Update() override;
    virtual void Draw() override;
};