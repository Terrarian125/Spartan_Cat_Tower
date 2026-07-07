#pragma once
#include "BuilderToolBase.h"

class SelectTool : public BuilderToolBase {
private:
    int startCol = -1, startRow = -1;
    int endCol = -1, endRow = -1;
    bool isSelecting = false;

public:
    using BuilderToolBase::BuilderToolBase;

    //更新処理
    void Update(int cameraX, int cameraY, float scale);

    //描画処理
    void Draw(int cameraX, int cameraY, float scale);

    //選択範囲のリセット
    void Reset();

    //現在範囲選択されているか
    bool HasSelection() const;

    //選択範囲の開始・終了の最小/最大グリッド座標を取得
    void GetBounds(int& sCol, int& sRow, int& eCol, int& eRow) const;
};