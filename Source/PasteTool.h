// PasteTool.h
#pragma once
#include "BuilderToolBase.h"

class PasteTool : public BuilderToolBase {
public:
    using BuilderToolBase::BuilderToolBase;
    //引数にカメラ情報を追加
    void Execute(std::vector<std::vector<int>>& grid, std::stack<std::shared_ptr<BuilderCommand>>& undoStack, std::stack<std::shared_ptr<BuilderCommand>>& redoStack, int cameraX, int cameraY, float scale);
};