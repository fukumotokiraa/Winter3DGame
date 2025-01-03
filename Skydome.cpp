#include "Skydome.h"
#include <cassert>

void Skydome::Initialize(Model* model,ViewProjection* viewProjection) {
	assert(model);
	worldTransform_.Initialize();
	viewProjection_ = viewProjection;
	//textureHandle_ = textureHandle;
	model_ = model;
	// テクスチャ読み込み
	textureHandle_ = TextureManager::Load("./Resources/Skydome/sky.jpg");
}

void Skydome::Update() {
	worldTransform_.scale_ = {500, 500, 500};
	worldTransform_.UpdateMatrix();
}

void Skydome::Draw() { model_->Draw(worldTransform_, *viewProjection_); }