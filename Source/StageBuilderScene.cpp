//
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "StageBuilderScene.h"
#include <DxLib.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "../Library/Input.h"
#include "../Library/ObjectManager.h"

StageBuilderScene::StageBuilderScene() {
    gridData.resize(GRID_ROWS, std::vector<int>(GRID_COLS, 0));

    // 個別インスタンスの生成
    brushTool = std::make_unique<BrushTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    eraserTool = std::make_unique<EraserTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    selectTool = std::make_unique<SelectTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    fillTool = std::make_unique<FillTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    moveTool = std::make_unique<MoveTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    copyTool = std::make_unique<CopyTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    pasteTool = std::make_unique<PasteTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    undoRedo = std::make_unique<UndoRedoManager>();

    LoadTileConfig();

    std::vector<std::pair<std::string, std::function<void()>>> tools = {
        { "BRUSH",  [this]() { currentMode = EditMode::BRUSH; selectTool->Reset(); } },
        { "ERASER", [this]() { currentMode = EditMode::ERASER; selectTool->Reset(); } },
        { "SELECT", [this]() { currentMode = EditMode::SELECT; } },
        { "FILL",   [this]() { fillTool->Execute(gridData, selectedTileId, *selectTool, undoRedo->GetUndoStack(), undoRedo->GetRedoStack()); } },
        { "MOVE",   [this]() { currentMode = EditMode::MOVE; } },
        { "UNDO",   [this]() { undoRedo->Undo(gridData); }},
        { "REDO",   [this]() { undoRedo->Redo(gridData); }},
        { "SAVE",   [this]() { this->SaveStage("Data/Stage/stage_built.csv"); } },
        { "LOAD",   [this]() { this->LoadStage("Data/Stage/stage_built.csv"); } }
    };

    int tx = 220;
    for (const auto& t : tools) {
        auto btn = new GuiButton(tx, 20, 75, 40, t.first);
        btn->onClick = t.second;
        toolbarButtons.push_back(btn);
        ObjectManager::Push(btn);
        tx += 80;
    }

    int startX = 20, startY = 100, btnSize = 40, gap = 10, columns = 3;
    for (int i = 0; i < (int)availableTiles.size(); i++) {
        int col = i % columns, row = i / columns;
        int bx = startX + col * (btnSize + gap), by = startY + row * (btnSize + gap);
        int tileId = availableTiles[i].id;
        auto btn = new GuiButton(bx, by, btnSize, btnSize, "");
        if (availableTiles[i].imageHandle != -1) btn->SetImage(availableTiles[i].imageHandle);
        btn->onClick = [this, tileId]() { selectedTileId = tileId; currentMode = EditMode::BRUSH; selectTool->Reset(); };
        paletteButtons.push_back(btn);
        ObjectManager::Push(btn);
    }
}

StageBuilderScene::~StageBuilderScene() {
    toolbarButtons.clear(); paletteButtons.clear();
    for (auto& tile : availableTiles) { if (tile.imageHandle != -1) DeleteGraph(tile.imageHandle); }
    ObjectManager::DeleteAllGameObject();
}

void StageBuilderScene::Update() {
    if (Input::IsKeyDown(KEY_INPUT_ESCAPE)) { SceneManager::ChangeScene("TITLE"); return; }

    if (Input::IsKeepKeyDown(KEY_INPUT_LCONTROL) || Input::IsKeepKeyDown(KEY_INPUT_RCONTROL)) {
        if (Input::IsKeyDown(KEY_INPUT_C)) copyTool->Execute(gridData, *selectTool);
        if (Input::IsKeyDown(KEY_INPUT_V)) pasteTool->Execute(gridData, undoRedo->GetUndoStack(), undoRedo->GetRedoStack());
    }

    auto& uStack = undoRedo->GetUndoStack();
    auto& rStack = undoRedo->GetRedoStack();

    switch (currentMode) {
    case EditMode::BRUSH:  brushTool->Update(gridData, selectedTileId, uStack, rStack); break;
    case EditMode::ERASER: eraserTool->Update(gridData, uStack, rStack); break;
    case EditMode::SELECT: selectTool->Update(); break;
    case EditMode::MOVE:   moveTool->Update(gridData, *selectTool, uStack, rStack); break;
    }
}

void StageBuilderScene::Draw() {
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            int x = gridStartX + c * TILE_SIZE, y = gridStartY + r * TILE_SIZE;
            DrawBox(x, y, x + TILE_SIZE, y + TILE_SIZE, GetColor(60, 60, 60), FALSE);
            int tileId = gridData[r][c];
            if (tileId != 0) {
                auto it = std::find_if(availableTiles.begin(), availableTiles.end(), [tileId](const BuilderTile& t) { return t.id == tileId; });
                if (it != availableTiles.end() && it->imageHandle != -1) DrawGraph(x, y, it->imageHandle, TRUE);
            }
        }
    }

    DrawBox(0, 0, gridStartX, 720, GetColor(40, 40, 45), TRUE);
    DrawLine(gridStartX, 0, gridStartX, 720, GetColor(100, 100, 100));
    DrawString(20, 40, "TILE PALETTE", GetColor(255, 255, 255));

    DrawBox(gridStartX, 0, 1280, gridStartY, GetColor(45, 45, 50), TRUE);
    DrawLine(gridStartX, gridStartY, 1280, gridStartY, GetColor(100, 100, 100));

    std::string modeTxt = "MODE: ";
    if (currentMode == EditMode::BRUSH) modeTxt += "BRUSH";
    else if (currentMode == EditMode::ERASER) modeTxt += "ERASER";
    else if (currentMode == EditMode::SELECT) modeTxt += "SELECT";
    else if (currentMode == EditMode::MOVE) modeTxt += "MOVE";
    DrawString(gridStartX + 20, 60, modeTxt.c_str(), GetColor(0, 255, 0));

    int startX = 20, startY = 100, btnSize = 40, gap = 10, columns = 3;
    for (int i = 0; i < (int)availableTiles.size(); i++) {
        if (availableTiles[i].id == selectedTileId && currentMode == EditMode::BRUSH) {
            int col = i % columns, row = i / columns;
            int bx = startX + col * (btnSize + gap), by = startY + row * (btnSize + gap);
            DrawBox(bx - 3, by - 3, bx + btnSize + 3, by + btnSize + 3, GetColor(255, 215, 0), FALSE);
        }
    }

    // 範囲選択があれば描画（どのモードでも枠線は常に見えるようにする）
    selectTool->Draw();
}

void StageBuilderScene::LoadTileConfig() {
    std::ifstream file("D:\\GE3A31\\03_MyGame\\Spartan_Cat_Tower\\Data\\Stage\\TileConfig.csv");
    if (!file.is_open()) return;
    std::string line; std::getline(file, line);
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line); std::string idStr, pathStr;
        std::getline(ss, idStr, ','); std::getline(ss, pathStr, ',');
        if (idStr.empty()) continue;
        BuilderTile tile; tile.id = std::stoi(idStr);
        std::string fullPath = "D:\\GE3A31\\03_MyGame\\Spartan_Cat_Tower\\Data\\Image\\" + pathStr;
        tile.imageHandle = LoadGraph(fullPath.c_str());
        availableTiles.push_back(tile);
    }
    file.close();
}

void StageBuilderScene::SaveStage(const std::string& filepath) {
    std::ofstream file(filepath); if (!file.is_open()) return;
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) { file << gridData[r][c]; if (c < GRID_COLS - 1) file << ","; }
        file << "\n";
    }
    file.close();
}

void StageBuilderScene::LoadStage(const std::string& filepath) {
    std::ifstream file(filepath); if (!file.is_open()) return;
    std::string line; int row = 0;
    while (std::getline(file, line) && row < GRID_ROWS) {
        std::stringstream ss(line); std::string val; int col = 0;
        while (std::getline(ss, val, ',') && col < GRID_COLS) { gridData[row][col] = std::stoi(val); col++; }
        row++;
    }
    file.close();
    undoRedo->Clear();
}