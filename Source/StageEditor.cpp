#include "StageEditor.h"
#include "../ImGui/imgui.h"
#include "DxLib.h"
#include "../Library/Input.h"
#include "Screen.h"
#include <string>

//コンストラクタ
StageEditor::StageEditor(Stage* _stage) : stage(_stage) {
    SetDrawOrder(0); //最前面に描画
}

//デストラクタ
StageEditor::~StageEditor() {
    //終了時に必要な処理があればここに記述
}

void StageEditor::Update() {
    //F1キーでデバッグモード切替
    //if (Input::IsKeyUP(KEY_INPUT_F1)) isDebug = !isDebug;
    //if (!isDebug) return;

    //Shift + W/S で拡大縮小
    if (CheckHitKey(KEY_INPUT_LSHIFT)) {
        if (CheckHitKey(KEY_INPUT_W)) zoomLevel += 0.02f;
        if (CheckHitKey(KEY_INPUT_S)) zoomLevel -= 0.02f;

        //限界値の設定 (0.2倍 ? 3.0倍)
        if (zoomLevel < 0.2f) zoomLevel = 0.2f;
        if (zoomLevel > 3.0f) zoomLevel = 3.0f;
    }
    else {
        UpdateCamera(); //Shiftを押していない時はカメラ移動
    }

    //ImGuiにマウスが吸われていない時だけタイル配置入力を受け付ける
    if (!ImGui::GetIO().WantCaptureMouse) {
        UpdateInput();
    }

    //Ctrl + S でクイックセーブ
    if (CheckHitKey(KEY_INPUT_LCONTROL) && Input::IsKeyUP(KEY_INPUT_S)) {
        if (stage) stage->SaveMap(stage->currentMapPath);
    }
}

void StageEditor::UpdateCamera() {
    if (!stage) return;
    float speed = CheckHitKey(KEY_INPUT_LSHIFT) ? camSpeed * 3.0f : camSpeed;

    if (CheckHitKey(KEY_INPUT_W)) stage->scroll.y -= speed;
    if (CheckHitKey(KEY_INPUT_S)) stage->scroll.y += speed;
    if (CheckHitKey(KEY_INPUT_A)) stage->scroll.x -= speed;
    if (CheckHitKey(KEY_INPUT_D)) stage->scroll.x += speed;
}

void StageEditor::UpdateInput() {
    if (!stage || stage->mapData.empty()) return;

    int mx = Input::GetMouseX();
    int my = Input::GetMouseY();

    //ズームを考慮したタイル座標変換
    float currentTileSize = (float)stage->TILE_SIZE * zoomLevel;
    int tx = (int)(((float)mx + stage->scroll.x) / currentTileSize);
    int ty = (int)(((float)my + stage->scroll.y) / currentTileSize);

    //配列の範囲外チェック
    if (ty < 0 || ty >= (int)stage->mapData.size()) return;
    if (tx < 0 || tx >= (int)stage->mapData[0].size()) return;

    //マウス入力処理
    if (GetMouseInput() & MOUSE_INPUT_MIDDLE) selectedID = stage->mapData[ty][tx]; //スポイト
    if (GetMouseInput() & MOUSE_INPUT_LEFT)   stage->mapData[ty][tx] = selectedID;     //配置
    if (GetMouseInput() & MOUSE_INPUT_RIGHT)  stage->mapData[ty][tx] = 0;              //消去
}

void StageEditor::ShowImGuiWindow() {
    if (!stage) return;

    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Stage Editor - Developer Mode")) {
        ImGui::End();
        return;
    }

    ImGui::Text("File: %s", stage->currentMapPath.c_str());

    //特定IDの一括置換
    ImGui::Separator();
    ImGui::Text("Global Replace");
    ImGui::InputInt("Target ID", &targetReplaceID);
    ImGui::InputInt("New ID", &newReplaceID);
    if (ImGui::Button("Replace All Tiles", ImVec2(-1, 25))) {
        for (auto& row : stage->mapData) {
            for (auto& tile : row) {
                if (tile == targetReplaceID) tile = newReplaceID;
            }
        }
    }

    ImGui::Separator();
    ImGui::Text("Zoom: %.2f", zoomLevel);
    if (ImGui::Button("Reset Zoom")) zoomLevel = 1.0f;

    if (ImGui::Button("Quick Save (Ctrl+S)", ImVec2(-1, 30))) {
        stage->SaveMap(stage->currentMapPath);
    }

    ImGui::Separator();
    ImGui::Text("Current BG: %s", stage->currentBgPath.c_str());

    ImGui::Separator();
    ImGui::Text("Tile Palette");

    //タイルパレットの子ウィンドウ
    if (ImGui::BeginChild("Palette", ImVec2(0, 250), true)) {
        int count = 0;
        for (auto const& [id, data] : stage->catalog) {
            if (id <= 1) continue; //空白や背景は除外

            ImGui::PushID(id);

            //選択中のタイルを黄色い枠で強調
            bool isSelected = (id == selectedID);
            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.8f, 0.0f, 0.6f));
            }

            //画像ハンドルが有効な場合のみ ImageButton を表示
            if (data.handles[0] > 0) {
                if (ImGui::ImageButton((ImTextureID)(intptr_t)data.handles[0], ImVec2(40, 40))) {
                    selectedID = id;
                }
            }
            else {
                //画像がない場合はIDテキストのボタン
                if (ImGui::Button(std::to_string(id).c_str(), ImVec2(40, 40))) {
                    selectedID = id;
                }
            }

            if (isSelected) ImGui::PopStyleColor();

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("ID: %d [%s]", id, data.func.c_str());
            }

            //4列ごとに改行
            if (++count % 4 != 0) ImGui::SameLine();
            ImGui::PopID();
        }
        ImGui::EndChild();
    }

    ImGui::Separator();
    ImGui::Text("Shortcuts:");
    ImGui::BulletText("Ctrl+S: Quick Save");
    ImGui::BulletText("Middle-Click: Dropper");
    ImGui::BulletText("Shift: Fast Move / Zoom (W/S)");

    ImGui::End();
}

void StageEditor::Draw() {
    if (!isDebug || !stage) return;

    float curSize = (float)stage->TILE_SIZE * zoomLevel;

    //グリッド描画（ズーム対応）
    int offsetX = -(int)stage->scroll.x % (int)curSize;
    int offsetY = -(int)stage->scroll.y % (int)curSize;

    for (int x = offsetX; x <= Screen::WIDTH; x += (int)curSize)
        DrawLine(x, 0, x, Screen::HEIGHT, GetColor(80, 80, 80));
    for (int y = offsetY; y <= Screen::HEIGHT; y += (int)curSize)
        DrawLine(0, y, Screen::WIDTH, y, GetColor(80, 80, 80));

    //ImGuiウィンドウの描画
    ShowImGuiWindow();

    //マウスカーソルに追従する配置プレビュー
    int mx = Input::GetMouseX();
    int my = Input::GetMouseY();
    if (stage->catalog.count(selectedID)) {
        int handle = stage->catalog[selectedID].handles[0];
        if (handle > 0) {
            DrawExtendGraph(mx + 10, my + 10, mx + 10 + (int)curSize, my + 10 + (int)curSize, handle, TRUE);
        }
    }
}

void StageEditor::DrawGrid() {
    if (!stage) return;

    int sX = (int)stage->scroll.x;
    int sY = (int)stage->scroll.y;
    int tSize = (int)((float)stage->TILE_SIZE * zoomLevel); //ズーム考慮

    //格子描画
    for (int x = -sX % tSize; x <= Screen::WIDTH; x += tSize) {
        DrawLine(x, 0, x, Screen::HEIGHT, GetColor(60, 60, 60));
    }
    for (int y = -sY % tSize; y <= Screen::HEIGHT; y += tSize) {
        DrawLine(0, y, Screen::WIDTH, y, GetColor(60, 60, 60));
    }

    //選択マスのハイライト計算
    int tx = (int)(((float)Input::GetMouseX() + stage->scroll.x) / (float)tSize);
    int ty = (int)(((float)Input::GetMouseY() + stage->scroll.y) / (float)tSize);
    int highlightX = tx * tSize - sX;
    int highlightY = ty * tSize - sY;

    DrawBox(highlightX, highlightY, highlightX + tSize, highlightY + tSize, GetColor(0, 255, 255), FALSE);
    DrawFormatString(10, 10, GetColor(255, 255, 0), "Grid: [%d, %d] Zoom: %.2f", tx, ty, zoomLevel);
}