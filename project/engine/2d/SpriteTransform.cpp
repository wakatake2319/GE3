#include "SpriteTransform.h"
#include <iostream>

// 初期化
void SpriteTransform::Initialize(Sprite* sprite) { sprite_ = sprite; }

void SpriteTransform::Move() {
	// 現在の座標を変数で受ける
	Vector2 position_ = sprite_->GetPosition();
	// 座標を変更する
	position_.x += 0.1f;
	position_.y += 0.1f; 
	// 変更を反映する
	sprite_->SetPosition(position_);
}

// 回転
void SpriteTransform::Rotate() { 
	float rotation_ = sprite_->GetRotation();
	rotation_ += 0.01f;
	sprite_->SetRotation(rotation_);
}