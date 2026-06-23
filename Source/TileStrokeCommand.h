//
#pragma once
#include "BuilderToolBase.h"

//1ストローク分のタイル変更をまとめて記録・適用する汎用コマンド
class TileStrokeCommand : public BuilderCommand {
private:
    struct TileChange { int col, row, oldId, newId; };
    std::vector<TileChange> changes;
public:
    TileStrokeCommand(const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>& strokeChanges) {
        for (const auto& c : strokeChanges) {
            changes.push_back({ c.first.first, c.first.second, c.second.first, c.second.second });
        }
    }
    void Undo(std::vector<std::vector<int>>& grid) override {
        for (auto it = changes.rbegin(); it != changes.rend(); ++it) grid[it->row][it->col] = it->oldId;
    }
    void Redo(std::vector<std::vector<int>>& grid) override {
        for (const auto& c : changes) grid[c.row][c.col] = c.newId;
    }
};