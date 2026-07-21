// FillTool.cpp
#include "FillTool.h"
#include "TileStrokeCommand.h"

void FillTool::Execute(std::vector<std::vector<int>>& grid, int tileId, const SelectTool& selectTool, std::stack<std::shared_ptr<BuilderCommand>>& undoStack, std::stack<std::shared_ptr<BuilderCommand>>& redoStack, int cameraX, int cameraY, float scale) {
    int col, row; if (!GetGridCoords(col, row, cameraX, cameraY, scale)) return;
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> changes;
    int targetId = grid[row][col]; if (targetId == tileId) return;

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

std::vector<std::pair<int, int>> FillTool::GetPreviewArea(std::vector<std::vector<int>>& grid, const SelectTool& selectTool, int cameraX, int cameraY, float scale) {
    std::vector<std::pair<int, int>> area;
    int col, row; if (!GetGridCoords(col, row, cameraX, cameraY, scale)) return area;
    int targetId = grid[row][col];

    if (selectTool.HasSelection()) {
        int sc, sr, ec, er; selectTool.GetBounds(sc, sr, ec, er);
        if (col >= sc && col <= ec && row >= sr && row <= er) {
            for (int r = sr; r <= er; r++) {
                for (int c = sc; c <= ec; c++) { if (grid[r][c] == targetId) area.push_back({ c, r }); }
            }
        }
    }
    else {
        //簡易シードフィルでプレビュー座標収集
        std::vector<std::pair<int, int>> queue = { {col, row} };
        std::vector<std::vector<bool>> visited(maxRows, std::vector<bool>(maxCols, false));
        visited[row][col] = true;
        while (!queue.empty()) {
            auto curr = queue.back(); queue.pop_back();
            area.push_back(curr);
            int dc[] = { 0, 0, -1, 1 }, dr[] = { -1, 1, 0, 0 };
            for (int i = 0; i < 4; i++) {
                int nc = curr.first + dc[i], nr = curr.second + dr[i];
                if (nc >= 0 && nc < maxCols && nr >= 0 && nr < maxRows) {
                    if (!visited[nr][nc] && grid[nr][nc] == targetId) {
                        visited[nr][nc] = true; queue.push_back({ nc, nr });
                    }
                }
            }
        }
    }
    return area;
}

void FillTool::FloodFill(std::vector<std::vector<int>>& grid, int sc, int sr, int targetId, int replaceId, std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>& changes) {
    std::vector<std::pair<int, int>> queue = { {sc, sr} };
    std::vector<std::vector<bool>> visited(maxRows, std::vector<bool>(maxCols, false));
    visited[sr][sc] = true;
    while (!queue.empty()) {
        auto curr = queue.back(); queue.pop_back();
        changes.push_back({ {curr.first, curr.second}, {grid[curr.second][curr.first], replaceId} });
        grid[curr.second][curr.first] = replaceId;
        int dc[] = { 0, 0, -1, 1 }, dr[] = { -1, 1, 0, 0 };
        for (int i = 0; i < 4; i++) {
            int nc = curr.first + dc[i], nr = curr.second + dr[i];
            if (nc >= 0 && nc < maxCols && nr >= 0 && nr < maxRows) {
                if (!visited[nr][nc] && grid[nr][nc] == targetId) {
                    visited[nr][nc] = true; queue.push_back({ nc, nr });
                }
            }
        }
    }
}