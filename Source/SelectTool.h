#pragma once
#include "BuilderToolBase.h"

class SelectTool : public BuilderToolBase {
private:
    int startCol = -1, startRow = -1;
    int endCol = -1, endRow = -1;
    bool isSelecting = false;
public:
    using BuilderToolBase::BuilderToolBase;
    void Update();
    void Draw();
    void Reset();
    bool HasSelection() const;
    void GetBounds(int& sCol, int& sRow, int& eCol, int& eRow) const;
};