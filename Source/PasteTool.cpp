// PasteTool.cpp
#include "PasteTool.h"
#include "TileStrokeCommand.h"
#include <Windows.h>
#include <sstream>

//引数にカメラ情報を追加
void PasteTool::Execute(std::vector<std::vector<int>>& grid, std::stack<std::shared_ptr<BuilderCommand>>& undoStack, std::stack<std::shared_ptr<BuilderCommand>>& redoStack, int cameraX, int cameraY, float scale) {
    int col, row;
    //GetGridCoordsにカメラ位置とスケールを渡す
    if (!GetGridCoords(col, row, cameraX, cameraY, scale)) return;

    if (!OpenClipboard(NULL)) return;
    HANDLE hData = GetClipboardData(CF_TEXT); if (hData == NULL) { CloseClipboard(); return; }
    char* pMem = (char*)GlobalLock(hData); if (pMem == nullptr) { CloseClipboard(); return; }
    std::string clipText(pMem); GlobalUnlock(hData); CloseClipboard();

    std::stringstream ss(clipText); std::string line; int rOffset = 0;
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> changes;
    while (std::getline(ss, line)) {
        int tr = row + rOffset; if (tr >= maxRows) break;
        std::stringstream lineStream(line); std::string val; int cOffset = 0;
        while (std::getline(lineStream, val, ',')) {
            int tc = col + cOffset; if (tc >= maxCols) break;
            if (!val.empty()) {
                int newId = std::stoi(val);
                if (grid[tr][tc] != newId) {
                    changes.push_back({ {tc, tr}, {grid[tr][tc], newId} });
                    grid[tr][tc] = newId;
                }
            }
            cOffset++;
        }
        rOffset++;
    }
    if (!changes.empty()) {
        undoStack.push(std::make_shared<TileStrokeCommand>(changes));
        while (!redoStack.empty()) redoStack.pop();
    }
}