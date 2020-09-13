#pragma once
#include "Bullet.h"
#include <stdlib.h>
#include "Granade.h"
#include <math.h>
#include "GLUT.h"
const int NUM_ROOMS = 20;

class Player
{
public:
	Player();
	Player(int id, Point2D* point);
	~Player();
	bool checkIfHit(Bullet* bt);
	int decideNextMove();
	double calcX();
	double calcY();
	int getHp();
	void setHp(int hp);
	Bullet* shoot(double angle, Node(&maze)[MSZ][MSZ]);
	Granade* throwGranade(double angle, Node(&maze)[MSZ][MSZ]);
	void shootSimulation(Node(&maze)[MSZ][MSZ]);
	void throwGranadeSimulation(Node(&maze)[MSZ][MSZ]);
	int getBelongsToTeam();
	void setBelongsToTeam(int teamNumber);
	Point2D* getPoint();
	void setPoint(Point2D* point);
	int getMode();
	void setMode(int mode);
	double getAngle(Player* opposingPlayer);
	double getAngleToNode(Node* node);
	int getRoom();
	void setRoom(int room);
	void showPlayer();
	void printRowAndCol();
	void reload();
	int getId();
	void setId(int id);
	int getNumOFGranades();
	void addToVisited(int roomNum);
	bool isVisitedTheRoom(int roomNum);
	bool allVisited();
	void resetVisited();
private:
	int id;
	int belongsToTeam;
	int room;
	Point2D* point;
	int hp;
	int ammo;
	int numOfGranades;
	int mode;
	bool roomsVisited[NUM_ROOMS];
};

