#include "Node.h"
#include <math.h>

Node::Node()
{
	value = SPACE;
	parent = nullptr;
	g = 0;
	this->specialSpot = 0;
	this->target = new Point2D(0, 0);
	this->dangerLevel = 0;
}


Node::~Node()
{
}

Node::Node(Point2D& pt, Point2D* t, int v, double g, Node* pr) {
	point = pt;
	target = t;
	value = v;
	parent = pr;
	this->g = g;
	computeH();
	this->dangerLevel = 0;
	this->specialSpot = 0;
}


void Node::SetValue(int value)
{
	this->value = value;
}

int Node::GetValue()
{
	return value;
}

double Node::getG()
{
	return g;
}

void Node::setG(double g)
{
	this->g = g;
}


void Node::computeH()
{
	this->h = sqrt(pow(point.getRow() - target->getRow(), 2) +
		pow(point.getCol() - target->getCol(), 2)) + this->dangerLevel;
}

double Node::getF()
{
	return g + this->h;
}

int Node::getDangerLevel()
{
	return this->dangerLevel;
}

void Node::setDangerLevel(int dangerLevel, int i, int j)
{
	this->dangerLevel = dangerLevel;
}

int Node::getSpecialSpot()
{
	return this->specialSpot;
}

void Node::setSpecialSpot(int kindOfSpot)
{
	this->specialSpot = kindOfSpot;
}

Point2D Node::getPoint()
{
	return point;
}

void Node::setPoint(Point2D point)
{
	this->point = point;
}

Node* Node::getParent()
{
	return parent;
}

void Node::setParent(Node* parent)
{
	this->parent = parent;
}

Point2D* Node::getTarget()
{
	return target;
}

void Node::setTarget(Point2D target)
{
	this->target = new Point2D(target.getRow(), target.getCol());
}
double Node::getH()
{
	return this->h;
}

double Node::calcX()
{
	double x = ((((double)this->point.getCol() * 2) / MSZ) - 1);
	return x;
}

double Node::calcY()
{
	double y = ((((double)this->point.getRow() * 2) / MSZ) - 1);
	return y;
}
