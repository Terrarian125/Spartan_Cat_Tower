// BuilderToolBase.cpp
#include "BuilderToolBase.h"
#include "../Library/Input.h"

BuilderToolBase::BuilderToolBase(int startX, int startY, int size, int cols, int rows)
    : gridStartX(startX), gridStartY(startY), tileSize(size), maxCols(cols), maxRows(rows) {
}

bool BuilderToolBase::GetGridCoords(int& outCol, int& outRow, int cameraX, int cameraY, float scale) {
    int mx = Input::GetMouseX();
    int my = Input::GetMouseY();

    //マウスの絶対画面座標からカメラとスケールを逆算してグリッド上のローカル座標を出す
    float currentTileSize = tileSize * scale;

    if (mx >= gridStartX && my >= gridStartY) {
        int col = static_cast<int>((mx - gridStartX + cameraX) / currentTileSize);
        int row = static_cast<int>((my - gridStartY + cameraY) / currentTileSize);

        if (col >= 0 && col < maxCols && row >= 0 && row < maxRows) {
            outCol = col;
            outRow = row;
            return true;
        }
    }
    return false;
}