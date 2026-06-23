//
#pragma once
#include "BuilderToolBase.h"

class PasteTool : public BuilderToolBase {
public:
    using BuilderToolBase::BuilderToolBase;
    void Execute(std::vector<std::vector<int>>& grid, std::stack<std::shared_ptr<BuilderCommand>>& undoStack, std::stack<std::shared_ptr<BuilderCommand>>& redoStack);
};