#include "Player.h"
#include <iostream>
const double deltaShoot = 2 * 3.14 / 6;

using namespace std;

Player::Player()
{
	this->id = 0;
	this->point = new Point2D();
	this->point = point;
	this->ammo = 10;
	this->numOfGranades = 2;
	this->hp = 100;
	this->mode = 1;
	this->belongsToTeam = 0;
}
Player::Player(int id, Point2D* point)
{
	this->id = id;
	this->point = point;
	this->ammo = 10;
	this->numOfGranades = 2;
	this->hp = 100;
	this->mode = 1;
	this->belongsToTeam = 0;


}


Player::~Player()
{
}

bool Player::checkIfHit(Bullet* bt)
{
	int i, j;
	i = MSZ * (bt->getY() + 1) / 2;
	j = MSZ * (bt->getX() + 1) / 2;
	if ((this->point->getRow() == i && this->point->getCol() == j) ||
		(this->point->getRow() == i + 1 && this->point->getCol() == j + 1) ||
		(this->point->getRow() == i + 1 && this->point->getCol() == j) ||
		(this->point->getRow() == i && this->point->getCol() == j + 1) ||
		(this->point->getRow() == i - 1 && this->point->getCol() == j - 1) ||
		(this->point->getRow() == i - 1 && this->point->getCol() == j) ||
		(this->point->getRow() == i && this->point->getCol() == j - 1))
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*list of returns and action taken
returns 1 - fight enemy (if he knows where is the enemy)
returns 2 - focus on survival (run)
returns 3 - focus on survival (search for healing spot)
returns 5 - search for ammo (know location of ammo)
*/
int Player::decideNextMove()
{
	if (this->mode == 1)  //aggressive
	{
		if (this->hp <= 50)
		{
			return 3;
		}
		else
		{
			if (this->ammo <= 5)
			{
				return 5;
			}
			else
			{
				return 1;
			}
		}
	}
	else if (this->mode == 2) //defensive
	{
		if (this->hp <= 30)
		{
			return 3;
		}
		else
		{
			if (this->ammo <= 6)
			{
				return 5;
			}
			else
			{
				return 1;
			}
		}
	}
	else // balanced
	{
		if (this->hp <= 50)
		{

			return 2;
		}
		else
		{
			if (this->ammo <= 4)
			{

				return 5;
			}
			else
			{
				return 1;
			}
		}
	}
}

double Player::calcX()
{
	double x = ((((double)this->point->getCol() * 2) / MSZ) - 1);
	return x;
}

double Player::calcY()
{
	double y = ((((double)this->point->getRow() * 2) / MSZ) - 1);
	return y;
}


int Player::getHp()
{
	return this->hp;
}

void Player::setHp(int hp)
{
	this->hp = hp;
}

Bullet* Player::shoot(double angle, Node(&maze)[MSZ][MSZ])
{
	Bullet* bt = new Bullet(this->calcX(), this->calcY());
	bt->SetDir(angle);
	bt->setBelongsToTeam(this->belongsToTeam);
	bt->SetIsMoving(true);
	this->ammo -= 1;
	return bt;
}

Granade* Player::throwGranade(double angle, Node(&maze)[MSZ][MSZ])
{
	double x = this->calcX();
	double y = this->calcY();
	Granade* gd = new Granade(x, y);
	gd->setDir(angle);
	this->numOfGranades -= 1;
	return gd;
}

void Player::shootSimulation(Node(&maze)[MSZ][MSZ])
{
	int i;
	double angle = 0;
	for (i = 0; i < 6; i++)
	{
		Bullet* bt = new Bullet(this->calcX(), this->calcY());
		bt->SetDir(angle);
		bt->SimulateMotion(maze);
		angle += deltaShoot;
		delete bt;
	}
}

void Player::throwGranadeSimulation(Node(&maze)[MSZ][MSZ])
{
	int i;
	double angle = 0;
	for (i = 0; i < 6; i++)
	{
		double x = this->calcX();
		double y = this->calcY();
		Granade* gd = new Granade(x, y);
		gd->SimulateExplosion(maze);
		angle += deltaShoot;
	}
}

int Player::getBelongsToTeam()
{
	return this->belongsToTeam;
}

void Player::setBelongsToTeam(int teamNumber)
{
	this->belongsToTeam = teamNumber;
}

Point2D* Player::getPoint()
{
	return this->point;
}

void Player::setPoint(Point2D* point)
{
	this->point = point;
}

int Player::getMode()
{
	return this->mode;
}

void Player::setMode(int mode)
{
	this->mode = mode;
}

double Player::getAngle(Player* opposingPlayer)
{
	return atan2(opposingPlayer->calcY() - this->calcY(), opposingPlayer->calcX() - this->calcX());
}

double Player::getAngleToNode(Node* node)
{
	return atan2(node->calcY() - this->calcY(), node->calcX() - this->calcX());
}

int Player::getRoom()
{
	return this->room;
}

void Player::setRoom(int room)
{
	this->room = room;
}

void Player::showPlayer()
{
	double sz, x, y;
	sz = 2.0 / MSZ;
	x = this->point->getCol() * sz - 1;
	y = this->point->getRow() * sz - 1;
	if (this->belongsToTeam == 1)
	{
		glColor3d(0, 0, 1); //blue
	}
	else
	{
		glColor3d(0, 1, 0); //green
	}
	glBegin(GL_POLYGON);
	glVertex2d(x, y);
	glVertex2d(x + sz, y);
	glVertex2d(x + sz, y + sz);
	glVertex2d(x, y + sz);

	glEnd();
}

void Player::printRowAndCol()
{
	cout << "Row " << this->point->getRow() << " Col " << this->point->getCol() << endl;
}

void Player::reload()
{
	this->ammo = 10;
}

int Player::getId()
{
	return this->id;
}

void Player::setId(int id)
{
	this->id = id;
}
int Player::getNumOFGranades()
{
	return this->numOfGranades;
}
void Player::addToVisited(int roomNum)
{
	if (roomNum != -1)
		this->roomsVisited[roomNum] = true;
}
bool Player::isVisitedTheRoom(int roomNum)
{
	return this->roomsVisited[roomNum];
}
bool Player::allVisited()
{
	int i;
	for (i = 0; i < NUM_ROOMS; i++)
	{
		if (this->roomsVisited[i] == false)
			return false;
	}
	return true;
}
void Player::resetVisited()
{
	int i;
	for (i = 0; i < NUM_ROOMS; i++)
	{
		this->roomsVisited[i] = false;
	}
}