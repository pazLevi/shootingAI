#include "Granade.h"
#include <math.h>
const double PI = 3.14;



Granade::Granade()
{
}

Granade::Granade(double x, double y)
{
	int i;
	double alpha, delta = 2 * PI / NUM_BULLETS;
	this->x = x;
	this->y = y;
	this->radius = 0.1;
	for (i = 0, alpha = 0; i < NUM_BULLETS; i++, alpha += delta)
	{
		bullets[i] = new Bullet(x, y);
		bullets[i]->SetDir(alpha);
	}

}


Granade::~Granade()
{
}

void Granade::move(Node(&maze)[MSZ][MSZ])
{
}

void Granade::explode()
{
	for (int i = 0; i < NUM_BULLETS; i++)
	{
		bullets[i]->setX(this->x);
		bullets[i]->setY(this->y);
		bullets[i]->SetIsMoving(true);
	}
}

void Granade::showMe()
{
	for (int i = 0; i < NUM_BULLETS; i++)
		bullets[i]->showMe();
}

void Granade::moveBullets(Node(&maze)[MSZ][MSZ])
{
	for (int i = 0; i < NUM_BULLETS; i++)
	{
		if (bullets[i]->GetIsMoving() == true)
		{
			bullets[i]->move(maze);
			if (maze[bullets[i]->calcRow()][bullets[i]->calcCol()].GetValue() == WALL)
			{
				bullets[i]->SetIsMoving(false);
			}
		}
	}
}

void Granade::SimulateExplosion(Node(&maze)[MSZ][MSZ])
{
	for (int i = 0; i < NUM_BULLETS; i++)
	{
		bullets[i]->SimulateMotion(maze);
	}

}

void Granade::setDir(double angle)
{
	dirx = cos(angle);
	diry = sin(angle);
}

