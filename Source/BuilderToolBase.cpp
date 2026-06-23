//
#include "BuilderToolBase.h"
#include "../Library/Input.h"

BuilderToolBase::BuilderToolBase(int startX, int startY, int size, int cols, int rows)
    : gridStartX(startX), gridStartY(startY), tileSize(size), maxCols(cols), maxRows(rows) {
}

bool BuilderToolBase::GetGridCoords(int& outCol, int& outRow) {
    int mx = Input::GetMouseX();
    int my = Input::GetMouseY();
    if (mx >= gridStartX && mx < gridStartX + maxCols * tileSize &&
        my >= gridStartY && my < gridStartY + maxRows * tileSize) {
        outCol = (mx - gridStartX) / tileSize;
        outRow = (my - gridStartY) / tileSize;
        return true;
    }
    return false;
}