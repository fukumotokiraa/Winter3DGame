#pragma once

#include "Model.h"
#include "WorldTransform.h"
#include"TextureManager.h"
#include"Calculation.h"

class EnemyBullet {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Model* model, const Vector3& positon, const Vector3& velocity);

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(const ViewProjection& viewProjection);

	void OnCollision(); // 衝突を検出したら呼び出されるコールバック関数

	bool IsDead() const { return isDead_; }

	Vector3 GetWorldPosition();

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	uint32_t textureHandle_ = 0u;
	Vector3 velocity_;
	static const int32_t kLifeTime = 60 * 5; // 寿命
	int32_t deathTimer_ = kLifeTime;         // デスタイマー
	bool isDead_ = false;                    // デスフラグ
	
};
