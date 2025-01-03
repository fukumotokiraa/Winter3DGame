#include "PlayerBullet.h"
#include"cassert"

void PlayerBullet::Initialize(Model* model, const Vector3& positon, const Vector3& velocity) {
	//NULLポインタチェック
	assert(model);

	model_ = model;
	//テクスチャ読み込み
	textureHandle_ = TextureManager::Load("./Resources/pBullet/bullet.png");

	worldTransform_.Initialize();
	//引数で受け取った初期座標をセット
	worldTransform_.translation_ = positon;

	velocity_ = velocity;
}

void PlayerBullet::Update() { 
	//座標を移動させる
	worldTransform_.translation_ = Add(worldTransform_.translation_, velocity_);

	//時間経過でデス
	if (--deathTimer_ <= 0){
		isDead_ = true;
	}

	//行列を更新
	worldTransform_.UpdateMatrix(); 
}

void PlayerBullet::Draw(const ViewProjection& viewProjection) { 
	model_->Draw(worldTransform_, viewProjection, textureHandle_); 
}

void PlayerBullet::OnCollision() { isDead_ = true; }

Vector3 PlayerBullet::GetWorldPosition() {
	Vector3 WorldPos; // ワールド座標を入れる変数
	// ワールド行列の平行移動成分を取得(ワールド座標)
	WorldPos.x = worldTransform_.matWorld_.m[3][0];
	WorldPos.y = worldTransform_.matWorld_.m[3][1];
	WorldPos.z = worldTransform_.matWorld_.m[3][2];

	return WorldPos;
}
