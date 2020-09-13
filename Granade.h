#pragma once
#include "Bullet.h"
#include "Node.h"

const int NUM_BULLETS = 16;

class Granade
{
public:
	Granade();
	Granade(double x, double y);
	~Granade();
	void move(Node(&maze)[MSZ][MSZ]);
	void explode();
	void showMe();
	void moveBullets(Node(&maze)[MSZ][MSZ]);
	void SimulateExplosion(Node(&maze)[MSZ][MSZ]);
	void setDir(double angle);
private:
	int radius;
	double x, y;
	double dirx, diry;
	Bullet* bullets[NUM_BULLETS];

};

