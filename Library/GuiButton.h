#pragma once
#include "GameObject.h"
#include <functional>
#include <string>

///<summary>
///簡単に使える、クリック可能なGUIボタン
///位置、大きさ、ラベルテキストを指定して生成します
///ボタン画像も設定可能です、無ければデフォルトの四角形ボタンになります
///onClickに関数を登録して、クリック時の処理を設定します
///マウスオーバーとフォーカスでハイライト表示します
///アクティブ/非アクティブの切り替えも可能です
///フォーカス時に説明文を表示する機能も追加予定
///以下、使用例です：--------------------------------------
////　int bx = 100, //ボタンのX座標
////　by = 300, //ボタンのY座標
////　bw = 400,//ボタンの幅
////　bh = 90,  //ボタンの高さ
////　bi = 100; //ボタン間隔
////
////　int btnImg_new = LoadGraph("data/image/btnImg_new.png");
////　int btnImg_tutorial = LoadGraph("data/image/btnImg_tutorial.png");
////
////　auto bNew = new GuiButton(bx, by, bw, bh, "ゲームスタート！");
////　bNew->SetImage(btnImg_new);//ボタン画像を設定、なければデフォルトの四角形ボタンになる
////　bNew->onClick = []() { SceneManager::ChangeScene("PLAY"); };//クリック時にシーンを変更する例
////　buttons.push_back(bNew);
////
////　auto bSet = new GuiButton(bx, by + bi, bw, bh, "設定");
////　bSet->onClick = [this]() { this->mySettingPanel->SetVisible(true); };//設定パネルを表示する例
////　buttons.push_back(bSet);
///------------------------------------------------------------
///作成：240169 GE2A33 髙橋
///</summary>

class GuiButton : public GameObject {
public:
    std::function<void()> onClick;

    GuiButton(int x, int y, int w, int h, std::string text);
    void Update() override;
    void Draw() override;

	void SetFocus(bool f) { isFocused = f; }//フォーカスされているかどうかを設定する関数
	void SetActive(bool a) { active = a; }//アクティブかどうかを設定する関数
	bool IsActive() const { return active; }//アクティブかどうかを取得する関数
	bool IsMouseOver();//マウスオーバーしているかを判定する関数

	void SetDescription(std::string desc) { description = desc; }//説明文を設定するための関数
	std::string GetDescription() const { return description; }//説明文を取得するための関数

    //画像を設定するための関数を追加
    void SetImage(int graphHandle) { imageHandle = graphHandle; }
private:
    int x, y, width, height;
    unsigned int color, hoverCol;
    std::string label;
    bool isFocused;
    bool active;
	std::string description;//説明文用
    int imageHandle = -1; //画像のハンドル（初期値は-1 = 画像なし）
};