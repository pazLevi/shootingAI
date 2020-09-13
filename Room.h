#pragma once
#include "Point2D.h"
#include "Node.h"
#include "Player.h"
#include <vector>
using namespace std;

class Room
{
public:
	Room();
	~Room();
	Room(int ci, int cj, int w, int h);
	Point2D getLeftTop();
	Point2D getRightBottom();
	bool CheckOverlapping(Room* pother);
	void toString();
	Point2D getCenter();
	int getNumOfExits();
	void setNumOfExits(int numOfExits);
	void addExit(Node exit);
	vector<Point2D> getHallwayExits();
	bool checkPlayersInTheSameRoom(Player* player1, Player* player2);
	bool checkIfPlayerInRoom(Player* player);
	bool checkIfNodeIsPartOfTheRoom(Node* node);

private:
	Point2D leftTop, rightBottom, center;
	int width, height;
	vector<Point2D> hallwayExits;
	int numOfExits;

};

