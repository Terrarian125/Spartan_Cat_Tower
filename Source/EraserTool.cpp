//消しゴム 
#include "EraserTool.h"
#include "TileStrokeCommand.h"
#include "../Library/Input.h"
#include "DxLib.h"

void EraserTool::Update(std::vector<std::vector<int>>& grid, std::stack<std::shared_ptr<BuilderCommand>>& undoStack, std::stack<std::shared_ptr<BuilderCommand>>& redoStack) {
    int col, row;
    int keepLeft = Input::IsKeepMouseDown(MOUSE_INPUT_LEFT);
    if (GetGridCoords(col, row)) {
        if (keepLeft == 1) currentStroke.clear();
        if (keepLeft >= 1) {
            if (grid[row][col] != 0) {
                currentStroke.push_back({ {col, row}, {grid[row][col], 0} });
                grid[row][col] = 0;
            }
        }
    }
    if (Input::IsMouseUP(MOUSE_INPUT_LEFT) && !currentStroke.empty()) {
        undoStack.push(std::make_shared<TileStrokeCommand>(currentStroke));
        while (!redoStack.empty()) redoStack.pop();
        currentStroke.clear();
    }
}