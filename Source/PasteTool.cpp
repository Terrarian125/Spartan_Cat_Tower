#include "PasteTool.h"
#include "TileStrokeCommand.h"
#include <Windows.h>
#include <sstream>

void PasteTool::Execute(std::vector<std::vector<int>>& grid, std::stack<std::shared_ptr<BuilderCommand>>& undoStack, std::stack<std::shared_ptr<BuilderCommand>>& redoStack, int cameraX, int cameraY, float scale) {
    int col, row;
    if (!GetGridCoords(col, row, cameraX, cameraY, scale)) return;

    if (!OpenClipboard(NULL)) return;
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == NULL) { CloseClipboard(); return; }
    char* pMem = (char*)GlobalLock(hData);
    if (pMem == nullptr) { CloseClipboard(); return; }
    std::string clipText(pMem);
    GlobalUnlock(hData);
    CloseClipboard();

    std::stringstream ss(clipText);
    std::string line;
    int rOffset = 0;
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> changes;

    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();

        int tr = row + rOffset;
        if (tr < maxRows && tr >= 0) {
            std::stringstream lineStream(line);
            std::string val;
            int cOffset = 0;
            while (std::getline(lineStream, val, ',')) {
                int tc = col + cOffset;
                if (tc < maxCols && tc >= 0) {
                    if (!val.empty()) {
                        try {
                            int newId = std::stoi(val);
                            if (grid[tr][tc] != newId) {
                                changes.push_back({ {tc, tr}, {grid[tr][tc], newId} });
                                grid[tr][tc] = newId;
                            }
                        }
                        catch (...) {}
                    }
                }
                cOffset++;
            }
        }
        rOffset++;
    }

    if (!changes.empty()) {
        undoStack.push(std::make_shared<TileStrokeCommand>(changes));
        while (!redoStack.empty()) redoStack.pop();
    }
}