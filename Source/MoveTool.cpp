//移動
#include "MoveTool.h"
#include "TileStrokeCommand.h"
#include "../Library/Input.h"
#include "DxLib.h"

void MoveTool::Update(std::vector<std::vector<int>>& grid, SelectTool& selectTool, std::stack<std::shared_ptr<BuilderCommand>>& undoStack, std::stack<std::shared_ptr<BuilderCommand>>& redoStack) {
    if (!selectTool.HasSelection()) return;
    int col, row; int keepLeft = Input::IsKeepMouseDown(MOUSE_INPUT_LEFT);
    if (GetGridCoords(col, row)) {
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
    if (isDragging && Input::IsMouseUP(MOUSE_INPUT_LEFT) && GetGridCoords(col, row)) {
        isDragging = false;
        std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> changes;
        for (int r = srcSRow; r <= srcERow; r++) {
            for (int c = srcSCol; c <= srcECol; c++) { changes.push_back({ {c, r}, {grid[r][c], 0} }); grid[r][c] = 0; }
        }
        int h = (int)draggedData.size(), w = (int)draggedData[0].size();
        for (int r = 0; r < h; r++) {
            for (int c = 0; c < w; c++) {
                int tc = col + c, tr = row + r;
                if (tc < maxCols && tr < maxRows) { changes.push_back({ {tc, tr}, {grid[tr][tc], draggedData[r][c]} }); grid[tr][tc] = draggedData[r][c]; }
            }
        }
        undoStack.push(std::make_shared<TileStrokeCommand>(changes));
        while (!redoStack.empty()) redoStack.pop();
        selectTool.Reset();
    }
}