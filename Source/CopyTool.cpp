#include "CopyTool.h"
#include <Windows.h>
#include <sstream>

void CopyTool::Execute(const std::vector<std::vector<int>>& grid, const SelectTool& selectTool) {
    if (!selectTool.HasSelection()) return;
    int sc, sr, ec, er; selectTool.GetBounds(sc, sr, ec, er);
    std::stringstream ss;
    for (int r = sr; r <= er; r++) {
        for (int c = sc; c <= ec; c++) {
            ss << grid[r][c];
            if (c < ec) ss << ",";
        }
        ss << "\r\n"; //Windowsのクリップボードフォーマットに合わせCRLFにする
    }
    std::string text = ss.str();
    if (!OpenClipboard(NULL)) return;
    EmptyClipboard();
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (hMem != NULL) {
        char* pMem = (char*)GlobalLock(hMem);
        if (pMem != nullptr) {
            memcpy(pMem, text.c_str(), text.size() + 1);
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
        }
    }
    CloseClipboard();
}