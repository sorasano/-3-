#include "DxLib.h"

#define PI    3.1415926535897932384626433832795f

// ウィンドウのタイトルに表示する文字列
const char TITLE[] = "LC1B: ライトメア";

// ウィンドウ横幅
const int WIN_WIDTH = 64 * 14;

// ウィンドウ縦幅
const int WIN_HEIGHT = 64 * 14;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine,
				   _In_ int nCmdShow) {
	// ウィンドウモードに設定
	ChangeWindowMode(TRUE);

	// ウィンドウサイズを手動では変更させず、
	// かつウィンドウサイズに合わせて拡大できないようにする
	SetWindowSizeChangeEnableFlag(FALSE, FALSE);

	// タイトルを変更
	SetMainWindowText(TITLE);

	// 画面サイズの最大サイズ、カラービット数を設定(モニターの解像度に合わせる)
	SetGraphMode(WIN_WIDTH, WIN_HEIGHT, 32);

	// 画面サイズを設定(解像度との比率で設定)
	SetWindowSizeExtendRate(1.0);

	// 画面の背景色を設定する
	SetBackgroundColor(0x00, 0x00, 0x00);

	// DXlibの初期化
	if (DxLib_Init() == -1) { return -1; }

	// (ダブルバッファ)描画先グラフィック領域は裏面を指定
	SetDrawScreen(DX_SCREEN_BACK);

	// 画像などのリソースデータの変数宣言と読み込み

	int mirrorGH = LoadGraph("mirror.png");

	// ゲームループで使う変数の宣言

	//枠

	const int BLOCKSIZE = 64;
	int outX = BLOCKSIZE * 2;
	int outY = BLOCKSIZE * 2;

	int startX = outX;
	int startY = outY;

	int endX = WIN_WIDTH - outX;
	int endY = WIN_HEIGHT - outY;

	//自機

	int playerPosX = 0;
	int playerPosY = 0;
	int playerRadius = 32;

	//鏡

	//鏡の最大値
	const int MIRROR_MAX = 5;
	int mirrorMax = 0;//ステージごとに変える用

	int mirrorPosX[MIRROR_MAX] = {};
	int mirrorPosY[MIRROR_MAX] = {};

	int mirrorRadiusX = 32;//鏡半径
	int mirrorRadiusY = 32;

	int mirrorDirection[MIRROR_MAX] = {};//0↗ 1↘ 2↙ 3↖
	float mirrorAngle[MIRROR_MAX] = {};//鏡角度

	//光

	//光の最大個数
	const int LIGHT_MAX = 5;
	int lightMax = 1;

	//現在の座標保存用

	int lightStartPosX[LIGHT_MAX] = {};
	int lightStartPosY[LIGHT_MAX] = {};

	int lightEndPosX[LIGHT_MAX] = {};
	int lightEndPosY[LIGHT_MAX] = {};

	//光の当たっていない時の保存用変数

	int lightOldStartPosX[LIGHT_MAX] = {};
	int lightOldStartPosY[LIGHT_MAX] = {};

	int lightOldEndPosX[LIGHT_MAX] = {};
	int lightOldEndPosY[LIGHT_MAX] = {};

	int lightRadius = 32;

	int lightDirection[MIRROR_MAX + 1] = {};//0↑　1→　2↓　3←
	int lightOldDirection[MIRROR_MAX + 1] = {};

	int reset[LIGHT_MAX] = {};

	int lightTouch[LIGHT_MAX] = {};//光が当たっているか
	int lightTouchMirror[LIGHT_MAX] = {};//光に今当たっている鏡

	int lightContact = 0;//当たっている鏡の数

	//ステージ

	int stage = 0;
	int stageReset = 0;

	//ゴール

	int goalPosX = 0;
	int goalPosY = 0;

	int goalRadius = 32;

	// 最新のキーボード情報用
	char keys[256] = { 0 };

	// 1ループ(フレーム)前のキーボード情報
	char oldkeys[256] = { 0 };

	// ゲームループ
	while (true) {
		// 最新のキーボード情報だったものは1フレーム前のキーボード情報として保存
		for (int i = 0; i < 256; ++i) {
			oldkeys[i] = keys[i];
		}
		// 最新のキーボード情報を取得
		GetHitKeyStateAll(keys);

		// 画面クリア
		ClearDrawScreen();
		//---------  ここからプログラムを記述  ----------//

		// 更新処理

		//ステージ移動
		if (keys[KEY_INPUT_RETURN] == 1 && oldkeys[KEY_INPUT_RETURN] == 0) {
			stage++;
			stageReset = 0;
		}

		//ステージ移動時初期化
		if (stageReset == 0) {

			//ステージ1

			if (stage == 1) {

				stageReset = 1;

				//最初の光の座標
				lightStartPosX[0] = WIN_WIDTH / 2;
				lightStartPosY[0] = startY;
				lightEndPosX[0] = WIN_WIDTH / 2;
				lightEndPosY[0] = endY;

				//初期座標の保存
				lightOldStartPosX[0] = lightStartPosX[0];
				lightOldStartPosY[0] = lightStartPosY[0];
				lightOldEndPosX[0] = lightEndPosX[0];
				lightOldEndPosY[0] = lightEndPosY[0];

				//出てくる鏡の個数
				mirrorMax = 2;

				//鏡の初期値+最初の光の向き

				mirrorPosX[0] = WIN_WIDTH / 2 - BLOCKSIZE * 2;
				mirrorPosY[0] = WIN_HEIGHT / 2;
				mirrorDirection[0] = 0;

				mirrorPosX[1] = WIN_WIDTH / 2 + BLOCKSIZE * 2;
				mirrorPosY[1] = WIN_HEIGHT / 2;
				mirrorDirection[1] = 2;
		
				lightDirection[0] = 2;

				//ゴールの初期位置

				goalPosX = endX - goalRadius;
				goalPosY = endY - goalRadius;

			}
		}

		if (keys[KEY_INPUT_UP] == 1) {

			for (int i = 0; i < mirrorMax; i++) {
				mirrorPosY[i]--;
			}
		}
		if (keys[KEY_INPUT_DOWN] == 1) {
			for (int i = 0; i < mirrorMax; i++) {
				mirrorPosY[i]++;
			}
		}

		if (keys[KEY_INPUT_LEFT] == 1) {
			for (int i = 0; i < mirrorMax; i++) {
				mirrorPosX[i]--;
			}
		}
		if (keys[KEY_INPUT_RIGHT] == 1) {
			for (int i = 0; i < mirrorMax; i++) {
				mirrorPosX[i]++;
			}
		}

		//----------鏡------------

		//角度変更

		if (keys[KEY_INPUT_SPACE] == 1 && oldkeys[KEY_INPUT_SPACE] == 0) {

			for (int i = 0; i < mirrorMax; i++) {
				mirrorDirection[i]++;

				if (mirrorDirection[i] > 3) {
					mirrorDirection[i] = 0;
				}
			}

		}

		for (int i = 0; i < mirrorMax; i++) {
			//角度を変数に保存
			if (mirrorDirection[i] == 0) {
				mirrorAngle[i] = 0.0f;
			}
			else if (mirrorDirection[i] == 1) {
				mirrorAngle[i] = PI / 2;
			}
			else if (mirrorDirection[i] == 2) {
				mirrorAngle[i] = PI;
			}
			else if (mirrorDirection[i] == 3) {
				mirrorAngle[i] = PI + (PI / 2);
			}
		}

		//-----------光-----------

		//光の反射

		//鏡と光の当たり判定

		for (int i = 0; i < mirrorMax; i++) {
			for (int j = 0; j < lightMax; j++) {

				reset[j] = 1;

				//光と鏡の当たり判定

				if (lightStartPosX[j] - lightRadius < mirrorPosX[i] + mirrorRadiusX && mirrorPosX[i] - mirrorRadiusX < lightEndPosX[j] + lightRadius) {
					if (lightStartPosY[j] < mirrorPosY[i] + mirrorRadiusY && mirrorPosY[i] - mirrorRadiusY < lightEndPosY[j]) {

						DrawFormatString(0, 0, GetColor(255, 255, 255), "接触してるi = %d,j=%d", i, j, true);

						reset[j] = 0;

						//鏡に反射した後の向きを計測(鏡と光の角度から次の光の角度を設定)

						//0↗ 1↘ 2↙ 3↖(mirror)//0↑　1→　2↓　3←(light)

						if (lightDirection[j] == 0) {
							if (mirrorDirection[i] == 0) {
								lightDirection[j + 1] = 0;
							}
							else if (mirrorDirection[i] == 1) {
								lightDirection[j + 1] = 1;
							}
							else if (mirrorDirection[i] == 2) {
								lightDirection[j + 1] = 3;
							}
							else if (mirrorDirection[i] == 3) {
								lightDirection[j + 1] = 1;
							}
						}

						else if (lightDirection[j] == 1) {
							if (mirrorDirection[i] == 0) {
								lightDirection[j + 1] = 1;
							}
							else if (mirrorDirection[i] == 1) {
								lightDirection[j + 1] = 1;
							}
							else if (mirrorDirection[i] == 2) {
								lightDirection[j + 1] = 2;
							}
							else if (mirrorDirection[i] == 3) {
								lightDirection[j + 1] = 0;
							}
						}

						else if (lightDirection[j] == 2) {
							if (mirrorDirection[i] == 0) {
								lightDirection[j + 1] = 1;
							}
							else if (mirrorDirection[i] == 1) {
								lightDirection[j + 1] = 2;
							}
							else if (mirrorDirection[i] == 2) {
								lightDirection[j + 1] = 2;
							}
							else if (mirrorDirection[i] == 3) {
								lightDirection[j + 1] = 3;
							}
						}

						else if (lightDirection[j] == 3) {
							if (mirrorDirection[i] == 0) {
								lightDirection[j + 1] = 0;
							}
							else if (mirrorDirection[i] == 1) {
								lightDirection[j + 1] = 2;
							}
							else if (mirrorDirection[i] == 2) {
								lightDirection[j + 1] = 3;
							}
							else if (mirrorDirection[i] == 3) {
								lightDirection[j + 1] = 3;
							}
						}


						//---------1回目の反射-----------
						
						//0↗ 1↘ 2↙ 3↖(mirror)//0↑　1→　2↓　3←(light)

						if (j == 0) {

							//DrawFormatString(200, 0, GetColor(255, 255, 255), "通ってる", true);

							lightContact = 1;
							lightMax = 2;

							//光の向きが縦か横で次の光の向きを計算

							if (lightDirection[j] % 2 == 0) {

								if (lightTouch[j] == 0) {
									lightOldEndPosY[j] = lightEndPosY[j];
									lightOldStartPosX[j + 1] = lightStartPosX[j + 1];
									lightOldStartPosX[j + 1] = lightStartPosY[j + 1];
								}

								lightEndPosY[j] = mirrorPosY[i];
								lightStartPosX[j + 1] = lightStartPosX[j];
								lightStartPosY[j + 1] = mirrorPosY[i];

							}
							else {

								if (lightTouch[j] == 0) {
									lightOldEndPosX[j] = lightEndPosX[j];
									lightOldStartPosX[j + 1] = lightStartPosX[j + 1];
									lightOldStartPosY[j + 1] = lightStartPosY[j + 1];
								}

								lightEndPosX[j] = lightStartPosX[j];
								lightStartPosX[j + 1] = mirrorPosX[i];
								lightStartPosY[j + 1] = mirrorPosY[i];

							}


							//0↗ 1↘ 2↙ 3↖(mirror)//0↑　1→　2↓　3←(light)

							//次の光の向きから光を作成

							if (lightDirection[j + 1] == 0) {

								if (lightTouch[j] == 0) {
									lightOldEndPosX[j + 1] = lightEndPosX[j + 1];
								}

								lightEndPosX[j + 1] = lightStartPosX[j];
								lightEndPosY[j + 1] = startY;
							}
							else if (lightDirection[j + 1] == 1) {

								if (lightTouch[j] == 0) {
									lightOldEndPosX[j + 1] = lightEndPosX[j + 1];
									lightOldEndPosY[j + 1] = lightEndPosY[j + 1];
								}
								lightEndPosX[j + 1] = endX;
								lightEndPosY[j + 1] = mirrorPosY[i];
							}
							else if (lightDirection[j + 1] == 2) {

								if (lightTouch[j] == 0) {
									lightOldEndPosX[j + 1] = lightEndPosX[j + 1];
									lightOldEndPosY[j + 1] = lightEndPosY[j + 1];
								}

								lightEndPosX[j + 1] = lightStartPosX[j];
								lightEndPosY[j + 1] = endY;
							}
							else if (lightDirection[j + 1] == 3) {

								if (lightTouch[j] == 0) {
									lightOldEndPosX[j + 1] = lightEndPosX[j + 1];
									lightOldEndPosY[j + 1] = lightEndPosY[j + 1];
								}

								lightEndPosX[j + 1] = startX;
								lightEndPosY[j + 1] = mirrorPosY[i];
							}

							lightTouchMirror[j] = i;

							DrawFormatString(200, 20, GetColor(255, 255, 255), "[0]sx%d,sy%d,ex%d,ey%d", lightStartPosX[0], lightStartPosY[0], lightEndPosX[0], lightEndPosY[0], true);
							DrawFormatString(200, 40, GetColor(255, 255, 255), "[1]sx%d,sy%d,ex%d,ey%d", lightStartPosX[1], lightStartPosY[1], lightEndPosX[1], lightEndPosY[1], true);
						}

						//------2回目以降の反射---------

						else if(j > 0 && j < LIGHT_MAX && lightTouchMirror[j] != i){

							DrawFormatString(200, 0, GetColor(255, 255, 255), "通ってる %d",j, true);

							lightContact = j + 1;

							//光の向きが縦か横で次の光の始点を計算

							if (lightDirection[j] % 2 == 0) {

								if (lightTouch[j] == 0) {
									lightOldEndPosY[j] = lightEndPosY[j];
									lightOldStartPosX[j + 1] = lightStartPosX[j + 1];
									lightOldStartPosX[j + 1] = lightStartPosY[j + 1];
								}

								lightEndPosY[j] = mirrorPosY[i];
								lightStartPosX[j + 1] = lightEndPosX[j];
								lightStartPosY[j + 1] = mirrorPosY[i];

							}
							else {

								if (lightTouch[j] == 0) {
									lightOldEndPosX[j] = lightEndPosX[j];
									lightOldStartPosX[j + 1] = lightStartPosX[j + 1];
									lightOldStartPosY[j + 1] = lightStartPosY[j + 1];
								}

								lightEndPosX[j] = mirrorPosX[i];
								lightStartPosX[j + 1] = mirrorPosX[i];
								lightStartPosY[j + 1] = mirrorPosY[i];

							}

							DrawFormatString(200, 20, GetColor(255, 255, 255), "%d", lightDirection[j + 1], true);

							//0↗ 1↘ 2↙ 3↖(mirror)//0↑　1→　2↓　3←(light)

							//次の光の向きから光を作成

							if (lightDirection[j + 1] == 0) {

								if (lightTouch[j] == 0) {
									lightOldEndPosX[j + 1] = lightEndPosX[j + 1];
								}

								lightEndPosX[j + 1] = mirrorPosX[i];
								lightEndPosY[j + 1] = startY;
							}
							else if (lightDirection[j + 1] == 1) {

								if (lightTouch[j] == 0) {
									lightOldEndPosX[j + 1] = lightEndPosX[j + 1];
									lightOldEndPosY[j + 1] = lightEndPosY[j + 1];
								}
								lightEndPosX[j + 1] = endX;
								lightEndPosY[j + 1] = mirrorPosY[i];
							}
							else if (lightDirection[j + 1] == 2) {

								if (lightTouch[j] == 0) {
									lightOldEndPosX[j + 1] = lightEndPosX[j + 1];
									lightOldEndPosY[j + 1] = lightEndPosY[j + 1];
								}

								lightEndPosX[j + 1] = mirrorPosX[i];
								lightEndPosY[j + 1] = endY;
							}
							else if (lightDirection[j + 1] == 3) {

								if (lightTouch[j] == 0) {
									lightOldEndPosX[j + 1] = lightEndPosX[j + 1];
									lightOldEndPosY[j + 1] = lightEndPosY[j + 1];
								}

								lightEndPosX[j + 1] = startX;
								lightEndPosY[j + 1] = mirrorPosY[i];
							}

						}

						lightTouch[j] = 1;

					}

				}

				//入って出たときにリセットする

				if (reset[j] == 1 && lightTouch[j] == 1 && lightTouchMirror[j] == i) {

					lightStartPosX[j] = lightOldStartPosX[j];
					lightStartPosY[j] = lightOldStartPosY[j];

					lightEndPosX[j] = lightOldEndPosX[j];
					lightEndPosY[j] = lightOldEndPosY[j];

					lightStartPosX[j + 1] = lightOldStartPosX[j + 1];
					lightStartPosY[j + 1] = lightOldStartPosY[j + 1];

					lightEndPosX[j + 1] = lightOldEndPosX[j + 1];
					lightEndPosY[j + 1] = lightOldEndPosY[j + 1];

					lightTouch[j] = 0;

					lightContact = j;

					lightMax = j + 1;

					DrawFormatString(0, 300, GetColor(255, 255, 255), "通ってる", true);
				}

			}
		}


		//ゴール接触時

			for (int j = 0; j < lightMax + 1; j++) {

				if (lightStartPosX[j] - lightRadius < goalPosX + goalRadius && goalPosX - goalRadius < lightEndPosX[j] + lightRadius) {
					if (lightStartPosY[j] < goalPosY + goalRadius && goalPosY - goalRadius < lightEndPosY[j]) {

						stageReset = 0;

						DrawFormatString(0, 500, GetColor(255, 255, 255), "通ってる", true);

					}
				}

			}
		

		// 描画処理


		//枠

		for (int i = 0; i <= WIN_HEIGHT - (outY * 2); i += BLOCKSIZE) {
			DrawLine(startX, startY + i, endX, startY + i, GetColor(255, 255, 255), true);
		}
		for (int i = 0; i <= WIN_WIDTH - (outX * 2); i += BLOCKSIZE) {
			DrawLine(startX + i, startY, startX + i, endY, GetColor(255, 255, 255), true);
		}

		//光

		for (int i = 0; i <= lightContact; i++) {

			if (lightDirection[i] % 2 == 0) {
				DrawBox(lightStartPosX[i] - lightRadius, lightStartPosY[i], lightEndPosX[i] + lightRadius, lightEndPosY[i], GetColor(255, 255, 255), true);
			}
			else {
				DrawBox(lightStartPosX[i], lightStartPosY[i] - lightRadius, lightEndPosX[i], lightEndPosY[i] + lightRadius, GetColor(255, 255, 255), true);
			}

		}

		//鏡

		for (int i = 0; i < mirrorMax; i++) {
			DrawRotaGraph(mirrorPosX[i], mirrorPosY[i], 1.0, mirrorAngle[i], mirrorGH, TRUE);
		}

		DrawFormatString(0, 20, GetColor(255, 255, 255), "%d", mirrorDirection[0], true);
		DrawFormatString(0, 40, GetColor(255, 255, 255), "%d", mirrorDirection[1], true);
		DrawFormatString(60, 60, GetColor(255, 255, 255), "%d", lightTouch[0], true);

		//ゴール

		DrawBox(goalPosX + goalRadius, goalPosY + goalRadius, goalPosX - goalRadius, goalPosY - goalRadius, GetColor(0, 0, 255), true);

		//---------  ここまでにプログラムを記述  ---------//
		// (ダブルバッファ)裏面
		ScreenFlip();

		// 20ミリ秒待機(疑似60FPS)
		WaitTimer(20);

		// Windowsシステムからくる情報を処理する
		if (ProcessMessage() == -1) {
			break;
		}

		// ESCキーが押されたらループから抜ける
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) {
			break;
		}
	}
	// Dxライブラリ終了処理
	DxLib_End();

	// 正常終了
	return 0;
}
