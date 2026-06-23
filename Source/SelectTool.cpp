//範囲選択
#include "SelectTool.h"
#include <DxLib.h>
#include <algorithm>
#include "../Library/Input.h"

void SelectTool::Update() {
    int col, row;
    int keepLeft = Input::IsKeepMouseDown(PAD_INPUT_LEFT);
    if (GetGridCoords(col, row)) {
        if (keepLeft == 1) {
            startCol = endCol = col; startRow = endRow = row; isSelecting = true;
        }
        else if (keepLeft > 1 && isSelecting) {
            endCol = col; endRow = row;
        }
    }
    if (Input::IsMouseUP(PAD_INPUT_LEFT)) isSelecting = false;
}

void SelectTool::Draw() {
    if (startCol == -1) return;
    int sc, sr, ec, er; GetBounds(sc, sr, ec, er);
    int x1 = gridStartX + sc * tileSize, y1 = gridStartY + sr * tileSize;
    int x2 = gridStartX + (ec + 1) * tileSize, y2 = gridStartY + (er + 1) * tileSize;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
    DrawBox(x1, y1, x2, y2, GetColor(0, 120, 215), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    DrawBox(x1, y1, x2, y2, GetColor(0, 120, 215), FALSE);
}

void SelectTool::Reset() { startCol = startRow = endCol = endRow = -1; isSelecting = false; }
bool SelectTool::HasSelection() const { return startCol != -1; }
void SelectTool::GetBounds(int& sCol, int& sRow, int& eCol, int& eRow) const {
    sCol = (std::min)(startCol, endCol); eCol = (std::max)(startCol, endCol);
    sRow = (std::min)(startRow, endRow); eRow = (std::max)(startRow, endRow);
}