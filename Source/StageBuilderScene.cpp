#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "StageBuilderScene.h"
#include <DxLib.h>
#include <Windows.h>
#include <commdlg.h>
#include <direct.h>   //_mkdir用
#include <algorithm>
#include <fstream>
#include <sstream>

#include "../Library/Input.h"
#include "../Library/ObjectManager.h"
#include "../Library/CsvReader.h"
#include "../Library/SceneManager.h"

//保存・読み込み先は」stage_built.csv に指定
static const std::string DEFAULT_STAGE_PATH = "Data/Stage/stage_built.csv";

StageBuilderScene::StageBuilderScene() {
    gridData.resize(GRID_ROWS, std::vector<int>(GRID_COLS, 0));

    brushTool = std::make_unique<BrushTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    eraserTool = std::make_unique<EraserTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    selectTool = std::make_unique<SelectTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    fillTool = std::make_unique<FillTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    moveTool = std::make_unique<MoveTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    copyTool = std::make_unique<CopyTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    pasteTool = std::make_unique<PasteTool>(gridStartX, gridStartY, TILE_SIZE, GRID_COLS, GRID_ROWS);
    undoRedo = std::make_unique<UndoRedoManager>();

    LoadTileConfig();

    //デフォルト背景の読み込み
    bgHandle = LoadGraph(("Data/Image/" + currentBgPath).c_str());

    //ツールバーボタン
    std::vector<std::pair<std::string, std::function<void()>>> tools = {
        { "BRUSH",   [this]() { selectTool->Reset(); currentMode = EditMode::BRUSH; } },
        { "ERASER",  [this]() { selectTool->Reset(); currentMode = EditMode::ERASER; } },
        { "SELECT",  [this]() { currentMode = EditMode::SELECT; } },
        { "FILL",    [this]() { selectTool->Reset(); currentMode = EditMode::FILL; } },
        { "MOVE",    [this]() { currentMode = EditMode::MOVE; } },
        { "COPY",    [this]() { if (selectTool->HasSelection()) copyTool->Execute(gridData, *selectTool); } },
        { "PASTE",   [this]() { currentMode = EditMode::PASTE; } },
        { "SETTING", [this]() { isSettingOpen = !isSettingOpen; } },
        { "UNDO",    [this]() { undoRedo->Undo(gridData); }},
        { "REDO",    [this]() { undoRedo->Redo(gridData); }},
        { "SAVE",    [this]() { this->SaveStage(DEFAULT_STAGE_PATH); } },
        { "LOAD",    [this]() { this->LoadStage(DEFAULT_STAGE_PATH); } }
    };

    int tx = 220;
    for (const auto& t : tools) {
        auto btn = new GuiButton(tx, 20, 70, 40, t.first);
        btn->onClick = t.second;
        toolbarButtons.push_back(btn);
        ObjectManager::Push(btn);
        tx += 75;
    }

    //スクロールボタン配置
    auto btnUp = new GuiButton(1150, 600, 40, 40, "^");
    auto btnDown = new GuiButton(1150, 650, 40, 40, "v");
    auto btnLeft = new GuiButton(1100, 650, 40, 40, "<");
    auto btnRight = new GuiButton(1200, 650, 40, 40, ">");
    scrollButtons = { btnUp, btnDown, btnLeft, btnRight };
    for (auto b : scrollButtons) ObjectManager::Push(b);

    btnToggleScroll = new GuiButton(1200, 600, 40, 40, "Hide");
    btnToggleScroll->onClick = [this]() {
        this->showScrollButtons = !this->showScrollButtons;
        for (auto b : this->scrollButtons) b->SetActive(this->showScrollButtons);
        this->btnToggleScroll->label = this->showScrollButtons ? "Hide" : "Show";
        };
    ObjectManager::Push(btnToggleScroll);

    //起動時に stage_built.csv をロード
    LoadStage(DEFAULT_STAGE_PATH);
}

StageBuilderScene::~StageBuilderScene() {
    toolbarButtons.clear(); paletteButtons.clear(); scrollButtons.clear();
    for (auto& tile : availableTiles) { if (tile.imageHandle != -1) DeleteGraph(tile.imageHandle); }
    if (bgHandle != -1) DeleteGraph(bgHandle);
    ObjectManager::DeleteAllGameObject();
}

bool StageBuilderScene::IsMouseOverUI() const {
    int mx = Input::GetMouseX();
    int my = Input::GetMouseY();

    //設定ダイアログが開いているなら、全画面で操作をブロック
    if (isSettingOpen) return true;

    //左側のタイルパレット上にある場合
    if (mx < gridStartX) return true;

    //上部のツールバーエリア上にある場合
    if (my < gridStartY) return true;

    //スクロールボタン領域にある場合
    if (showScrollButtons) {
        if (mx >= 1080 && mx <= 1250 && my >= 590 && my <= 700) {
            return true;
        }
    }

    return false;
}

std::string StageBuilderScene::OpenImageFileDialog() {
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetMainWindowHandle();
    ofn.lpstrFilter = "PNG Image (*.png)\0*.png\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn)) {
        std::string fullPath(filename);
        size_t pos = fullPath.find("Data\\Image\\");
        if (pos != std::string::npos) {
            std::string rel = fullPath.substr(pos + 11);
            std::replace(rel.begin(), rel.end(), '\\', '/');
            return rel;
        }
        size_t lastSlash = fullPath.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
            return "Stage/" + fullPath.substr(lastSlash + 1);
        }
        return fullPath;
    }
    return "";
}

void StageBuilderScene::Update() {
    if (Input::IsKeyDown(KEY_INPUT_ESCAPE)) {
        if (isSettingOpen) { isSettingOpen = false; return; }
        SceneManager::ChangeScene("TITLE"); return;
    }

    int mx = Input::GetMouseX();
    int my = Input::GetMouseY();
    int wheel = Input::GetMouseWheel();

    //キーボードショートカット (Ctrl+C, Ctrl+V, Ctrl+Z, Ctrl+Y)
    bool isCtrl = Input::IsKeepKeyDown(KEY_INPUT_LCONTROL) || Input::IsKeepKeyDown(KEY_INPUT_RCONTROL);

    if (isCtrl) {
        if (Input::IsKeyDown(KEY_INPUT_C)) {
            if (selectTool->HasSelection()) {
                copyTool->Execute(gridData, *selectTool);
            }
        }
        if (Input::IsKeyDown(KEY_INPUT_V)) {
            currentMode = EditMode::PASTE;
            pasteTool->Execute(gridData, undoRedo->GetUndoStack(), undoRedo->GetRedoStack(), cameraX, cameraY, scale);
        }
        if (Input::IsKeyDown(KEY_INPUT_Z)) {
            undoRedo->Undo(gridData);
        }
        if (Input::IsKeyDown(KEY_INPUT_Y)) {
            undoRedo->Redo(gridData);
        }
    }

    //設定ダイアログ開閉中の操作
    if (isSettingOpen) {
        int dlgX = 340, dlgY = 100, dlgW = 600, dlgH = 520;

        if (mx >= dlgX && mx <= dlgX + dlgW && my >= dlgY && my <= dlgY + dlgH) {
            if (wheel != 0) {
                settingScrollY -= wheel * 20;
                settingScrollY = std::max(0, settingScrollY);
            }

            if (Input::IsMouseDown(MOUSE_INPUT_LEFT) == 1) {
                if (mx >= dlgX + 410 && mx <= dlgX + 520 && my >= dlgY + 55 && my <= dlgY + 85) {
                    std::string selectedPath = OpenImageFileDialog();
                    if (!selectedPath.empty()) {
                        currentBgPath = selectedPath;
                        if (bgHandle != -1) DeleteGraph(bgHandle);
                        bgHandle = LoadGraph(("Data/Image/" + currentBgPath).c_str());
                    }
                }

                int listStartY = dlgY + 140 - settingScrollY;
                for (size_t i = 0; i < availableTiles.size(); i++) {
                    int itemY = listStartY + static_cast<int>(i) * 45;
                    if (itemY >= dlgY + 140 && itemY <= dlgY + dlgH - 60) {
                        if (mx >= dlgX + 40 && mx <= dlgX + dlgW - 60 && my >= itemY && my <= itemY + 40) {
                            int id = availableTiles[i].id;
                            auto it = std::find(unlockTileIds.begin(), unlockTileIds.end(), id);
                            if (it != unlockTileIds.end()) unlockTileIds.erase(it);
                            else unlockTileIds.push_back(id);
                        }
                    }
                }

                if (mx >= dlgX + dlgW - 120 && my >= dlgY + dlgH - 45 && mx <= dlgX + dlgW - 20 && my <= dlgY + dlgH - 10) {
                    isSettingOpen = false;
                }
            }
        }
        return;
    }

    //左側パレットの操作＆縦スクロール
    if (mx < gridStartX) {
        if (wheel != 0) {
            paletteScrollY -= wheel * 30;
            paletteScrollY = std::max(0, paletteScrollY);
        }
        if (Input::IsMouseDown(MOUSE_INPUT_LEFT) == 1) {
            int startX = 20, startY = 100 - paletteScrollY, btnSize = 45, gap = 10, columns = 3;
            for (size_t i = 0; i < availableTiles.size(); i++) {
                int col = i % columns, row = static_cast<int>(i / columns);
                int bx = startX + col * (btnSize + gap), by = startY + row * (btnSize + gap);
                if (by >= 80 && by <= 720) {
                    if (mx >= bx && mx <= bx + btnSize && my >= by && my <= by + btnSize) {
                        selectTool->Reset();
                        selectedTileId = availableTiles[i].id;
                        currentMode = EditMode::BRUSH;
                        break;
                    }
                }
            }
        }
        return;
    }

    //ズーム＆カメラ操作
    if (isCtrl) {
        if (wheel > 0) scale += 0.1f;
        if (wheel < 0) scale -= 0.1f;
        scale = std::max(0.5f, std::min(scale, 2.0f));
    }

    int moveSpeed = 10;
    if (Input::IsKeepKeyDown(KEY_INPUT_UP))    cameraY -= moveSpeed;
    if (Input::IsKeepKeyDown(KEY_INPUT_DOWN))  cameraY += moveSpeed;
    if (Input::IsKeepKeyDown(KEY_INPUT_LEFT))  cameraX -= moveSpeed;
    if (Input::IsKeepKeyDown(KEY_INPUT_RIGHT)) cameraX += moveSpeed;

    if (Input::IsKeepMouseDown(MOUSE_INPUT_LEFT) > 0) {
        if (scrollButtons[0]->IsMouseOver()) cameraY -= moveSpeed;
        if (scrollButtons[1]->IsMouseOver()) cameraY += moveSpeed;
        if (scrollButtons[2]->IsMouseOver()) cameraX -= moveSpeed;
        if (scrollButtons[3]->IsMouseOver()) cameraX += moveSpeed;
    }

    float currentTileSize = TILE_SIZE * scale;
    int maxCamX = static_cast<int>(GRID_COLS * currentTileSize) - (1280 - gridStartX);
    int maxCamY = static_cast<int>(GRID_ROWS * currentTileSize) - (720 - gridStartY);
    cameraX = std::max(0, std::min(cameraX, std::max(0, maxCamX)));
    cameraY = std::max(0, std::min(cameraY, std::max(0, maxCamY)));

    auto& uStack = undoRedo->GetUndoStack();
    auto& rStack = undoRedo->GetRedoStack();
    //各編集モードの更新 (UI上ではスキップして誤爆を防ぐ)
    if (!IsMouseOverUI()) {
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
            moveTool->Update(gridData, *selectTool, uStack, rStack, cameraX, cameraY, scale);
            break;
        case EditMode::PASTE:
            if (Input::IsMouseDown(MOUSE_INPUT_LEFT)) {
                pasteTool->Execute(gridData, uStack, rStack, cameraX, cameraY, scale);
            }
            break;
        }
    }
}

void StageBuilderScene::Draw() {
    float currentTileSize = TILE_SIZE * scale;

    SetDrawArea(gridStartX, gridStartY, 1280, 720);

    //背景画像プレビュー描画
    if (bgHandle != -1) {
        DrawExtendGraph(gridStartX, gridStartY, 1280, 720, bgHandle, FALSE);
    }

    //グリッドとタイルの描画
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            int x = gridStartX + static_cast<int>(c * currentTileSize) - cameraX;
            int y = gridStartY + static_cast<int>(r * currentTileSize) - cameraY;

            DrawBox(x, y, x + static_cast<int>(currentTileSize), y + static_cast<int>(currentTileSize), GetColor(60, 60, 60), FALSE);

            int tileId = gridData[r][c];
            if (tileId != 0) {
                auto it = std::find_if(availableTiles.begin(), availableTiles.end(), [tileId](const BuilderTile& t) { return t.id == tileId; });
                if (it != availableTiles.end() && it->imageHandle != -1) {
                    int imgW, imgH; GetGraphSize(it->imageHandle, &imgW, &imgH);
                    int animCount = (it->animCount <= 0) ? 1 : it->animCount;
                    int srcTileW = imgW / animCount;
                    DrawRectExtendGraph(x, y, x + static_cast<int>(currentTileSize), y + static_cast<int>(currentTileSize), 0, 0, srcTileW, imgH, it->imageHandle, TRUE);
                }
            }
        }
    }

    //半透明プレビュー (UI上では非表示)
    int mouseCol, mouseRow;
    if (!IsMouseOverUI() && brushTool->GetGridCoords(mouseCol, mouseRow, cameraX, cameraY, scale)) {
        std::vector<std::pair<int, int>> previewTiles;
        if (currentMode == EditMode::BRUSH) previewTiles.push_back({ mouseCol, mouseRow });
        else if (currentMode == EditMode::FILL) previewTiles = fillTool->GetPreviewArea(gridData, *selectTool, cameraX, cameraY, scale);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
        auto it = std::find_if(availableTiles.begin(), availableTiles.end(), [this](const BuilderTile& t) { return t.id == selectedTileId; });
        for (const auto& pos : previewTiles) {
            int px = gridStartX + static_cast<int>(pos.first * currentTileSize) - cameraX;
            int py = gridStartY + static_cast<int>(pos.second * currentTileSize) - cameraY;

            if (currentMode == EditMode::ERASER) {
                DrawBox(px, py, px + static_cast<int>(currentTileSize), py + static_cast<int>(currentTileSize), GetColor(255, 0, 0), TRUE);
            }
            else if (it != availableTiles.end() && it->imageHandle != -1) {
                int imgW, imgH; GetGraphSize(it->imageHandle, &imgW, &imgH);
                int animCount = (it->animCount <= 0) ? 1 : it->animCount;
                int srcTileW = imgW / animCount;
                DrawRectExtendGraph(px, py, px + static_cast<int>(currentTileSize), py + static_cast<int>(currentTileSize), 0, 0, srcTileW, imgH, it->imageHandle, TRUE);
            }
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    if (currentMode == EditMode::SELECT || currentMode == EditMode::MOVE || selectTool->HasSelection()) {
        selectTool->Draw(cameraX, cameraY, scale);
    }

    SetDrawArea(0, 0, 1280, 720);

    //左側タイルパレットエリア
    DrawBox(0, 0, gridStartX, 720, GetColor(40, 40, 45), TRUE);
    DrawLine(gridStartX, 0, gridStartX, 720, GetColor(100, 100, 100));
    DrawString(20, 40, "TILE PALETTE", GetColor(255, 255, 255));

    SetDrawArea(0, 80, gridStartX, 720);
    int startX = 20, startY = 100 - paletteScrollY, btnSize = 45, gap = 10, columns = 3;
    for (size_t i = 0; i < availableTiles.size(); i++) {
        int id = availableTiles[i].id;
        int col = i % columns, row = static_cast<int>(i / columns);
        int bx = startX + col * (btnSize + gap), by = startY + row * (btnSize + gap);

        if (by + btnSize >= 80 && by <= 720) {
            DrawBox(bx, by, bx + btnSize, by + btnSize, GetColor(70, 70, 75), TRUE);
            DrawBox(bx, by, bx + btnSize, by + btnSize, GetColor(120, 120, 125), FALSE);

            if (availableTiles[i].imageHandle != -1) {
                int imgW, imgH; GetGraphSize(availableTiles[i].imageHandle, &imgW, &imgH);
                int animCount = (availableTiles[i].animCount <= 0) ? 1 : availableTiles[i].animCount;
                int srcTileW = imgW / animCount;
                DrawRectExtendGraph(bx + 2, by + 2, bx + btnSize - 2, by + btnSize - 2, 0, 0, srcTileW, imgH, availableTiles[i].imageHandle, TRUE);
            }

            if (id == selectedTileId && currentMode == EditMode::BRUSH) {
                DrawBox(bx - 2, by - 2, bx + btnSize + 2, by + btnSize + 2, GetColor(255, 215, 0), FALSE);
            }
        }
    }
    SetDrawArea(0, 0, 1280, 720);

    //上部ツールバーエリア
    DrawBox(gridStartX, 0, 1280, gridStartY, GetColor(45, 45, 50), TRUE);
    DrawLine(gridStartX, gridStartY, 1280, gridStartY, GetColor(100, 100, 100));

    std::string modeTxt = "MODE: ";
    if (currentMode == EditMode::BRUSH) modeTxt += "BRUSH";
    else if (currentMode == EditMode::ERASER) modeTxt += "ERASER";
    else if (currentMode == EditMode::SELECT) modeTxt += "SELECT";
    else if (currentMode == EditMode::FILL) modeTxt += "FILL";
    else if (currentMode == EditMode::MOVE) modeTxt += "MOVE";
    else if (currentMode == EditMode::PASTE) modeTxt += "PASTE";
    DrawString(gridStartX + 20, 60, modeTxt.c_str(), GetColor(0, 255, 0));

    //設定ダイアログ描画
    if (isSettingOpen) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160);
        DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        int dlgX = 340, dlgY = 100, dlgW = 600, dlgH = 520;
        DrawBox(dlgX, dlgY, dlgX + dlgW, dlgY + dlgH, GetColor(50, 52, 60), TRUE);
        DrawBox(dlgX, dlgY, dlgX + dlgW, dlgY + dlgH, GetColor(200, 200, 200), FALSE);
        DrawString(dlgX + 20, dlgY + 20, "STAGE SETTINGS", GetColor(255, 255, 255));

        DrawString(dlgX + 40, dlgY + 60, "Background Image:", GetColor(200, 200, 200));
        DrawBox(dlgX + 200, dlgY + 55, dlgX + 400, dlgY + 85, GetColor(70, 70, 80), TRUE);
        DrawBox(dlgX + 200, dlgY + 55, dlgX + 400, dlgY + 85, GetColor(150, 150, 150), FALSE);

        std::string bgFileName = currentBgPath.substr(currentBgPath.find_last_of('/') + 1);
        DrawString(dlgX + 210, dlgY + 62, bgFileName.c_str(), GetColor(255, 255, 255));

        DrawBox(dlgX + 410, dlgY + 55, dlgX + 520, dlgY + 85, GetColor(80, 120, 200), TRUE);
        DrawString(dlgX + 425, dlgY + 62, "[ Open... ]", GetColor(255, 255, 255));

        DrawString(dlgX + 40, dlgY + 115, "Unlock Tiles on Clear:", GetColor(200, 200, 200));

        SetDrawArea(dlgX + 40, dlgY + 140, dlgX + dlgW - 40, dlgY + dlgH - 60);
        int listStartY = dlgY + 140 - settingScrollY;

        for (size_t i = 0; i < availableTiles.size(); i++) {
            int itemY = listStartY + static_cast<int>(i) * 45;
            int id = availableTiles[i].id;
            bool isUnlocked = std::find(unlockTileIds.begin(), unlockTileIds.end(), id) != unlockTileIds.end();

            DrawBox(dlgX + 40, itemY, dlgX + dlgW - 60, itemY + 40, GetColor(65, 68, 75), TRUE);
            DrawBox(dlgX + 40, itemY, dlgX + dlgW - 60, itemY + 40, GetColor(100, 100, 100), FALSE);

            //チェックボックス描画
            DrawBox(dlgX + 50, itemY + 10, dlgX + 70, itemY + 30, GetColor(200, 200, 200), FALSE);
            if (isUnlocked) {
                DrawString(dlgX + 53, itemY + 12, "v", GetColor(50, 255, 50));
            }

            if (availableTiles[i].imageHandle != -1) {
                int imgW, imgH; GetGraphSize(availableTiles[i].imageHandle, &imgW, &imgH);
                int animCount = (availableTiles[i].animCount <= 0) ? 1 : availableTiles[i].animCount;
                int srcTileW = imgW / animCount;
                DrawRectExtendGraph(dlgX + 85, itemY + 5, dlgX + 115, itemY + 35, 0, 0, srcTileW, imgH, availableTiles[i].imageHandle, TRUE);
            }

            DrawFormatString(dlgX + 130, itemY + 12, GetColor(255, 255, 255), "ID: %d (%s)", id, availableTiles[i].function.c_str());
        }
        SetDrawArea(0, 0, 1280, 720);

        DrawBox(dlgX + dlgW - 120, dlgY + dlgH - 45, dlgX + dlgW - 20, dlgY + dlgH - 10, GetColor(100, 100, 100), TRUE);
        DrawString(dlgX + dlgW - 95, dlgY + dlgH - 35, "CLOSE", GetColor(255, 255, 255));
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
        int parsedAnim = animStr.empty() ? 1 : std::stoi(animStr);
        tile.animCount = (parsedAnim <= 0) ? 1 : parsedAnim;
        tile.function = funcStr;

        std::string fullPath = "Data/Image/" + pathStr;
        tile.imageHandle = LoadGraph(fullPath.c_str());
        availableTiles.push_back(tile);
    }
    file.close();
}

void StageBuilderScene::SaveStage(const std::string& filepath) {
    _mkdir("Data");
    _mkdir("Data/Stage");

    std::ofstream file(filepath);
    if (!file.is_open()) {
        OutputDebugStringA("Failed to open save file!\n");
        return;
    }

    std::string bgFileName = currentBgPath;
    size_t lastSlash = bgFileName.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        bgFileName = bgFileName.substr(lastSlash + 1);
    }

    std::string unlockStr = "";
    for (size_t i = 0; i < unlockTileIds.size(); i++) {
        unlockStr += std::to_string(unlockTileIds[i]) + (i < unlockTileIds.size() - 1 ? " " : "");
    }

    file << "CONFIG," << bgFileName << "," << unlockStr << "\n";

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            file << gridData[r][c];
            if (c < GRID_COLS - 1) file << ",";
        }
        file << "\n";
    }

    file.close();
}

void StageBuilderScene::LoadStage(const std::string& filepath) {
    CsvReader csv(filepath);
    if (csv.GetLines() <= 0) return;

    int startLine = 0;
    std::string firstTag = csv.GetString(0, 0);

    if (firstTag == "BG" || firstTag == "CONFIG") {
        std::string bgName = csv.GetString(0, 1);
        if (!bgName.empty()) {
            if (bgName.find("Stage/") != 0 && bgName.find("Stage\\") != 0) {
                currentBgPath = "Stage/" + bgName;
            }
            else {
                currentBgPath = bgName;
            }

            if (bgHandle != -1) DeleteGraph(bgHandle);
            bgHandle = LoadGraph(("Data/Image/" + currentBgPath).c_str());
        }

        unlockTileIds.clear();
        if (csv.GetColumns(0) >= 3) {
            std::string unlockStr = csv.GetString(0, 2);
            if (!unlockStr.empty()) {
                std::stringstream ss(unlockStr);
                int id;
                while (ss >> id) {
                    unlockTileIds.push_back(id);
                }
            }
        }
        startLine = 1;
    }

    gridData.assign(GRID_ROWS, std::vector<int>(GRID_COLS, 0));
    for (int r = startLine; r < csv.GetLines() && (r - startLine) < GRID_ROWS; r++) {
        for (int c = 0; c < csv.GetColumns(r) && c < GRID_COLS; c++) {
            std::string str = csv.GetString(r, c);
            if (!str.empty()) {
                try {
                    gridData[r - startLine][c] = std::stoi(str);
                }
                catch (...) {
                    gridData[r - startLine][c] = 0;
                }
            }
        }
    }

    undoRedo->Clear();
}