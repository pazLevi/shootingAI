#pragma once
#include "Point2D.h"
const int MSZ = 100;
const int SPACE = 0;
const int WALL = 1;
const int TEAM1PLAYER = 2;
const int TEAM2PLAYER = 3;
const int PATH = 4; // belongs to the path to target
const int GRAY = 5; // Fringe
const int BLACK = 6; // VISITED

//for special spot 
const int INSIDE_ROOM = 0;
const int HEAL = 1;
const int AMMO = 2;
const int HALLWAY = 3;



class Node
{
public:
	Node();
	~Node();

	Node(Point2D& pt, Point2D* t, int v, double g, Node* pr);

private:
	int value;
	double h, g;
	Node* parent;
	Point2D* target;
	Point2D point;
	double dangerLevel;
	int specialSpot;


public:

	void SetValue(int value);
	int GetValue();
	double getG();
	void setG(double g);
	void computeH();
	double getF();
	int getDangerLevel();
	void setDangerLevel(int dangerLevel, int	i, int j);
	int getSpecialSpot();
	void setSpecialSpot(int kindOfSpot);
	Point2D getPoint();
	void setPoint(Point2D point);
	Node* getParent();
	void setParent(Node* parent);
	Point2D* getTarget();
	void setTarget(Point2D target);
	double getH();
	double calcX();
	double calcY();
	bool operator == (const Node& other) {
		return point == other.point;
	}
};

