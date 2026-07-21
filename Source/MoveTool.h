// MoveTool.h
#pragma once
#include "BuilderToolBase.h"
#include "SelectTool.h"

class MoveTool : public BuilderToolBase {
private:
    bool isDragging = false;
    std::vector<std::vector<int>> draggedData;
    int srcSCol, srcSRow, srcECol, srcERow;
public:
    using BuilderToolBase::BuilderToolBase;
    void Update(std::vector<std::vector<int>>& grid, SelectTool& selectTool, std::stack<std::shared_ptr<BuilderCommand>>& undoStack, std::stack<std::shared_ptr<BuilderCommand>>& redoStack, int cameraX, int cameraY, float scale);
};