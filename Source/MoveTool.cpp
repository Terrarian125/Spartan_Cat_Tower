// MoveTool.cpp
#include "MoveTool.h"
#include "TileStrokeCommand.h"
#include "../Library/Input.h"
#include "DxLib.h"

//引数のシグネチャを合わせ、GetGridCoordsにそのまま渡す
void MoveTool::Update(std::vector<std::vector<int>>& grid, SelectTool& selectTool, std::stack<std::shared_ptr<BuilderCommand>>& undoStack, std::stack<std::shared_ptr<BuilderCommand>>& redoStack, int cameraX, int cameraY, float scale) {
    if (!selectTool.HasSelection()) return;
    int col, row;
    int keepLeft = Input::IsKeepMouseDown(MOUSE_INPUT_LEFT);

    //カメラ情報を渡す
    if (GetGridCoords(col, row, cameraX, cameraY, scale)) {
        if (keepLeft == 1) {
            selectTool.GetBounds(srcSCol, srcSRow, srcECol, srcERow);
            if (col >= srcSCol && col <= srcECol && row >= srcSRow && row <= srcERow) {
                isDragging = true;
                draggedData.assign(srcERow - srcSRow + 1, std::vector<int>(srcECol - srcSCol + 1, 0));
                for (int r = srcSRow; r <= srcERow; r++) {
                    for (int c = srcSCol; c <= srcECol; c++) draggedData[r - srcSRow][c - srcSCol] = grid[r][c];
                }
            }
        }
    }

    //離した際のGetGridCoordsにもカメラ情報を渡す
    if (isDragging && Input::IsMouseUP(MOUSE_INPUT_LEFT) && GetGridCoords(col, row, cameraX, cameraY, scale)) {
        isDragging = false;
        std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> changes;
        for (int r = srcSRow; r <= srcERow; r++) {
            for (int c = srcSCol; c <= srcECol; c++) { changes.push_back({ {c, r}, {grid[r][c], 0} }); grid[r][c] = 0; }
        }
        int h = (int)draggedData.size(), w = (int)draggedData[0].size();
        for (int r = 0; r < h; r++) {
            for (int c = 0; c < w; c++) {
                int tr = row + r, tc = col + c;
                if (tr >= 0 && tr < maxRows && tc >= 0 && tc < maxCols) {
                    changes.push_back({ {tc, tr}, {grid[tr][tc], draggedData[r][c]} });
                    grid[tr][tc] = draggedData[r][c];
                }
            }
        }
        if (!changes.empty()) {
            undoStack.push(std::make_shared<TileStrokeCommand>(changes));
            while (!redoStack.empty()) redoStack.pop();
        }
        selectTool.Reset();
    }
}