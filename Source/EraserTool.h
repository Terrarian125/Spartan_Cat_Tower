//
#pragma once
#include "BuilderToolBase.h"

class EraserTool : public BuilderToolBase {
private:
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> currentStroke;
public:
    using BuilderToolBase::BuilderToolBase;
    void Update(std::vector<std::vector<int>>& grid, std::stack<std::shared_ptr<BuilderCommand>>& undoStack, std::stack<std::shared_ptr<BuilderCommand>>& redoStack);
};