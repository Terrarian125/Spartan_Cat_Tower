#include "GameListScene.h"
#include <DxLib.h>
#include "../Library/Input.h"
#include <algorithm> // std::sort のために必要
#include <sstream>   // 日付表示の整形のために必要
#include <cstdlib>   // system() のために必要
#include <iomanip>   // 日付表示の整形のために必要
#include <ctime>     // time_t, localtime_s のために必要
#include <cassert>   // assert のために必要


// GameListScene::Draw() で使用するヘルパー関数（再掲）
// localtime_s を使用してセキュリティ警告を解消しています
std::string formatTime(time_t t) {
    if (t == 0) return "---";

    std::tm tm_struct;
    errno_t err = localtime_s(&tm_struct, &t);

    if (err != 0) return "Error";

    std::stringstream ss;
    ss << std::put_time(&tm_struct, "%Y/%m/%d");
    return ss.str();
}

GameListScene::GameListScene()
{
    // 画像リソースの読み込み
    Bg = LoadGraph("data/image/Bg.png");
    UI_Back = LoadGraph("data/image/UI/UI_Q_back.png");
	UI_Wait = LoadGraph("data/image/UI/UI_Wait.png");
	UI_Play = LoadGraph("data/image/UI/UI_Play.png");

    // リソースの読み込みチェック
    assert(Bg != -1);
    assert(UI_Back != -1);
	assert(UI_Wait != -1);
	assert(UI_Play != -1);

    //リスト表示テスト用 サンプルデータ
    int blockIcon = LoadGraph("data/image/Icon/block_icon.png");
    int rogueIcon = LoadGraph("data/image/Icon/rogue_icon.png");
	int LabyrinthIcon = LoadGraph("data/image/Icon/Labyrinth_icon.png");

    // 1. ゲームA: Block.exe
    GameInfo gameA;
    gameA.commandName = "game_a";
    gameA.displayName = "ブロック崩し";
    // 実行ファイルパス
    gameA.execPath = "Data\\Games\\Block.exe";
    gameA.iconHandle = blockIcon;
    gameA.registrationDate = time(nullptr) - 86400 * 5;
    gameA.lastPlayedDate = time(nullptr) - 86400 * 2;
    gameList.push_back(gameA);

    // 2. ゲームB: Rogue.exe
    GameInfo gameB;
    gameB.commandName = "game_b";
    gameB.displayName = "ローグライク RPG";
    // 実行ファイルパス
    gameB.execPath = "Data\\Games\\Rogue.exe";
    gameB.iconHandle = rogueIcon;
    gameB.registrationDate = time(nullptr) - 86400 * 10;
    gameB.lastPlayedDate = time(nullptr); // 今日プレイしたことに
    gameList.push_back(gameB);

    // 3. ゲームC: Labyrinth.exe
    GameInfo gameC;
    gameC.commandName = "game_c";
    gameC.displayName = "迷路";
    // 実行ファイルパス
    gameC.execPath = "Data\\Games\\Labyrinth.exe";
    gameC.iconHandle = LabyrinthIcon;
    gameC.registrationDate = time(nullptr) - 86400 * 10;
    gameC.lastPlayedDate = time(nullptr); // 今日プレイしたことに
    gameList.push_back(gameC);

    selectedIndex = 0;
    currentSortMode = 0; // 初期ソートモードは名前順
}

GameListScene::~GameListScene()
{
    // リソースの解放
    DeleteGraph(Bg);
    DeleteGraph(UI_Back);
	DeleteGraph(UI_Wait);
	DeleteGraph(UI_Play);

    // アイコンの解放処理（重複を考慮）
    std::vector<int> uniqueIcons;
    for (const auto& info : gameList) {
        if (info.iconHandle != -1) {
            bool found = false;
            for (int handle : uniqueIcons) {
                if (handle == info.iconHandle) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                uniqueIcons.push_back(info.iconHandle);
            }
        }
    }

    for (int handle : uniqueIcons) {
        DeleteGraph(handle);
    }
    gameList.clear();
}

void GameListScene::Update()
{
    // Qキーが押されたらBUTTONシーンに戻る
    if (Input::IsKeyUP(KEY_INPUT_Q)) {
        SceneManager::ChangeScene("BUTTON");
    }

    if (!gameList.empty()) {
        int listSize = (int)gameList.size();

        //操作
        if (Input::IsKeyDown(KEY_INPUT_DOWN) || Input::IsKeyDown(KEY_INPUT_S)) {
            selectedIndex++;
            if (selectedIndex >= listSize) {
                selectedIndex = 0;
            }
        }

        if (Input::IsKeyDown(KEY_INPUT_UP) || Input::IsKeyDown(KEY_INPUT_W)) {
            selectedIndex--;
            if (selectedIndex < 0) {
                selectedIndex = listSize - 1;
            }
        }

        // ソートモード切り替え (Rキー)
        if (Input::IsKeyDown(KEY_INPUT_R)) {
            currentSortMode = (currentSortMode + 1) % 3; // 0, 1, 2 を循環

            // リストを新しいソートモードで並び替える
            std::sort(gameList.begin(), gameList.end(),
                [this](const GameInfo& a, const GameInfo& b) {
                    if (currentSortMode == 0) {
                        // モード 0: 名前順 (昇順)
                        return a.displayName < b.displayName;
                    }
                    else if (currentSortMode == 1) {
                        // モード 1: 登録日順 (新しいものが上 = 降順)
                        return a.registrationDate > b.registrationDate;
                    }
                    else { // currentSortMode == 2
                        // モード 2: 最終プレイ日順 (新しいものが上 = 降順)
                        return a.lastPlayedDate > b.lastPlayedDate;
                    }
                }
            );
            // ソート後、選択カーソルをリストの先頭に戻す
            selectedIndex = 0;
        }


        //ゲーム起動
        if (Input::IsKeyDown(KEY_INPUT_RETURN) || Input::IsKeyDown(KEY_INPUT_SPACE)) {
            const GameInfo& selectedGame = gameList[selectedIndex];

            // CUIゲームのウィンドウを維持するため、cmd /k で実行ファイルをラップ
            std::string command = "cmd /k \"";
            command += selectedGame.execPath;
            command += "\"";

            // 外部ゲームを起動！
            int result = std::system(command.c_str());

            if (result == 0) {
                // 起動成功: 最終プレイ日を更新する
                // gameList[selectedIndex] は const 参照ではないため、更新可能らしい
                gameList[selectedIndex].lastPlayedDate = time(nullptr);
            }
            else {
                // 起動失敗: エラーメッセージを表示する処理
                DrawFormatString(100, 100, GetColor(255, 0, 0), "起動失敗: %s", selectedGame.execPath.c_str());
            }
        }
    }
}
void GameListScene::Draw()
{
    DrawGraph(0, 0, Bg, TRUE);

    if ((Input::IsKeyDown(KEY_INPUT_RETURN))) {
        DrawGraph(0, 0, UI_Wait, TRUE);
    }

    // === リスト描画の開始位置と行間 ===
    int startY = 100;
    int lineSpacing = 40;
    int listWidth = 800;

    // ヘッダーの描画
    int headerColor = GetColor(150, 255, 150);
    DrawString(100, 60, "アイコン", headerColor);
    DrawString(200, 60, "ゲーム名", headerColor);
    DrawString(450, 60, "実行ファイル名", headerColor);
    DrawString(650, 60, "登録日", headerColor);
    DrawString(800, 60, "最終プレイ日", headerColor);
    DrawLine(100, 85, 100 + listWidth, 85, headerColor);

    //現在のソートモードを表示
    const char* sortModeNames[] = { "ゲーム名順", "登録日順", "最終プレイ日順" };
    int sortColor = GetColor(200, 200, 255);
    DrawFormatString(900, 30, sortColor, "ソート: %s (Rキー)", sortModeNames[currentSortMode]);


    // リストアイテムの描画
    for (size_t i = 0; i < gameList.size(); ++i) {
        const GameInfo& info = gameList[i];
        int y = startY + i * lineSpacing;

        // 選択されている行の色を変更
        int drawColor = (i == selectedIndex) ? GetColor(255, 255, 0) : GetColor(255, 255, 255);

        // 選択行の場合はハイライトボックスを描画
        if (i == selectedIndex) {
            DrawBox(90, y - 5, 90 + listWidth + 10, y + lineSpacing - 10, GetColor(50, 50, 100), TRUE);
        }

        // ｜アイコン｜
        if (info.iconHandle != -1) {
            DrawGraph(100, y, info.iconHandle, TRUE);
        }
        else {
            DrawString(100, y, "[No Icon]", GetColor(150, 150, 150));
        }

        // ｜ゲーム名｜
        DrawString(200, y, info.displayName.c_str(), drawColor);

        // ｜実行ファイル名｜
        DrawString(450, y, info.execPath.c_str(), drawColor);

        // ｜登録日｜
        DrawString(650, y, formatTime(info.registrationDate).c_str(), drawColor);

        // ｜最終プレイ日｜
        DrawString(800, y, formatTime(info.lastPlayedDate).c_str(), drawColor);
    }

    DrawGraph(800, 550, UI_Back, TRUE);
    DrawGraph(50, 600, UI_Play, TRUE);
}