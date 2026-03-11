#include "TitleScene.h"
#include <DxLib.h>
#include "../Library/Input.h"
#include "../Library/SceneManager.h"
#include "../Library/GuiButton.h"
#include "../Library/GameSetting.h"
#include "SettingPanel.h"

TitleScene::TitleScene() : currentSelect(0), isExitDialogVisible(false) {
    LogoBg = LoadGraph("data/Movie/Bg.mp4");
    if (LogoBg != -1) {
        PlayMovieToGraph(LogoBg, 1);
    }

    Logo = LoadGraph("data/image/Title.png");
    Op_Music = LoadSoundMem("Data/Music/OP.mp3");

    //設定の読み込みと適用
    GameSetting::Load();
    GameSetting::Apply(Op_Music);
    PlaySoundMem(Op_Music, DX_PLAYTYPE_LOOP);

    mySettingPanel = new SettingPanel();

    //メインメニューのボタン配置
	int bx = 50, //ボタンのX座標
		by = 300, //ボタンのY座標
		bw = 400,//ボタンの幅
		bh = 90,  //ボタンの高さ
	    bi = 100; //ボタン間隔

    int btnImg_bNew = LoadGraph("data/image/btnImg_bNew.png");
	int btnImg_bTutorial = LoadGraph("data/image/btnImg_bTutorial.png"); 
    int btnImg_bSet = LoadGraph("data/image/btnImg_bSet.png");
    int btnImg_bExit = LoadGraph("data/image/btnImg_bExit.png");

    auto bNew = new GuiButton(bx, by, bw, bh, "ゲームスタート！");
	bNew->SetImage(btnImg_bNew);//ボタン画像を設定、なければデフォルトの四角形ボタンになる
    bNew->onClick = []() { SceneManager::ChangeScene("STAGE"); };
    buttons.push_back(bNew);

    auto bTutorial = new GuiButton(bx, by + bi, bw, bh, "チュートリアル");
	bTutorial->SetImage(btnImg_bTutorial);//ボタン画像を設定
    bTutorial->onClick = []() { SceneManager::ChangeScene("TUTORIAL"); };
    buttons.push_back(bTutorial);

    auto bSet = new GuiButton(bx, by+bi*2, bw, bh, "設定");
    bSet->SetImage(btnImg_bSet);//ボタン画像を設定
    bSet->onClick = [this]() { this->mySettingPanel->SetVisible(true); };
    buttons.push_back(bSet);

    auto bExit = new GuiButton(bx, by+bi*3, bw, bh, "ゲーム終了");
    bExit->SetImage(btnImg_bExit);//ボタン画像を設定
    bExit->onClick = [this]() {
        this->isExitDialogVisible = true;
        for (auto b : this->exitButtons) b->SetActive(true);
        this->currentSelect = 1; //ダイアログ内での初期選択をYESに
        };
    buttons.push_back(bExit);

    //終了確認ダイアログ用のボタン配置
    int cx = 1280 / 2;
    int cy = 720 / 2;

    auto bYes = new GuiButton(cx - 160, cy + 50, 150, 50, "はい");
    bYes->onClick = []() { SceneManager::Exit(); };
    bYes->SetActive(false); //初期状態は非アクティブ
    exitButtons.push_back(bYes);

    auto bNo = new GuiButton(cx + 10, cy + 50, 150, 50, "いいえ");
    bNo->onClick = [this]() {
        this->isExitDialogVisible = false;
        for (auto b : this->exitButtons) b->SetActive(false);
        };
    bNo->SetActive(false); //初期状態は非アクティブ
    exitButtons.push_back(bNo);
}

void TitleScene::Update() {
	//動画更新
    if (LogoBg != -1) {
        //GetMovieStateToGraph が 1 を返している間は再生中
        if (GetMovieStateToGraph(LogoBg) == 1) {
			UpdateMovieToGraph(LogoBg);
		}
	}
    //終了確認ダイアログ表示中
    if (isExitDialogVisible) {
        for (auto b : exitButtons) b->Update();

        //ダイアログ内のキー操作 (左右でYES/NO切り替え)
        if (Input::IsKeyDown(KEY_INPUT_LEFT))  currentSelect = 0;
        if (Input::IsKeyDown(KEY_INPUT_RIGHT)) currentSelect = 1;

        for (int i = 0; i < (int)exitButtons.size(); i++) {
            exitButtons[i]->SetFocus(i == currentSelect);
            if (exitButtons[i]->IsMouseOver()) currentSelect = i;
        }
        return;
    }

    //設定パネル表示中
    if (mySettingPanel->IsVisible()) {
        mySettingPanel->Update();
        GameSetting::Apply(Op_Music);
        return;
    }

    //通常時（メインメニュー）
    for (auto b : buttons) {
        b->Update();
    }

    //メインメニューのキー操作 (上下)
    if (Input::IsKeyDown(KEY_INPUT_DOWN)) currentSelect = (currentSelect + 1) % buttons.size();
    if (Input::IsKeyDown(KEY_INPUT_UP))   currentSelect = (currentSelect - 1 + (int)buttons.size()) % (int)buttons.size();

    for (int i = 0; i < (int)buttons.size(); i++) {
        buttons[i]->SetFocus(i == currentSelect);
        if (buttons[i]->IsMouseOver()) currentSelect = i;
    }
}

void TitleScene::Draw() {
    //基本背景
    //PlayMovieToGraph(LogoBg, DX_PLAYTYPE_LOOP);
    DrawExtendGraph(0, 0, 1280, 720, LogoBg, FALSE);
    DrawGraph(100, 50, Logo, TRUE);

    //メインメニューボタン
    for (auto b : buttons) b->Draw();

    //設定パネル
    mySettingPanel->Draw();

    //終了確認ダイアログの最前面描画
    if (isExitDialogVisible) {
        //背景を暗くして操作不能感を出す
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
        DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        //ダイアログ外枠
        int vx = 440, vy = 260, vw = 400, vh = 200;
        DrawBox(vx, vy, vx + vw, vy + vh, GetColor(20, 20, 40), TRUE);
        DrawBox(vx, vy, vx + vw, vy + vh, GetColor(255, 255, 255), FALSE);

		DrawFormatString(vx + 95, vy + 50, GetColor(255, 255, 255), "本当にゲームを終了しますか？");

        //YES/NOボタン
        for (auto b : exitButtons) b->Draw();
    }
}

TitleScene::~TitleScene() {
    StopSoundMem(Op_Music);
    DeleteSoundMem(Op_Music);
    DeleteGraph(LogoBg);
    DeleteGraph(Logo);

    //メモリ解放
    delete mySettingPanel;
}