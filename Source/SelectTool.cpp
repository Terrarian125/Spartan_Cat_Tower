#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "SelectTool.h"
#include <DxLib.h>
#include <algorithm>
#include "../Library/Input.h"

// 引数にカメラ情報を追加して更新
void SelectTool::Update(int cameraX, int cameraY, float scale) {
    int col, row;
    // マウスの左クリック状態を取得
    int keepLeft = Input::IsKeepMouseDown(MOUSE_INPUT_LEFT);

    // 基底クラスのGetGridCoordsにすべての引数を渡してグリッド内判定を行う
    if (GetGridCoords(col, row, cameraX, cameraY, scale)) {
        if (keepLeft == 1) {
            // クリックされた瞬間：始点と終点を現在のマスに設定
            startCol = endCol = col;
            startRow = endRow = row;
            isSelecting = true;
        }
        else if (keepLeft > 1 && isSelecting) {
            // ドラッグ中：終点のみを更新
            endCol = col;
            endRow = row;
        }
    }

    // マウスが離されたらドラッグ選択を終了
    if (Input::IsMouseUP(MOUSE_INPUT_LEFT)) {
        isSelecting = false;
    }
}

// 描画処理（カメラ位置とズーム倍率を考慮）
void SelectTool::Draw(int cameraX, int cameraY, float scale) {
    // 選択が開始されていなければ描画しない
    if (startCol == -1) return;

    // 描画用の矩形範囲（左上と右下のグリッド座標）を計算
    int sc, sr, ec, er;
    GetBounds(sc, sr, ec, er);

    // 現在のカメラズーム倍率を考慮した1タイルのピクセルサイズ
    float currentTileSize = tileSize * scale;

    // グリッド座標から、カメラ位置(遷移量)とスケールを反映した画面上のピクセル座標を計算
    int x1 = gridStartX + static_cast<int>(sc * currentTileSize) - cameraX;
    int y1 = gridStartY + static_cast<int>(sr * currentTileSize) - cameraY;
    int x2 = gridStartX + static_cast<int>((ec + 1) * currentTileSize) - cameraX;
    int y2 = gridStartY + static_cast<int>((er + 1) * currentTileSize) - cameraY;

    // 選択範囲内を青色の半透明矩形で塗りつぶし描画
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
    DrawBox(x1, y1, x2, y2, GetColor(0, 120, 215), TRUE);

    // 選択範囲の枠線を不透明な青色で描画
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    DrawBox(x1, y1, x2, y2, GetColor(0, 120, 215), FALSE);
}

// 選択範囲のリセット
void SelectTool::Reset() {
    startCol = startRow = -1;
    endCol = endRow = -1;
    isSelecting = false;
}

// 現在範囲選択されているか
bool SelectTool::HasSelection() const {
    return startCol != -1;
}

// 選択範囲の開始・終了の最小/最大グリッド座標を取得
void SelectTool::GetBounds(int& sCol, int& sRow, int& eCol, int& eRow) const {
    sCol = std::min(startCol, endCol);
    sRow = std::min(startRow, endRow);
    eCol = std::max(startCol, endCol);
    eRow = std::max(startRow, endRow);
}