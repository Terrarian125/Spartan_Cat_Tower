//
#pragma once
#include "BuilderToolBase.h"
#include "SelectTool.h"

class CopyTool : public BuilderToolBase {
public:
    using BuilderToolBase::BuilderToolBase;
    void Execute(const std::vector<std::vector<int>>& grid, const SelectTool& selectTool);
};