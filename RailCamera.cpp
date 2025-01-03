#include "RailCamera.h"

void RailCamera::Initialize(WorldTransform& worldTransform) { 
	worldTransform_.translation_.x = worldTransform.matWorld_.m[3][0];
	worldTransform_.translation_.y = worldTransform.matWorld_.m[3][1];
	worldTransform_.translation_.z = worldTransform.matWorld_.m[3][2];

	worldTransform_.rotation_ = worldTransform.rotation_;

	viewProjection_.Initialize();

	velocity_ = {0, 0, CameraSpeed_};
}

void RailCamera::Update() {
	//カメラを移動させる
	velocity_ = {0, 0, CameraSpeed_};
	worldTransform_.translation_ = Add(worldTransform_.translation_, velocity_);

	//カメラオブジェクトのワールド行列からビュー行列を計算する
	viewProjection_.matView = Inverse(worldTransform_.matWorld_);

	//カメラの座標を画面表示する処理
	//ImGui::Begin("Camera");
	//ImGui::SliderFloat3("CameraTransration", &worldTransform_.translation_.x, -100.0f, 100.0f);
	//ImGui::SliderFloat3("CameraRotation", &worldTransform_.rotation_.x, -100.0f, 100.0f);
	//ImGui::End();

	// 行列を更新
	worldTransform_.UpdateWorld();
}

void RailCamera::Draw() {}
