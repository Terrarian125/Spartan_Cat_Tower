//#pragma once
//#include "../Library/GameObject.h"
//#include "Stage.h"
//#include <vector>
//
//class StageEditor : public GameObject {
//public:
//    StageEditor(Stage* _stage);
//    virtual ~StageEditor();
//    void Update() override;
//    void Draw() override;
//
//private:
//    void UpdateInput();      //配置・削除の入力
//    void UpdateCamera();     //カメラの自由移動
//    void ShowImGuiWindow();  //ImGuiウィンドウの表示
//    void DrawGrid();         //格子の描画
//
//    Stage* stage;
//    bool isDebug = false;    //F1で切り替え
//    int selectedID = 2;      //現在選択中のタイルID
//    float camSpeed = 10.0f;  //カメラ移動速度
//
//    float zoomLevel = 1.0f;      //拡大率 (1.0 = 標準)
//    int targetReplaceID = 0;     //置換対象のID
//    int newReplaceID = 2;        //置換後のID
//};