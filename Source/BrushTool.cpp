//ブラシ 
#include "DxLib.h"
#include "BrushTool.h"
#include "TileStrokeCommand.h"
#include "../Library/Input.h"

void BrushTool::Update(std::vector<std::vector<int>>& grid, int tileId, std::stack<std::shared_ptr<BuilderCommand>>& undoStack, std::stack<std::shared_ptr<BuilderCommand>>& redoStack) {
    int col, row;
	int keepLeft = Input::IsKeepMouseDown(MOUSE_INPUT_LEFT);
    if (GetGridCoords(col, row)) {
        if (keepLeft == 1) currentStroke.clear();
        if (keepLeft >= 1) {
            if (grid[row][col] != tileId) {
                currentStroke.push_back({ {col, row}, {grid[row][col], tileId} });
                grid[row][col] = tileId;
            }
        }
    }
    if (Input::IsMouseUP(MOUSE_INPUT_LEFT) && !currentStroke.empty()) {
        undoStack.push(std::make_shared<TileStrokeCommand>(currentStroke));
        while (!redoStack.empty()) redoStack.pop();
        currentStroke.clear();
    }
}