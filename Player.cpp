#include "Player.h"
#include <cassert>
#include"GameScene.h"



void Player::Initialize(Model* model, Vector3 playerPosition) {
	assert(model);
	worldTransform_.Initialize();
	viewProjection_.Initialize();
	model_ = model;
	modelBullet_ = Model::CreateFromOBJ("pBullet", true);
	textureHandle_ = TextureManager::Load("./Resources/pShooter/pShooter.png");
	input_ = Input::GetInstance();
	worldTransform_.translation_ = Add(worldTransform_.translation_, playerPosition);
	worldTransform3DReticle_.Initialize();
	uint32_t textureReticle = TextureManager::Load("./Resources/Reticle.png");
	sprite2DReticle_ = Sprite::Create(textureReticle, {640, 360}, {255, 255, 255, 255}, {0.5f, 0.5f});
}

void Player::Update() {
	XINPUT_STATE joyState;



	//デスフラグの立った玉を削除
	bullets_.remove_if([](PlayerBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

	//キャラクターの移動ベクトル
	Vector3 move = {0, 0, 0};
	//キャラクターの移動速さ
	const float kCharacterSpeed = 0.2f;

	if (Input::GetInstance()->GetJoystickState(0, joyState)) {
		move.x += (float)joyState.Gamepad.sThumbLX / SHRT_MAX * kCharacterSpeed;
		move.y += (float)joyState.Gamepad.sThumbLY / SHRT_MAX * kCharacterSpeed;
	}
	worldTransform_.translation_ = Add(worldTransform_.translation_, move);

	//ImGui::Begin(" ");
	//ImGui::SliderFloat3("Player", &worldTransform_.translation_.x, -50.0f, 50.0f);
	//ImGui::End();

	//移動限界座標
	const float kMoveLimitX = 30;
	const float kMoveLimitY = 15;

	//範囲を超えない処理
	worldTransform_.translation_.x = max(worldTransform_.translation_.x, -kMoveLimitX);
	worldTransform_.translation_.x = min(worldTransform_.translation_.x, +kMoveLimitX);
	worldTransform_.translation_.y = max(worldTransform_.translation_.y, -kMoveLimitY);
	worldTransform_.translation_.y = min(worldTransform_.translation_.y, +kMoveLimitY);

	Rotate();


	Attack();

	for (PlayerBullet*bullet : bullets_) {
		bullet->Update();
	}

	//自機のワールド座標から3Dレティクルのワールド座標を計算
	{
		//自機から3Dレティクルへの距離
		const float kDistancePlayerTo3DReticle = 50.0f;
		//自機から3Dレティクルへのオフセット(Z+向き)
		Vector3 offset = {0, 0, kDistancePlayerTo3DReticle};
		{
			Vector2 spritePosition = sprite2DReticle_->GetPosition();
			//XINPUT_STATE joyState;
			// ジョイスティック状態取得
			if (Input::GetInstance()->GetJoystickState(0, joyState)) {
				spritePosition.x += (float)joyState.Gamepad.sThumbRX / SHRT_MAX * 10.0f;
				spritePosition.y -= (float)joyState.Gamepad.sThumbRY / SHRT_MAX * 10.0f;
				sprite2DReticle_->SetPosition(spritePosition);
			}

			Matrix4x4 matViewport = MakeViewportMatrix(0, 0, WinApp::kWindowWidth, WinApp::kWindowHeight, 0, 1);
			Matrix4x4 matVPV = Multiply4x4(viewProjection_.matView, Multiply4x4(viewProjection_.matProjection, matViewport));
			Matrix4x4 matInverseVPV = Inverse(matVPV);

			Vector3 posNear = Vector3(sprite2DReticle_->GetPosition().x, sprite2DReticle_->GetPosition().y, 0);
			Vector3 posFar = Vector3(sprite2DReticle_->GetPosition().x, sprite2DReticle_->GetPosition().y, 1);
			posNear = Transform(posNear, matInverseVPV);
			posFar = Transform(posFar, matInverseVPV);

			// マウスカーソルのスクリーン座標からワールド座標を取得
			const float kDistanceTestObject = 150.0f;
			worldTransform3DReticle_.translation_ = Add(posNear, Multiply(kDistanceTestObject, Normalize(Subtract(posFar, posNear))));
		}
		worldTransform3DReticle_.UpdateMatrix();
	}
	//3Dレティクルのワールド座標から2Dレティクルのスクリーン座標を計算
	{
		Vector3 positionReticle = worldTransform3DReticle_.translation_;
		//ビューポート行列
		Matrix4x4 matViewport = MakeViewportMatrix(0, 0, WinApp::kWindowWidth, WinApp::kWindowHeight, 0, 1);
		//ビュー行列とプロジェクション行列、ビューポート行列を合成する
		Matrix4x4 matViewProjectionViewport = Multiply4x4(Multiply4x4(viewProjection_.matView, viewProjection_.matProjection), matViewport);
		//ワールド→スクリーン座標変換
		positionReticle = Transform(positionReticle, matViewProjectionViewport);
		//スプライトのレティクルに座標設定
		sprite2DReticle_->SetPosition(Vector2(positionReticle.x, positionReticle.y));
	}

	worldTransform_.UpdateMatrix();
}

void Player::Draw(ViewProjection& viewProjection) {
	model_->Draw(worldTransform_, viewProjection, textureHandle_); 
	for (PlayerBullet* bullet : bullets_) {
		bullet->Draw(viewProjection);
	}
}

void Player::Rotate() {
	//回転速さ
	const float kRotSpeed = 0.02f;
	//押した方向で移動ベクトルを変更
	if (input_->PushKey(DIK_A)) {
		worldTransform_.rotation_.y -= kRotSpeed;
	} else if (input_->PushKey(DIK_D)) {
		worldTransform_.rotation_.y += kRotSpeed;
	}
}

void Player::Attack() {
	XINPUT_STATE joyState;

	// ゲームパッド未接続なら何もせずに抜ける
	if (!Input::GetInstance()->GetJoystickState(0, joyState)) {
		return;
	}

	// クールタイムの更新
	if (coolTime_ > 0.0f) {
		coolTime_ -= deltaTime; // deltaTimeはフレーム間の経過時間
		return;                 // クールタイム中なら弾を発射しない
	}

	if (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
		// 弾の速度
		const float kBulletSpeed = 1.0f;
		Vector3 velocity(0.0f, 0.0f, kBulletSpeed);
		velocity = TransformNormal(velocity, worldTransform_.matWorld_);
		// 自機から照準オブジェクトへのベクトル
		velocity = Subtract(GetReticleWorldPosition(), GetWorldPosition());
		velocity = Multiply(kBulletSpeed, Normalize(velocity));

		// 弾を生成し、初期化
		PlayerBullet* newBullet = new PlayerBullet();
		newBullet->Initialize(modelBullet_, GetWorldPosition(), velocity);

		// 弾を登録する
		bullets_.push_back(newBullet);

		// クールタイムを設定
		coolTime_ = coolTimeDuration_;
	}
}

void Player::OnCollision() {}

void Player::SetParent(const WorldTransform* parent) {
	//親子関係を結ぶ
	worldTransform_.parent_ = parent;
}

void Player::DrawUI() { 
	sprite2DReticle_->Draw(); 
}

Player::~Player() { 
	for (PlayerBullet* bullet : bullets_) {
		delete bullet;
	}
	delete sprite2DReticle_;
}

Vector3 Player::GetWorldPosition() {
	Vector3 WorldPos;//ワールド座標を入れる変数
	//ワールド行列の平行移動成分を取得(ワールド座標)
	WorldPos.x = worldTransform_.matWorld_.m[3][0];
	WorldPos.y = worldTransform_.matWorld_.m[3][1];
	WorldPos.z = worldTransform_.matWorld_.m[3][2];

	return WorldPos;
}

Vector3 Player::GetReticleWorldPosition() {
	Vector3 WorldPos; // ワールド座標を入れる変数
	// ワールド行列の平行移動成分を取得(ワールド座標)
	WorldPos.x = worldTransform3DReticle_.matWorld_.m[3][0];
	WorldPos.y = worldTransform3DReticle_.matWorld_.m[3][1];
	WorldPos.z = worldTransform3DReticle_.matWorld_.m[3][2];

	return WorldPos;
}