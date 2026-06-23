//
#pragma once
#include <vector>
#include <memory>
#include <stack>

enum class EditMode { BRUSH, ERASER, SELECT, FILL, MOVE };

//コマンドパターンの基底 (Undo/Redo用)
class BuilderCommand {
public:
    virtual ~BuilderCommand() = default;
    virtual void Undo(std::vector<std::vector<int>>& grid) = 0;
    virtual void Redo(std::vector<std::vector<int>>& grid) = 0;
};

//ツール共通の基底クラス
class BuilderToolBase {
protected:
    int gridStartX, gridStartY, tileSize, maxCols, maxRows;
public:
    BuilderToolBase(int startX, int startY, int size, int cols, int rows);
    virtual ~BuilderToolBase() = default;
    bool GetGridCoords(int& outCol, int& outRow);
};