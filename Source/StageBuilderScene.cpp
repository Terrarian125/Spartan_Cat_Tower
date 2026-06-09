// 最優先で NOMINMAX を定義し、Windowsのマクロを無効化します
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "StageBuilderScene.h"
#include <DxLib.h>
#include <Windows.h> // クリップボード用
#include "../Library/Input.h"
#include "../Library/ObjectManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm> // std::min, std::max用

// コマンドパターン：ブロック配置・消去の記録
class PlaceBlockCommand : public BuilderCommand {
private:
    struct TileChange {
        int col, row;
        int oldId;
        int newId;
    };
    std::vector<TileChange> changes;

public:
    PlaceBlockCommand(const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>& strokeChanges) {
        for (const auto& c : strokeChanges) {
            changes.push_back({ c.first.first, c.first.second, c.second.first, c.second.second });
        }
    }

    void Undo(std::vector<std::vector<int>>& grid) override {
        for (auto it = changes.rbegin(); it != changes.rend(); ++it) {
            grid[it->row][it->col] = it->oldId;
        }
    }

    void Redo(std::vector<std::vector<int>>& grid) override {
        for (const auto& c : changes) {
            grid[c.row][c.col] = c.newId;
        }
    }
};

// シーンの実装
StageBuilderScene::StageBuilderScene() {
    gridData.resize(GRID_ROWS, std::vector<int>(GRID_COLS, 0));

    LoadTileConfig();

    // ツールバー配置
    int tx = 10, ty = 10, tw = 90, th = 50, ti = 95;

    auto bBrush = new GuiButton(tx, ty, tw, th, "ブラシ");
    bBrush->onClick = [this]() { currentMode = EditMode::BRUSH; };
    toolbarButtons.push_back(bBrush);

    auto bEraser = new GuiButton(tx += ti, ty, tw, th, "消しゴム");
    bEraser->onClick = [this]() { currentMode = EditMode::ERASER; };
    toolbarButtons.push_back(bEraser);

    auto bSelect = new GuiButton(tx += ti, ty, tw, th, "範囲選択");
    bSelect->onClick = [this]() { currentMode = EditMode::SELECT; };
    toolbarButtons.push_back(bSelect);

    auto bFill = new GuiButton(tx += ti, ty, tw, th, "塗りつぶし");
    bFill->onClick = [this]() { currentMode = EditMode::FILL; };
    toolbarButtons.push_back(bFill);

    auto bMove = new GuiButton(tx += ti, ty, tw, th, "移動");
    bMove->onClick = [this]() { currentMode = EditMode::MOVE; };
    toolbarButtons.push_back(bMove);

    auto bCopy = new GuiButton(tx += ti, ty, tw, th, "コピー");
    bCopy->onClick = [this]() { CopyToClipboard(); };
    toolbarButtons.push_back(bCopy);

    auto bPaste = new GuiButton(tx += ti, ty, tw, th, "ペースト");
    bPaste->onClick = [this]() { PasteFromClipboard(); };
    toolbarButtons.push_back(bPaste);

    auto bUndo = new GuiButton(tx += ti, ty, tw, th, "Undo");
    bUndo->onClick = [this]() {
        if (!undoStack.empty()) {
            auto cmd = undoStack.top();
            undoStack.pop();
            cmd->Undo(gridData);
            redoStack.push(cmd);
        }
        }; // ←【修正】余分な括弧とセミコロンの重複を解消
    toolbarButtons.push_back(bUndo);

    auto bRedo = new GuiButton(tx += ti, ty, tw, th, "Redo");
    bRedo->onClick = [this]() {
        if (!redoStack.empty()) {
            auto cmd = redoStack.top();
            redoStack.pop();
            cmd->Redo(gridData);
            undoStack.push(cmd);
        }
        }; // ←【修正】余分な括弧とセミコロンの重複を解消
    toolbarButtons.push_back(bRedo);

    auto bLoad = new GuiButton(tx += ti, ty, tw, th, "読み込み");
    bLoad->onClick = [this]() { LoadFromCSV("Data/Stage/stage_edit.csv"); };
    toolbarButtons.push_back(bLoad);

    auto bSave = new GuiButton(tx += ti, ty, tw, th, "書き出し");
    bSave->onClick = [this]() { SaveToCSV("Data/Stage/stage_edit.csv"); };
    toolbarButtons.push_back(bSave);

    for (auto b : toolbarButtons) ObjectManager::Push(b);

    // 左側タイルパレットの配置
    int px = 20, py = 100, pw = 45, ph = 45, pi = 50;
    for (size_t i = 0; i < availableTiles.size(); i++) {
        int r = (int)i / 3;
        int c = (int)i % 3;
        int tileId = availableTiles[i].id;

        auto bTile = new GuiButton(px + (c * pi), py + (r * pi), pw, ph, "");
        if (availableTiles[i].imageHandle != -1) {
            bTile->SetImage(availableTiles[i].imageHandle);
        }
        bTile->onClick = [this, tileId]() {
            this->selectedTileId = tileId;
            this->currentMode = EditMode::BRUSH;
            }; // ←【修正】余分な括弧とセミコロンの重複を解消
        paletteButtons.push_back(bTile);
        ObjectManager::Push(bTile);
    }
}

StageBuilderScene::~StageBuilderScene() {
    for (auto& tile : availableTiles) {
        if (tile.imageHandle != -1) DeleteGraph(tile.imageHandle);
    }
    toolbarButtons.clear();
    paletteButtons.clear();
    ObjectManager::DeleteAllGameObject();
}

void StageBuilderScene::LoadTileConfig() {
    std::ifstream file("Data/TileConfig.csv");
    if (!file.is_open()) return;

    std::string line;
    std::getline(file, line); // ヘッダースキップ

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string idStr, imagePath, animCountStr, function, description, unlockStr;

        std::getline(ss, idStr, ',');
        std::getline(ss, imagePath, ',');
        std::getline(ss, animCountStr, ',');
        std::getline(ss, function, ',');
        std::getline(ss, description, ',');
        std::getline(ss, unlockStr, ',');

        if (unlockStr == "1") {
            BuilderTile tile;
            tile.id = std::stoi(idStr);
            tile.function = function;
            std::string fullPath = "Data/Image/" + imagePath;
            tile.imageHandle = LoadGraph(fullPath.c_str());
            availableTiles.push_back(tile);
        }
    }
    file.close();
}

void StageBuilderScene::ExecuteCommand(std::shared_ptr<BuilderCommand> cmd) {
    undoStack.push(cmd);
    while (!redoStack.empty()) redoStack.pop();
}

void StageBuilderScene::Update() {
    for (auto b : toolbarButtons) b->Update();
    for (auto b : paletteButtons) b->Update();

    int mx = Input::GetMouseX();
    int my = Input::GetMouseY();
    int col = (mx - gridStartX) / TILE_SIZE;
    int row = (my - gridStartY) / TILE_SIZE;

    bool isInsideGrid = (col >= 0 && col < GRID_COLS && row >= 0 && row < GRID_ROWS);

    if (Input::IsMouseDown(MOUSE_INPUT_LEFT) && isInsideGrid) {
        if (currentMode == EditMode::SELECT) {
            selectStartX = selectEndX = col;
            selectStartY = selectEndY = row;
            isSelecting = true;
        }
        else if (currentMode == EditMode::FILL) {
            int targetId = gridData[row][col];
            if (targetId != selectedTileId) {
                currentStrokeChanges.clear();
                FloodFill(col, row, targetId, selectedTileId);
                PushCurrentStrokeAsCommand();
            }
        }
    }

    if (Input::IsKeepMouseDown(MOUSE_INPUT_LEFT) > 0 && isInsideGrid) {
        if (currentMode == EditMode::BRUSH) {
            if (gridData[row][col] != selectedTileId) {
                bool alreadyChanged = false;
                for (const auto& c : currentStrokeChanges) {
                    if (c.first.first == col && c.first.second == row) alreadyChanged = true;
                }
                if (!alreadyChanged) {
                    currentStrokeChanges.push_back({ {col, row}, {gridData[row][col], selectedTileId} });
                    gridData[row][col] = selectedTileId;
                }
            }
        }
        else if (currentMode == EditMode::ERASER) {
            if (gridData[row][col] != 0) {
                bool alreadyChanged = false;
                for (const auto& c : currentStrokeChanges) {
                    if (c.first.first == col && c.first.second == row) alreadyChanged = true;
                }
                if (!alreadyChanged) {
                    currentStrokeChanges.push_back({ {col, row}, {gridData[row][col], 0} });
                    gridData[row][col] = 0;
                }
            }
        }
        else if (currentMode == EditMode::SELECT && isSelecting) {
            selectEndX = col;
            selectEndY = row;
        }
    }

    if (Input::IsMouseUP(MOUSE_INPUT_LEFT)) {
        if (currentMode == EditMode::BRUSH || currentMode == EditMode::ERASER) {
            PushCurrentStrokeAsCommand();
        }
        else if (currentMode == EditMode::SELECT) {
            isSelecting = false;
        }
    }
}

void StageBuilderScene::PushCurrentStrokeAsCommand() {
    if (!currentStrokeChanges.empty()) {
        auto cmd = std::make_shared<PlaceBlockCommand>(currentStrokeChanges);
        ExecuteCommand(cmd);
        currentStrokeChanges.clear();
    }
}

void StageBuilderScene::FloodFill(int x, int y, int targetId, int replacementId) {
    if (x < 0 || x >= GRID_COLS || y < 0 || y >= GRID_ROWS) return;
    if (gridData[y][x] != targetId) return;

    currentStrokeChanges.push_back({ {x, y}, {gridData[y][x], replacementId} });
    gridData[y][x] = replacementId;

    FloodFill(x + 1, y, targetId, replacementId);
    FloodFill(x - 1, y, targetId, replacementId);
    FloodFill(x, y + 1, targetId, replacementId);
    FloodFill(x, y - 1, targetId, replacementId);
}

void StageBuilderScene::Draw() {
    DrawBox(0, 0, 1280, 720, GetColor(30, 30, 35), TRUE);

    // グリッドとタイルの描画
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            int x1 = gridStartX + c * TILE_SIZE;
            int y1 = gridStartY + r * TILE_SIZE;

            DrawBox(x1, y1, x1 + TILE_SIZE, y1 + TILE_SIZE, GetColor(50, 50, 55), FALSE);

            int tileId = gridData[r][c];
            if (tileId != 0) {
                for (const auto& tile : availableTiles) {
                    if (tile.id == tileId && tile.imageHandle != -1) {
                        DrawExtendGraph(x1, y1, x1 + TILE_SIZE, y1 + TILE_SIZE, tile.imageHandle, TRUE);
                        break;
                    }
                }
            }
        }
    }

    // 範囲選択ハイライト
    if (selectStartX != -1 && currentMode == EditMode::SELECT) {
        int x1 = gridStartX + (std::min)(selectStartX, selectEndX) * TILE_SIZE;
        int y1 = gridStartY + (std::min)(selectStartY, selectEndY) * TILE_SIZE;
        int x2 = gridStartX + ((std::max)(selectStartX, selectEndX) + 1) * TILE_SIZE;
        int y2 = gridStartY + ((std::max)(selectStartY, selectEndY) + 1) * TILE_SIZE;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawBox(x1, y1, x2, y2, GetColor(0, 120, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        DrawBox(x1, y1, x2, y2, GetColor(0, 180, 255), FALSE);
    }

    for (auto b : toolbarButtons) b->Draw();

    DrawBox(10, 80, 190, 710, GetColor(20, 20, 25), TRUE);
    DrawString(20, 85, "TILE PALETTE", GetColor(200, 200, 200));
    for (auto b : paletteButtons) b->Draw();

    std::string modeText = "MODE: ";
    if (currentMode == EditMode::BRUSH) modeText += "ブラシ (ID:" + std::to_string(selectedTileId) + ")";
    if (currentMode == EditMode::ERASER) modeText += "消しゴム";
    if (currentMode == EditMode::SELECT) modeText += "範囲選択";
    if (currentMode == EditMode::FILL) modeText += "塗りつぶし";
    if (currentMode == EditMode::MOVE) modeText += "移動";
    DrawString(20, 55, modeText.c_str(), GetColor(255, 255, 255));
}

void StageBuilderScene::SaveToCSV(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) return;

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            file << gridData[r][c];
            if (c < GRID_COLS - 1) file << ",";
        }
        file << "\n";
    }
    file.close();
}

void StageBuilderScene::LoadFromCSV(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    std::string line;
    int row = 0;
    while (std::getline(file, line) && row < GRID_ROWS) {
        std::stringstream ss(line);
        std::string val;
        int col = 0;
        while (std::getline(ss, val, ',') && col < GRID_COLS) {
            gridData[row][col] = std::stoi(val);
            col++;
        }
        row++;
    }
    file.close();

    while (!undoStack.empty()) undoStack.pop();
    while (!redoStack.empty()) redoStack.pop();
}

void StageBuilderScene::CopyToClipboard() {
    if (selectStartX == -1) return;

    int xStart = (std::min)(selectStartX, selectEndX);
    int xEnd = (std::max)(selectStartX, selectEndX);
    int yStart = (std::min)(selectStartY, selectEndY);
    int yEnd = (std::max)(selectStartY, selectEndY);

    std::string clipText = "";
    for (int r = yStart; r <= yEnd; r++) {
        for (int c = xStart; c <= xEnd; c++) {
            clipText += std::to_string(gridData[r][c]);
            if (c < xEnd) clipText += ",";
        }
        if (r < yEnd) clipText += "\n";
    }

    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, clipText.size() + 1);
        if (hMem != NULL) {
            char* pMem = (char*)GlobalLock(hMem);
            if (pMem != nullptr) {
                strcpy_s(pMem, clipText.size() + 1, clipText.c_str());
            }
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
        }
        CloseClipboard();
    }
}

void StageBuilderScene::PasteFromClipboard() {
    int mx = Input::GetMouseX();
    int my = Input::GetMouseY();
    int startCol = (mx - gridStartX) / TILE_SIZE;
    int startRow = (my - gridStartY) / TILE_SIZE;

    if (startCol < 0 || startCol >= GRID_COLS || startRow < 0 || startRow >= GRID_ROWS) return;

    if (!OpenClipboard(NULL)) return;
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == NULL) { CloseClipboard(); return; }

    char* pMem = (char*)GlobalLock(hData);
    if (pMem == nullptr) { CloseClipboard(); return; }
    std::string clipText(pMem);
    GlobalUnlock(hData);
    CloseClipboard();

    std::stringstream ss(clipText);
    std::string line;
    int rOffset = 0;

    currentStrokeChanges.clear();

    while (std::getline(ss, line)) {
        int targetRow = startRow + rOffset;
        if (targetRow >= GRID_ROWS) break;

        std::stringstream lineStream(line);
        std::string val;
        int cOffset = 0;

        while (std::getline(lineStream, val, ',')) {
            int targetCol = startCol + cOffset;
            if (targetCol >= GRID_COLS) break;

            int newId = std::stoi(val);
            if (gridData[targetRow][targetCol] != newId) {
                currentStrokeChanges.push_back({ {targetCol, targetRow}, {gridData[targetRow][targetCol], newId} });
                gridData[targetRow][targetCol] = newId;
            }
            cOffset++;
        }
        rOffset++;
    }

    PushCurrentStrokeAsCommand();
}