#pragma once
#include <vector>
#include <memory>
#include <stack>

enum class EditMode { BRUSH, ERASER, SELECT, FILL, MOVE, PASTE };

class BuilderCommand {
public:
    virtual ~BuilderCommand() = default;
    virtual void Undo(std::vector<std::vector<int>>& grid) = 0;
    virtual void Redo(std::vector<std::vector<int>>& grid) = 0;
};

class BuilderToolBase {
protected:
    int gridStartX, gridStartY, tileSize, maxCols, maxRows;
public:
    BuilderToolBase(int startX, int startY, int size, int cols, int rows);
    virtual ~BuilderToolBase() = default;

    //カメラ
    bool GetGridCoords(int& outCol, int& outRow, int cameraX, int cameraY, float scale);
};