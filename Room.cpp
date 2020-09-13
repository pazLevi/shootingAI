#include "Room.h"
#include <math.h>
#include <iostream>
#include <stdlib.h>
#include "Point2D.h"
#include <vector>


using namespace std;




Room::Room()
{
}


Room::~Room()
{
}

Room::Room(int ci, int cj, int w, int h) {
	center.setCol(cj);
	center.setRow(ci);
	leftTop.setCol(cj - w / 2);
	leftTop.setRow(ci - h / 2);
	rightBottom.setCol(cj + w / 2);
	rightBottom.setRow(ci + h / 2);
	width = w;
	height = h;
	this->numOfExits = 0;

}

Point2D Room::getLeftTop()
{
	return leftTop;
}
Point2D Room::getRightBottom() {
	return rightBottom;
}

bool Room::CheckOverlapping(Room* pother)
{
	int horiz_dist, vert_dist, vsz, hsz;
	horiz_dist = abs(center.getCol() - pother->center.getCol());
	vert_dist = abs(center.getRow() - pother->center.getRow());
	vsz = height / 2 + pother->height / 2;
	hsz = width / 2 + pother->width / 2;

	return horiz_dist <= hsz + 2 && vert_dist <= vsz + 2;
}


void Room::toString()
{
	cout << "check new Room " << "center: (" << center.getRow() << "," << center.getCol()
		<< "), width: " << width << ", height" << height << endl;
}

Point2D Room::getCenter()
{
	return center;
}

int Room::getNumOfExits()
{
	return this->hallwayExits.size();
}

void Room::setNumOfExits(int numOfExits)
{
	this->numOfExits = numOfExits;
}

void Room::addExit(Node exit)
{
	this->hallwayExits.push_back(exit.getPoint());
}

vector<Point2D> Room::getHallwayExits()
{
	return this->hallwayExits;
}

bool Room::checkPlayersInTheSameRoom(Player* player1, Player* player2)
{
	if (checkIfPlayerInRoom(player1) == true && checkIfPlayerInRoom(player2) == true)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Room::checkIfPlayerInRoom(Player* player)
{
	if ((this->leftTop.getRow() <= player->getPoint()->getRow() && player->getPoint()->getRow() <= this->rightBottom.getRow()) && (this->leftTop.getCol() <= player->getPoint()->getCol() && player->getPoint()->getCol() <= this->rightBottom.getCol()))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Room::checkIfNodeIsPartOfTheRoom(Node* node)
{
	if ((this->leftTop.getRow() <= node->getPoint().getRow() && node->getPoint().getRow() <= this->rightBottom.getRow()) && (this->leftTop.getCol() <= node->getPoint().getCol() && node->getPoint().getCol() <= this->rightBottom.getCol()))
	{
		return true;
	}
	else
	{
		return false;
	}

}

