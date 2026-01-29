#pragma once
#include "../Library/SceneBase.h"

class PlayScene : public SceneBase {
public:
    PlayScene();
    virtual ~PlayScene();

    // シーン開始時に一度だけ呼ばれる
    virtual void Init();

    // SceneManagerから毎フレーム呼ばれる
    virtual void Update() override;
    virtual void Draw() override;
};