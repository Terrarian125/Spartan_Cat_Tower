#include "Tutorial.h"
#include "../Library/Input.h"
#include "../Library/SceneManager.h"
#include "../Library/ObjectManager.h"
#include "../Library/GuiButton.h"
#include <cstdio>

TutorialScene::TutorialScene() : currentPage(0), btnPrev(nullptr), btnNext(nullptr)
{
    //画像のロード
    for (int i = 0; i <= 5; i++) {
        char path[256];
        sprintf_s(path, "Data/Image/Tutorial/Tutorial_%02d.png", i);

        int handle = LoadGraph(path);
        if (handle != -1) {
            imageHandles.push_back(handle);
        }
    }

    //ボタンの配置設定
    int btnW = 100;
    int btnH = 60;
    int btnY = (720 - btnH) / 2; // 上下中央

    //左ボタン
    btnPrev = new GuiButton(40, btnY, btnW, btnH, "<-");
    btnPrev->onClick = [this]() {
        if (this->currentPage > 0) {
            this->currentPage--;
        }
        };
    ObjectManager::Push(btnPrev);

    //右ボタン
    btnNext = new GuiButton(1280 - 40 - btnW, btnY, btnW, btnH, "->");
    btnNext->onClick = [this]() {
        if (this->currentPage < (int)this->imageHandles.size() - 1) {
            this->currentPage++;
        }
        };
    ObjectManager::Push(btnNext);

    //戻る
    auto bBack = new GuiButton(10, 620, btnW, btnH, "Back");
    //bBack->SetImage(btnImg_bBack);//ボタン画像を設定
    bBack->onClick = [this]() { SceneManager::ChangeScene("TITLE"); };
    ObjectManager::Push(bBack);
}

TutorialScene::~TutorialScene()
{
    //ロードした画像のメモリ解放
    for (int handle : imageHandles) {
        if (handle != -1) {
            DeleteGraph(handle);
        }
    }
    imageHandles.clear();

    //TitleSceneと同様、ObjectManagerの全滅を呼んでボタンを綺麗にする
    ObjectManager::DeleteAllGameObject();
}

void TutorialScene::Update()
{
    //ESCキーでタイトルへ戻る
    if (Input::IsKeyDown(KEY_INPUT_ESCAPE)) {
        SceneManager::ChangeScene("TITLE");
        return;
    }

    //ページの端に応じてボタンの表示・非表示を制御
    if (btnPrev) btnPrev->SetActive(currentPage > 0);
    if (btnNext) btnNext->SetActive(currentPage < (int)imageHandles.size() - 1);
}

void TutorialScene::Draw()
{
    //チュートリアル画像の描画
    if (currentPage >= 0 && currentPage < (int)imageHandles.size()) {
        int currentHandle = imageHandles[currentPage];
        if (currentHandle != -1) {
            DrawExtendGraph(0, 0, 1280, 720, currentHandle, FALSE);
        }
    }

    //ページ数の描画
    //DrawFormatString(1280 / 2 - 20, 680, GetColor(255, 255, 255), "%d / %d", currentPage + 1, (int)imageHandles.size());
}