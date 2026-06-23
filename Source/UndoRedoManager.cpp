//
#include "UndoRedoManager.h"

void UndoRedoManager::Undo(std::vector<std::vector<int>>& grid) {
    if (undoStack.empty()) return;
    auto cmd = undoStack.top();
    cmd->Undo(grid);
    redoStack.push(cmd);
    undoStack.pop();
}

void UndoRedoManager::Redo(std::vector<std::vector<int>>& grid) {
    if (redoStack.empty()) return;
    auto cmd = redoStack.top();
    cmd->Redo(grid);
    undoStack.push(cmd);
    redoStack.pop();
}

void UndoRedoManager::Clear() {
    while (!undoStack.empty()) undoStack.pop();
    while (!redoStack.empty()) redoStack.pop();
}