//StageBuilderScene.cpp
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
#include "../Library/GuiButton.h"

StageBuilderScene::StageBuilderScene() {
    gridData.resize(GRID_ROWS, std::vector<int>(GRID_COLS, 0));

    //各ツールの初期化
    brushTool = std::make_unique<BrushTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    eraserTool = std::make_unique<EraserTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    selectTool = std::make_unique<SelectTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    fillTool = std::make_unique<FillTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    moveTool = std::make_unique<MoveTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    copyTool = std::make_unique<CopyTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    pasteTool = std::make_unique<PasteTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    undoRedo = std::make_unique<UndoRedoManager>();

    LoadTileConfig();

    //ツールバーボタンの設定
    std::vector<std::pair<std::string, std::function<void()>>> tools = {
        { "BRUSH",  [this]() { selectTool->Reset(); currentMode = EditMode::BRUSH; } },
        { "ERASER", [this]() { selectTool->Reset(); currentMode = EditMode::ERASER; } },
        { "SELECT", [this]() { currentMode = EditMode::SELECT; } },
        { "FILL",   [this]() { selectTool->Reset(); currentMode = EditMode::FILL; } },
        { "MOVE",   [this]() { currentMode = EditMode::MOVE; } },
        { "COPY",   [this]() {
            //範囲選択されていればコピーを実行
            if (selectTool->HasSelection()) {
                copyTool->Execute(gridData, *selectTool);
            }
        }},
        { "PASTE",  [this]() {
            //ペーストボタンが押された瞬間に、現在のマウス位置に対して即座に実行する
            auto& uStack = undoRedo->GetUndoStack();
            auto& rStack = undoRedo->GetRedoStack();
            pasteTool->Execute(gridData, uStack, rStack, cameraX, cameraY, scale);
        }},
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

    //パレットボタンの設定
    int startX = 20, startY = 100, btnSize = 40, gap = 10, columns = 3;
    for (int i = 0; i < (int)availableTiles.size(); i++) {
        int col = i % columns, row = i / columns;
        int bx = startX + col * (btnSize + gap), by = startY + row * (btnSize + gap);
        int tileId = availableTiles[i].id;
        auto btn = new GuiButton(bx, by, btnSize, btnSize, "");
        if (availableTiles[i].imageHandle != -1) btn->SetImage(availableTiles[i].imageHandle);
        btn->onClick = [this, tileId]() { selectTool->Reset(); selectedTileId = tileId; currentMode = EditMode::BRUSH; };
        paletteButtons.push_back(btn);
        ObjectManager::Push(btn);
    }

    //スクロールボタンの配置 (画面右下に配置)
    auto btnUp = new GuiButton(1150, 600, 40, 40, "▲");
    auto btnDown = new GuiButton(1150, 650, 40, 40, "▼");
    auto btnLeft = new GuiButton(1100, 650, 40, 40, "◀");
    auto btnRight = new GuiButton(1200, 650, 40, 40, "▶");
    scrollButtons = { btnUp, btnDown, btnLeft, btnRight };
    for (auto b : scrollButtons) ObjectManager::Push(b);
}

StageBuilderScene::~StageBuilderScene() {
    toolbarButtons.clear(); paletteButtons.clear(); scrollButtons.clear();
    for (auto& tile : availableTiles) { if (tile.imageHandle != -1) DeleteGraph(tile.imageHandle); }
    ObjectManager::DeleteAllGameObject();
}

void StageBuilderScene::Update() {
    if (Input::IsKeyDown(KEY_INPUT_ESCAPE)) { SceneManager::ChangeScene("TITLE"); return; }

    //コントロール＋ホイールによるズーム処理
    if (Input::IsKeepKeyDown(KEY_INPUT_LCONTROL) || Input::IsKeepKeyDown(KEY_INPUT_RCONTROL)) {
        int wheel = Input::GetMouseWheel();
        if (wheel > 0) scale += 0.1f;
        if (wheel < 0) scale -= 0.1f;
        scale = std::max(0.5f, std::min(scale, 2.0f)); //0.5倍〜2倍に制限
    }

    //アローキーによるカメラ移動処理
    int moveSpeed = 10;
    if (Input::IsKeepKeyDown(KEY_INPUT_UP))    cameraY -= moveSpeed;
    if (Input::IsKeepKeyDown(KEY_INPUT_DOWN))  cameraY += moveSpeed;
    if (Input::IsKeepKeyDown(KEY_INPUT_LEFT))  cameraX -= moveSpeed;
    if (Input::IsKeepKeyDown(KEY_INPUT_RIGHT)) cameraX += moveSpeed;

    //スクロールボタンの長押し判定
    if (Input::IsKeepMouseDown(MOUSE_INPUT_LEFT) > 0) {
        if (scrollButtons[0]->IsMouseOver()) cameraY -= moveSpeed; //▲
        if (scrollButtons[1]->IsMouseOver()) cameraY += moveSpeed; //▼
        if (scrollButtons[2]->IsMouseOver()) cameraX -= moveSpeed; //◀
        if (scrollButtons[3]->IsMouseOver()) cameraX += moveSpeed; //▶
    }

    //カメラの移動制限 (グリッド全体からはみ出さないガード)
    float currentTileSize = TILE_SIZE * scale;
    int maxCamX = static_cast<int>(GRID_COLS * currentTileSize) - (1280 - gridStartX);
    int maxCamY = static_cast<int>(GRID_ROWS * currentTileSize) - (720 - gridStartY);
    cameraX = std::max(0, std::min(cameraX, std::max(0, maxCamX)));
    cameraY = std::max(0, std::min(cameraY, std::max(0, maxCamY)));

    auto& uStack = undoRedo->GetUndoStack();
    auto& rStack = undoRedo->GetRedoStack();

    //各ツールへカメラ位置とスケールを伝達
    switch (currentMode) {
    case EditMode::BRUSH:  brushTool->Update(gridData, selectedTileId, uStack, rStack, cameraX, cameraY, scale); break;
    case EditMode::ERASER: eraserTool->Update(gridData, uStack, rStack, cameraX, cameraY, scale); break;
    case EditMode::SELECT: selectTool->Update(cameraX, cameraY, scale); break;
    case EditMode::FILL:
        if (Input::IsMouseDown(MOUSE_INPUT_LEFT)) {
            fillTool->Execute(gridData, selectedTileId, *selectTool, uStack, rStack, cameraX, cameraY, scale);
        }
        break;
    case EditMode::MOVE:
        //MOVEツールの更新処理（引数の構成がもし異なる場合はここを調整してください）
        moveTool->Update(gridData, *selectTool, uStack, rStack, cameraX, cameraY, scale);
        break;
    }
}

void StageBuilderScene::Draw() {
    float currentTileSize = TILE_SIZE * scale;

    //クリップエリアを設定（メイングリッド描画領域外にはみ出さないようにする）
    SetDrawArea(gridStartX, gridStartY, 1280, 720);

    //グリッドと配置済みタイルの描画
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            //カメラとスケールを反映した描画座標
            int x = gridStartX + static_cast<int>(c * currentTileSize) - cameraX;
            int y = gridStartY + static_cast<int>(r * currentTileSize) - cameraY;

            DrawBox(x, y, x + static_cast<int>(currentTileSize), y + static_cast<int>(currentTileSize), GetColor(60, 60, 60), FALSE);

            int tileId = gridData[r][c];
            if (tileId != 0) {
                auto it = std::find_if(availableTiles.begin(), availableTiles.end(), [tileId](const BuilderTile& t) { return t.id == tileId; });
                if (it != availableTiles.end() && it->imageHandle != -1) {
                    int imgW, imgH; GetGraphSize(it->imageHandle, &imgW, &imgH);
                    int srcTileW = imgW / it->animCount;
                    DrawRectExtendGraph(x, y, x + static_cast<int>(currentTileSize), y + static_cast<int>(currentTileSize), 0, 0, srcTileW, imgH, it->imageHandle, TRUE);
                }
            }
        }
    }

    //半透明プレビュー機能の描画
    int mouseCol, mouseRow;
    if (brushTool->GetGridCoords(mouseCol, mouseRow, cameraX, cameraY, scale)) {
        std::vector<std::pair<int, int>> previewTiles;

        if (currentMode == EditMode::BRUSH) {
            previewTiles.push_back({ mouseCol, mouseRow });
        }
        else if (currentMode == EditMode::FILL) {
            previewTiles = fillTool->GetPreviewArea(gridData, *selectTool, cameraX, cameraY, scale);
        }

        //ブレンドモードを半透明に設定（50%の薄さ）
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);

        auto it = std::find_if(availableTiles.begin(), availableTiles.end(), [this](const BuilderTile& t) { return t.id == selectedTileId; });
        for (const auto& pos : previewTiles) {
            int px = gridStartX + static_cast<int>(pos.first * currentTileSize) - cameraX;
            int py = gridStartY + static_cast<int>(pos.second * currentTileSize) - cameraY;

            if (currentMode == EditMode::ERASER) {
                //消しゴムツールは赤色の枠線／塗りでプレビュー
                DrawBox(px, py, px + static_cast<int>(currentTileSize), py + static_cast<int>(currentTileSize), GetColor(255, 0, 0), TRUE);
            }
            else if (it != availableTiles.end() && it->imageHandle != -1) {
                //ブラシ・塗りつぶしは選択中のタイルを半透明で描画
                int imgW, imgH; GetGraphSize(it->imageHandle, &imgW, &imgH);
                int srcTileW = imgW / it->animCount;
                DrawRectExtendGraph(px, py, px + static_cast<int>(currentTileSize), py + static_cast<int>(currentTileSize), 0, 0, srcTileW, imgH, it->imageHandle, TRUE);
            }
        }
        //ブレンドモードを通常に戻す
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    //選択範囲ツールの枠・塗りつぶしを描画
    if (currentMode == EditMode::SELECT || currentMode == EditMode::MOVE || selectTool->HasSelection()) {
        selectTool->Draw(cameraX, cameraY, scale);
    }

    //クリップエリアを全面に解除
    SetDrawArea(0, 0, 1280, 720);

    //UI背景の描画
    DrawBox(0, 0, gridStartX, 720, GetColor(40, 40, 45), TRUE);
    DrawLine(gridStartX, 0, gridStartX, 720, GetColor(100, 100, 100));
    DrawString(20, 40, "TILE PALETTE", GetColor(255, 255, 255));

    DrawBox(gridStartX, 0, 1280, gridStartY, GetColor(45, 45, 50), TRUE);
    DrawLine(gridStartX, gridStartY, 1280, gridStartY, GetColor(100, 100, 100));

    std::string modeTxt = "MODE: ";
    if (currentMode == EditMode::BRUSH) modeTxt += "BRUSH";
    else if (currentMode == EditMode::ERASER) modeTxt += "ERASER";
    else if (currentMode == EditMode::SELECT) modeTxt += "SELECT";
    else if (currentMode == EditMode::FILL) modeTxt += "FILL";
    else if (currentMode == EditMode::MOVE) modeTxt += "MOVE";
    DrawString(gridStartX + 20, 60, modeTxt.c_str(), GetColor(0, 255, 0));

    //パレットの選択枠表示
    int startX = 20, startY = 100, btnSize = 40, gap = 10, columns = 3;
    for (int i = 0; i < (int)availableTiles.size(); i++) {
        if (availableTiles[i].id == selectedTileId && currentMode == EditMode::BRUSH) {
            int col = i % columns, row = i / columns;
            int bx = startX + col * (btnSize + gap), by = startY + row * (btnSize + gap);
            DrawBox(bx - 3, by - 3, bx + btnSize + 3, by + btnSize + 3, GetColor(255, 215, 0), FALSE);
        }
    }
}

void StageBuilderScene::LoadTileConfig() {
    std::ifstream file("Data/Stage/TileConfig.csv");
    if (!file.is_open()) return;
    std::string line; std::getline(file, line);
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string idStr, pathStr, animStr, funcStr;
        std::getline(ss, idStr, ',');
        std::getline(ss, pathStr, ',');
        std::getline(ss, animStr, ',');
        std::getline(ss, funcStr, ',');
        if (idStr.empty()) continue;

        BuilderTile tile;
        tile.id = std::stoi(idStr);
        tile.animCount = animStr.empty() ? 1 : std::stoi(animStr);
        tile.function = funcStr;

        std::string fullPath = "Data/Image/" + pathStr;
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