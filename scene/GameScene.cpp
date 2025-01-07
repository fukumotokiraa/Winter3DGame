#include "GameScene.h"
#include "AxisIndicator.h"
#include "TextureManager.h"
#include "fstream"
#include <cassert>

GameScene::GameScene() {}

GameScene::~GameScene() {
	delete player_;
	delete model_;
	delete modelEnemy_;
	delete debugCamera_;
	// delete enemy_;
	delete skydome_;
	delete modelSkydome_;
	delete railCamera_;
	for (EnemyBullet* enemyBullet : enemyBullets_) {
		delete enemyBullet;
	}
	for (Enemy* enemy : enemies_) {
		delete enemy;
	};
}

void GameScene::Initialize() {
	worldTransform_.Initialize();
	// textureHandle_ = TextureManager::Load("uvChecker.png");
	model_ = Model::CreateFromOBJ("pShooter", true);
	viewProjection_.Initialize();
	railCamera_ = new RailCamera();
	railCamera_->Initialize(worldTransform_);
	TextureManager::Load("./resources/Reticle.png");
	player_ = new Player();
	Vector3 playerPosition(0, 0, 20);
	player_->Initialize(model_, playerPosition);
	debugCamera_ = new DebugCamera(1280, 720);
	modelEnemy_ = Model::CreateFromOBJ("eShooter", true);
	skydome_ = new Skydome();
	modelSkydome_ = Model::CreateFromOBJ("Skydome", true);
	skydome_->Initialize(modelSkydome_, &viewProjection_);

	uint32_t GameoverTexture = TextureManager::Load("./resources/Scene/Gameover.png");
	spriteGameover_ = Sprite::Create(GameoverTexture, {0, 0}, {255, 255, 255, 255}, {0, 0});
	uint32_t ClearTexture = TextureManager::Load("./resources/Scene/Clear.png");
	spriteClear_ = Sprite::Create(ClearTexture, {0, 0}, {255, 255, 255, 255}, {0, 0});
	uint32_t TitleTexture = TextureManager::Load("./resources/Scene/Title.png");
	spriteTitle_ = Sprite::Create(TitleTexture, {0, 0}, {255, 255, 255, 255}, {0, 0});

	bgm = Audio::GetInstance()->LoadWave("./resources/bgm.mp3");

	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();

	GameTime_ = 2000;
	enemyAlive = 8;
	playerLife = 3;
	LoadEnemyPopData();
	// GenerateEnemy({0,10,100});

	// 自キャラとレールカメラの親子関係を結ぶ
	player_->SetParent(&railCamera_->GetWorldTransform());

	//AxisIndicator::GetInstance()->SetVisible(true);
	//AxisIndicator::GetInstance()->SetTargetViewProjection(&viewProjection_);

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();
}

void GameScene::Update() {
	XINPUT_STATE joyState;
	DWORD dwResult = XInputGetState(0, &joyState);
	if (dwResult == ERROR_SUCCESS) {
		// コントローラーが接続されている場合
		// ゲームクリアまたはゲームオーバー中の処理
		if (isGameOver_ || isGameClear_) {

			if (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
				// Aボタンでタイトル画面へ戻る
				TitleFr = false;
				isGameOver_ = false;
				isGameClear_ = false;

				Initialize();
				return;
			}
		}
		if (playing == -1) {
			playing = (uint32_t)Audio::GetInstance()->PlayWave(bgm, true, 0.2f);
		}
		if (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
			// Aボタンが押された場合
			TitleFr = true;
		}
	} else {
		// コントローラーが接続されていない場合
		TitleFr = false;
	}
	if (TitleFr) {
		GameTime_--;
		player_->Update();
		debugCamera_->Update();
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}
		skydome_->Update();
		for (EnemyBullet* bullet : enemyBullets_) {
			bullet->Update();
		}
		enemies_.remove_if([](Enemy* enemy) {
			if (enemy->IsDead()) {
				delete enemy;
				return true;
			}
			return false;
		});
		enemyBullets_.remove_if([](EnemyBullet* bullet) {
			if (bullet->IsDead()) {
				delete bullet;
				return true;
			}
			return false;
		});

		UpdateEnemyPopCommands();

		    // ゲームクリアまたはゲームオーバーの判定
		if (enemyAlive <= 0) {
			Audio::GetInstance()->StopWave(playing);
			playing = -1;
			isGameClear_ = true;
		} else if (playerLife <= 0 || GameTime_ <= 0) {
			Audio::GetInstance()->StopWave(playing);
			playing = -1;
			isGameOver_ = true;
		}
#ifdef _DEBUG
		if (input_->TriggerKey(DIK_1)) {
			if (isDebugCameraActive_) {
				isDebugCameraActive_ = false;
			} else {
				isDebugCameraActive_ = true;
			}
		}
#endif // DEBUG
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			viewProjection_.matView = debugCamera_->GetViewMatrix();
			viewProjection_.matProjection = debugCamera_->GetProjectionMatrix();
			// ビュープロジェクション行列の転送
			viewProjection_.TransferMatrix();
		} else {
			//// ビュープロジェクション行列の更新と転送
			// viewProjection_.UpdateMatrix();
			railCamera_->Update();
			viewProjection_.matView = railCamera_->GetViewMatrix();
			viewProjection_.matProjection = railCamera_->GetProjectionMatrix();

			viewProjection_.TransferMatrix();
		}
		CheckAllCollisions();
	}
	
}

void GameScene::Draw() {

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>
	player_->Draw(viewProjection_);
	// enemy_->Draw(viewProjection_);
	skydome_->Draw();


	for (EnemyBullet* bullet : enemyBullets_) {
		bullet->Draw(viewProjection_);
	}
	for (Enemy* enemy : enemies_) {
		enemy->Draw(viewProjection_);
	}

	// 3Dオブジェクト描画後処理
	Model::PostDraw();

#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>
	//player_->DrawUI();


    if (!TitleFr) {
		spriteTitle_->Draw(); // タイトル画面を描画
	} else if (isGameOver_) {
		spriteGameover_->Draw(); // ゲームオーバー画面を描画
	} else if (isGameClear_) {
		spriteClear_->Draw(); // ゲームクリア画面を描画
	} else {
		player_->DrawUI(); // ゲームプレイ中のUIを描画
	}

	// スプライト描画後処理
	Sprite::PostDraw();
#pragma endregion
}

void GameScene::CheckAllCollisions() {
	// 判定対象AとBの座標
	Vector3 posA, posB;

	// 自弾リストの取得
	const std::list<PlayerBullet*>& playerBullets = player_->GetBullet();
	// 敵弾リストの取得
	const std::list<EnemyBullet*>& enemyBullets = GetEnemyBullet();
	// 敵リストの取得
	const std::list<Enemy*>& enemies = GetEnemy();

#pragma region 自キャラと敵弾の当たり判定
	posA = player_->GetWorldPosition();

	for (EnemyBullet* bullet : enemyBullets) {
		posB = bullet->GetWorldPosition();

		float Distance = ((posB.x - posA.x) * (posB.x - posA.x)) + ((posB.y - posA.y) * (posB.y - posA.y)) + ((posB.z - posA.z) * (posB.z - posA.z));
		float ARadius = 1.0f;
		float BRadius = 1.0f;

		if (Distance <= (ARadius + BRadius) * (ARadius + BRadius)) {
			// 衝突時コールバックを呼び出す
			player_->OnCollision();
			bullet->OnCollision();
			playerLife--;
		}
	}
#pragma endregion

#pragma region 自弾と敵キャラの当たり判定
	for (Enemy* enemy : enemies) {
		posA = enemy->GetWorldPosition();
		for (PlayerBullet* bullet : playerBullets) {
			posB = bullet->GetWorldPosition();

			float Distance = ((posB.x - posA.x) * (posB.x - posA.x)) + ((posB.y - posA.y) * (posB.y - posA.y)) + ((posB.z - posA.z) * (posB.z - posA.z));
			float ARadius = 1.0f;
			float BRadius = 1.0f;

			if (Distance <= (ARadius + BRadius) * (ARadius + BRadius)) {
				// 衝突時コールバックを呼び出す
				enemy->OnCollision();
				bullet->OnCollision();
				enemyAlive--;
			}
		}
	}
#pragma endregion

#pragma region 自弾と敵弾の当たり判定
	for (PlayerBullet* playerBullet : playerBullets) {
		posA = playerBullet->GetWorldPosition();
		for (EnemyBullet* enemyBullet : enemyBullets) {
			posB = enemyBullet->GetWorldPosition();

			float Distance = ((posB.x - posA.x) * (posB.x - posA.x)) + ((posB.y - posA.y) * (posB.y - posA.y)) + ((posB.z - posA.z) * (posB.z - posA.z));
			float ARadius = 1.0f;
			float BRadius = 1.0f;

			if (Distance <= (ARadius + BRadius) * (ARadius + BRadius)) {
				// 衝突時コールバックを呼び出す
				playerBullet->OnCollision();
				enemyBullet->OnCollision();
			}
		}
	}
#pragma endregion
}

void GameScene::AddEnemyBullet(EnemyBullet* enemyBullet) {
	// リストに登録する
	enemyBullets_.push_back(enemyBullet);
}

void GameScene::GenerateEnemy(const Vector3 position) {
	// 敵を生成し、初期化
	Enemy* newEnemy = new Enemy();
	newEnemy->Initialize(modelEnemy_, position);
	newEnemy->SetPlayer(player_);
	newEnemy->SetGameScene(this);
	// 敵を登録する
	enemies_.push_back(newEnemy);
}

void GameScene::LoadEnemyPopData() {
	enemyPopCommands.clear();
	enemyPopCommands.str("");

	// ファイルを開く
	std::ifstream file;
	file.open("./Resources/enemyPop.csv");
	assert(file.is_open());

	// ファイルの内容を文字列ストリームにコピー
	enemyPopCommands << file.rdbuf();

	// ファイルを閉じる
	file.close();
}

void GameScene::UpdateEnemyPopCommands() {
	// 待機処理
	if (isWait_) {
		waitTimer_--;
		if (waitTimer_ <= 0) {
			// 待機完了
			isWait_ = false;
		}
		return;
	}

	// 1行分の文字列を入れる変数
	std::string line;

	// コマンド実行をループ
	while (std::getline(enemyPopCommands, line)) {
		// 1行分の文字列をストリームに変換して解析しやすくする
		std::istringstream line_stream(line);

		std::string word;
		//,区切りで行の先頭文字列を取得
		std::getline(line_stream, word, ',');

		// "//"から始まる行はコメント
		if (word.find("//") == 0) {
			// コメント行を飛ばす
			continue;
		}

		// POPコマンド
		if (word.find("POP") == 0) {
			// x座標
			std::getline(line_stream, word, ',');
			float x = (float)std::atof(word.c_str());

			// y座標
			std::getline(line_stream, word, ',');
			float y = (float)std::atof(word.c_str());

			// z座標
			std::getline(line_stream, word, ',');
			float z = (float)std::atof(word.c_str());

			// 敵を発生させる
			GenerateEnemy(Vector3(x, y, z));
		}
		// WAITコマンド
		else if (word.find("WAIT") == 0) {
			std::getline(line_stream, word, ',');

			// 待ち時間
			int32_t waitTime = atoi(word.c_str());

			// 待機開始
			isWait_ = true;
			waitTimer_ = waitTime;

			// コマンドループを抜ける
			break;
		}
	}
}