#pragma once
#include "GameObject.h"


class Obstacle : public GameObject
{
public:
	Obstacle();
	virtual ~Obstacle();

	void Animate(double _timeElapsed) final;
};

