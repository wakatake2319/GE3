#include "SpriteTransform.h"
#include <iostream>

// 初期化
void SpriteTransform::Initialize(Sprite* sprite) { sprite_ = sprite; }

// 移動
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

// 色
void SpriteTransform::ChangeColor() {
	Vector4 color = sprite_->GetColor();
	color.x += 0.01f;
	if (color.x > 1.0f) {
		color.x -= 1.0f;
	}
	sprite_->SetColor(color);
}

// 拡縮
void SpriteTransform::Scale() { 
	Vector2 size = sprite_->GetSize();
	size.x += 0.1f;
	size.y += 0.1f;
	sprite_->SetSize(size);
}