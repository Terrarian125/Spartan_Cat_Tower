#include "FillTool.h"
#include "TileStrokeCommand.h"

void FillTool::Execute(std::vector<std::vector<int>>& grid, int tileId, const SelectTool& selectTool, std::stack<std::shared_ptr<BuilderCommand>>& undoStack, std::stack<std::shared_ptr<BuilderCommand>>& redoStack) {
    int col, row; if (!GetGridCoords(col, row)) return;
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> changes;
    int targetId = grid[row][col]; if (targetId == tileId) return;

    // 範囲選択があればその中だけ、なければ全体をシードフィル
    if (selectTool.HasSelection()) {
        int sc, sr, ec, er; selectTool.GetBounds(sc, sr, ec, er);
        if (col >= sc && col <= ec && row >= sr && row <= er) {
            for (int r = sr; r <= er; r++) {
                for (int c = sc; c <= ec; c++) {
                    if (grid[r][c] == targetId) { changes.push_back({ {c, r}, {grid[r][c], tileId} }); grid[r][c] = tileId; }
                }
            }
        }
    }
    else {
        FloodFill(grid, col, row, targetId, tileId, changes);
    }
    if (!changes.empty()) {
        undoStack.push(std::make_shared<TileStrokeCommand>(changes));
        while (!redoStack.empty()) redoStack.pop();
    }
}

void FillTool::FloodFill(std::vector<std::vector<int>>& grid, int sc, int sr, int targetId, int replaceId, std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>& changes) {
    std::vector<std::pair<int, int>> queue = { {sc, sr} };
    while (!queue.empty()) {
        auto curr = queue.back(); queue.pop_back();
        int c = curr.first, r = curr.second;
        if (c < 0 || c >= maxCols || r < 0 || r >= maxRows || grid[r][c] != targetId) continue;
        changes.push_back({ {c, r}, {grid[r][c], replaceId} }); grid[r][c] = replaceId;
        queue.push_back({ c + 1, r }); queue.push_back({ c - 1, r }); queue.push_back({ c, r + 1 }); queue.push_back({ c, r - 1 });
    }
}