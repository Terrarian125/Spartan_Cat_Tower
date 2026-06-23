#pragma once
#include "BuilderToolBase.h"

class UndoRedoManager {
private:
    std::stack<std::shared_ptr<BuilderCommand>> undoStack;
    std::stack<std::shared_ptr<BuilderCommand>> redoStack;
public:
    void Undo(std::vector<std::vector<int>>& grid);
    void Redo(std::vector<std::vector<int>>& grid);
    void Clear();
    std::stack<std::shared_ptr<BuilderCommand>>& GetUndoStack() { return undoStack; }
    std::stack<std::shared_ptr<BuilderCommand>>& GetRedoStack() { return redoStack; }
};