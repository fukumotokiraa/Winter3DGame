#pragma once

#include "WorldTransform.h"
#include "ViewProjection.h"
#include "Calculation.h"

class RailCamera {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(WorldTransform& worldTransform);

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	const Matrix4x4& GetViewMatrix() { return viewProjection_.matView; }

	const Matrix4x4& GetProjectionMatrix() { return viewProjection_.matProjection; }

	const WorldTransform& GetWorldTransform() { return worldTransform_; }

private:
	WorldTransform worldTransform_;
	ViewProjection viewProjection_;
	float CameraSpeed_ = 0.01f;//レールカメラのスピード
	Vector3 velocity_;

};
