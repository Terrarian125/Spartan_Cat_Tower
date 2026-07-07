// FillTool.h
#pragma once
#include "BuilderToolBase.h"
#include "SelectTool.h"

class FillTool : public BuilderToolBase {
public:
    using BuilderToolBase::BuilderToolBase;
    void Execute(std::vector<std::vector<int>>& grid, int tileId, const SelectTool& selectTool, std::stack<std::shared_ptr<BuilderCommand>>& undoStack, std::stack<std::shared_ptr<BuilderCommand>>& redoStack, int cameraX, int cameraY, float scale);

	//プレビュー用
    std::vector<std::pair<int, int>> GetPreviewArea(std::vector<std::vector<int>>& grid, const SelectTool& selectTool, int cameraX, int cameraY, float scale);
private:
    void FloodFill(std::vector<std::vector<int>>& grid, int sc, int sr, int targetId, int replaceId, std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>& changes);
};