#pragma once
#include "Sprite.h"
#include "base/MathTypes.h"

class SpriteTransform {
public:
	void Initialize(Sprite* sprite);

	// 移動
	void Move();

	// 回転
	void Rotate();

	// 色
	void ChangeColor();

	// 拡縮
	void Scale();

private:
	Sprite* sprite_ = nullptr;
};
