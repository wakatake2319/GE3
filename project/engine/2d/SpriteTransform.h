#pragma once
#include "Sprite.h"
#include "base/MathTypes.h"

class SpriteTransform {
public:
	void Initialize(Sprite* sprite);

	void Move();

private:
	Sprite* sprite_ = nullptr;
};
