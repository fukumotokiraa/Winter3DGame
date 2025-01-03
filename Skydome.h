#include "Model.h"
#include "ViewProjection.h"
#include "WorldTransform.h"
#include "TextureManager.h"

#pragma once
class Skydome {
public:
	void Initialize(Model* model,ViewProjection* viewProjection);
	void Update();
	void Draw();

private:
	// ワールド変換データ
	WorldTransform worldTransform_;
	// モデル
	Model* model_ = nullptr;
	uint32_t textureHandle_ = 0;
	ViewProjection* viewProjection_ = nullptr;
};