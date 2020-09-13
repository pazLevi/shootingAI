#include "GLUT.h"
#include <math.h>
#include <time.h>
#include "Node.h"
#include "Room.h"
#include <vector>
#include <iostream>
#include <queue>
#include <math.h>
#include <Windows.h>
#include "CompareNodes.h"
#include "CompareNodesOther.h"
#include "Bullet.h"
#include "Granade.h"
#include "Player.h"
#pragma comment(lib, "Winmm.lib")


using namespace std;

const int W = 600; // window width
const int H = 600; // window height

const int NUM_OF_HEAL_SPOT = 2;
const int NUM_OF_AMMO_SPOT = 2;
const int NUM_OF_MAX_STEPS = 20;
const int DEF_NUM = 50;
const int DEF_HEAL_NUM = 30;
bool run_bfs = false;

Node maze[MSZ][MSZ];
Room rooms[NUM_ROOMS];
int numExistingRooms = 0;
int numOfSteps = 0;

Player* team1player1, * team1player2, * team2player1, * team2player2;
Node* healNodes[NUM_OF_HEAL_SPOT];
Node* ammoNodes[NUM_OF_AMMO_SPOT];
priority_queue <Node*, vector<Node*>, CompareNodes> pqAstar;
vector <Node> gray; // gray nodes


void SetupMaze();
void display();
void generateSafetyMapFromPlayer(Player* enemyPlayer);
bool checkGameEnd();
void gameStart();
Node* aStar(Player* playerInTurn, Node* target);
Node* getNextStep(Player* playerInTurn, Node* finalNode);
Node* getclosetNodeFromOpposingTeam(int yourTeam);
Node* getCloserHealSpot(Player* playerInTurn);
Node* getCloserAmmoSpot(Player* playerInTurn);
void generatePlayers();
bool checkOverlappingWithOtherPlayers(int i, int j);
void executeDecision(Player* player, int decision);
void resetMazeDangerLevel();
void hitTarget(Player* playerInTurn, Player* player);
void displayBullet(Bullet* bt);
bool checkOtherPlayerOnNode(Node* node);
int checkWhoCanBeShot(Player* player);
int checkPlayersInTheSamRoom(Player* player1, Player* player2);
void throwGranadeAtTarget(Player* player, Room* roomIn);
int getRoomNumber(Player* player);
Node* getNextStepLookingForTheEnemy(Player* player);
bool isThereCover(Room* roomIn);
Node* getCover(Room* roomIn);
void searchAndFight(Player* player);


void init()
{
	srand(time(0)); // pseudo randomizer
	glClearColor(0.7, 0.7, 0.7, 0);
	SetupMaze();
	glOrtho(-1, 1, -1, 1, -1, 1);
}


Room GenerateRoom()
{
	int w, h, ci, cj;
	Room* pr = nullptr;
	bool isOveralaping;

	do
	{
		delete pr;
		isOveralaping = false;
		w = 6 + rand() % 10;
		h = 6 + rand() % 10;

		ci = h / 2 + rand() % (MSZ - h);
		cj = w / 2 + rand() % (MSZ - w);

		pr = new Room(ci, cj, w, h);
		for (int i = 0; i < numExistingRooms && !isOveralaping; i++)
		{
			if (rooms[i].CheckOverlapping(pr))
				isOveralaping = true;
		}
	} while (isOveralaping);

	//pr is not overlapping with other rooms
	for (int i = pr->getLeftTop().getRow(); i <= pr->getRightBottom().getRow(); i++)
		for (int j = pr->getLeftTop().getCol(); j <= pr->getRightBottom().getCol(); j++)
			maze[i][j].SetValue(SPACE);
	return *pr;
}


// check if the node at row,col is white or gray that is better then the previous one
// and if so add it to pq
void AddNode(int row, int col, Node* pn, vector<Node>& gray, vector<Node>& black,
	priority_queue <Node*, vector<Node*>, CompareNodes>& pq)
{
	Point2D pt;
	Node* pn1;
	vector<Node>::iterator gray_it;
	vector<Node>::iterator black_it;
	double cost = 0;

	pt.setRow(row);
	pt.setCol(col);
	if (maze[row][col].GetValue() == SPACE)
		cost = 0.1; // space cost
	else if (maze[row][col].GetValue() == WALL)
		cost = 3;
	// cost depends on is it a wall or a space
	pn1 = new Node(pt, pn->getTarget(), maze[pt.getRow()][pt.getCol()].GetValue(), pn->getG() + cost, pn);

	black_it = find(black.begin(), black.end(), *pn1);
	gray_it = find(gray.begin(), gray.end(), *pn1);
	if (black_it == black.end() && gray_it == gray.end()) // it is not black and not gray!
	{// i.e. it is white
		pq.push(pn1);
		gray.push_back(*pn1);
	}
}


void AddNeighbours(Node* pn, vector<Node>& gray, vector<Node>& black,
	priority_queue <Node*, vector<Node*>, CompareNodes>& pq)
{
	// try down
	if (pn->getPoint().getRow() < MSZ - 1)
		AddNode(pn->getPoint().getRow() + 1, pn->getPoint().getCol(), pn, gray, black, pq);
	// try up
	if (pn->getPoint().getRow() > 0)
		AddNode(pn->getPoint().getRow() - 1, pn->getPoint().getCol(), pn, gray, black, pq);
	// try left
	if (pn->getPoint().getCol() > 0)
		AddNode(pn->getPoint().getRow(), pn->getPoint().getCol() - 1, pn, gray, black, pq);
	// try right
	if (pn->getPoint().getCol() < MSZ - 1)
		AddNode(pn->getPoint().getRow(), pn->getPoint().getCol() + 1, pn, gray, black, pq);
}

// implement A* from start to target
void GeneratePath(Point2D start, Point2D target)
{
	priority_queue <Node*, vector<Node*>, CompareNodes> pq;
	vector<Node> gray;
	vector<Node> black;
	Node* pn;
	bool stop = false;
	vector<Node>::iterator gray_it;
	vector<Node>::iterator black_it;

	pn = new Node(start, &target, maze[start.getRow()][start.getCol()].GetValue(), 0, nullptr);
	pq.push(pn);
	gray.push_back(*pn);
	while (!pq.empty() && !stop)
	{
		// take the best node from pq
		pn = pq.top();
		// remove top Node from pq
		pq.pop();
		if (pn->getPoint() == target) // the path has been found
		{
			stop = true;
			// restore path to dig tunnels
			// set SPACE instead of WALL on the path
			while (!(pn->getPoint() == start))
			{
				if (maze[pn->getPoint().getRow()][pn->getPoint().getCol()].GetValue() == WALL)
				{
					maze[pn->getPoint().getRow()][pn->getPoint().getCol()].SetValue(SPACE);
					maze[pn->getPoint().getRow()][pn->getPoint().getCol()].setSpecialSpot(HALLWAY);
					pn = pn->getParent();
				}
				else
				{
					maze[pn->getPoint().getRow()][pn->getPoint().getCol()].SetValue(SPACE);
					pn = pn->getParent();
				}
			}
			return;
		}
		else // pn is not target
		{
			// remove Node from gray and add it to black
			gray_it = find(gray.begin(), gray.end(), *pn); // operator == must be implemented in Node
			if (gray_it != gray.end())
				gray.erase(gray_it);
			black.push_back(*pn);
			// check the neighbours
			AddNeighbours(pn, gray, black, pq);
		}
	}
}

void DigTunnels()
{
	int i, j;

	for (i = 0; i < NUM_ROOMS; i++)
	{
		cout << "Path from " << i << endl;
		for (j = i + 1; j < NUM_ROOMS; j++)
		{
			cout << " to " << j << endl;
			GeneratePath(rooms[i].getCenter(), rooms[j].getCenter());
		}
	}
}

void SetupMaze()
{
	int i, j, k;
	for (i = 0; i < MSZ; i++)
	{
		for (j = 0; j < MSZ; j++)
		{
			maze[i][j].setPoint(Point2D(i, j));
			maze[i][j].setTarget(Point2D(0, 0));
			
		}
	}
	for (i = 0; i < MSZ; i++)
		for (j = 0; j < MSZ; j++)
			maze[i][j].SetValue(WALL);

	for (numExistingRooms = 0; numExistingRooms < NUM_ROOMS; numExistingRooms++)
		rooms[numExistingRooms] = GenerateRoom();

	for (k = 0; k < 30; k++)
	{
		i = rand() % MSZ;
		j = rand() % MSZ;
		maze[i][j].SetValue(WALL);
	}

	DigTunnels();
	//add entrance(hallway exits) to all rooms
	for (i = 0; i < NUM_ROOMS; i++)
	{
		// upper line
		for (j = rooms[i].getLeftTop().getCol(); j <= rooms[i].getRightBottom().getCol(); j++)
		{
			if (rooms[i].getLeftTop().getRow() > 0)
				if (maze[rooms[i].getLeftTop().getRow() - 1][j].getSpecialSpot() == HALLWAY)
					rooms[i].addExit(maze[rooms[i].getLeftTop().getRow()][j]);
		}
		// left column
		for (j = rooms[i].getLeftTop().getRow(); j <= rooms[i].getRightBottom().getRow(); j++)
		{
			if (rooms[i].getLeftTop().getCol() > 0)
				if (maze[j][rooms[i].getLeftTop().getCol() - 1].getSpecialSpot() == HALLWAY)
					rooms[i].addExit(maze[j][rooms[i].getLeftTop().getCol()]);
		}
		//right column
		for (j = rooms[i].getLeftTop().getRow(); j <= rooms[i].getRightBottom().getRow(); j++)
		{
			if (rooms[i].getRightBottom().getCol() < MSZ - 1)
				if (maze[j][rooms[i].getRightBottom().getCol() + 1].getSpecialSpot() == HALLWAY)
					rooms[i].addExit(maze[j][rooms[i].getRightBottom().getCol()]);
		}
		// bottom line
		for (j = rooms[i].getLeftTop().getCol(); j <= rooms[i].getRightBottom().getCol(); j++)
		{
			if (rooms[i].getRightBottom().getRow() < MSZ - 1)
				if (maze[rooms[i].getRightBottom().getRow() + 1][j].getSpecialSpot() == HALLWAY)
					rooms[i].addExit(maze[rooms[i].getRightBottom().getRow()][j]);
		}
	}

	for (i = 0; i < NUM_OF_HEAL_SPOT; i++)
	{
		do {
			k = rand() % MSZ;
			j = rand() % MSZ;
		} while (maze[k][j].GetValue() == WALL || maze[k][j].getSpecialSpot() == HALLWAY);
		maze[k][j].setSpecialSpot(HEAL);
		healNodes[i] = &maze[k][j];
	}
	for (i = 0; i < NUM_OF_AMMO_SPOT; i++)
	{
		do {
			k = rand() % MSZ;
			j = rand() % MSZ;
		} while (maze[k][j].GetValue() == WALL || maze[k][j].getSpecialSpot() == HALLWAY || maze[k][j].getSpecialSpot() == HEAL);
		maze[k][j].setSpecialSpot(AMMO);
		ammoNodes[i] = &maze[k][j];
	}


}
void generatePlayers()
{
	team1player1 = new Player();
	team1player2 = new Player();
	team2player1 = new Player();
	team2player2 = new Player();
	int i, j;
	team1player1->setId(1);
	do
	{
		i = rand() % MSZ;
		j = rand() % MSZ;
	} while (maze[i][j].GetValue() == WALL || checkOverlappingWithOtherPlayers(i, j));
	team1player1->setBelongsToTeam(1);
	team1player1->setPoint(new Point2D(i, j));
	team1player1->setMode(rand() % 3 + 1);
	team1player2->setId(2);
	do
	{
		i = rand() % MSZ;
		j = rand() % MSZ;
	} while (maze[i][j].GetValue() == WALL || checkOverlappingWithOtherPlayers(i, j));
	team1player2->setBelongsToTeam(1);
	team1player2->setPoint(new Point2D(i, j));
	team1player2->setMode(rand() % 3 + 1);
	team2player1->setId(3);
	do
	{
		i = rand() % MSZ;
		j = rand() % MSZ;
	} while (maze[i][j].GetValue() == WALL || checkOverlappingWithOtherPlayers(i, j));
	team2player1->setBelongsToTeam(2);
	team2player1->setPoint(new Point2D(i, j));
	team2player1->setMode(rand() % 3 + 1);
	team2player2->setId(4);
	do
	{
		i = rand() % MSZ;
		j = rand() % MSZ;
	} while (maze[i][j].GetValue() == WALL || checkOverlappingWithOtherPlayers(i, j));
	team2player2->setBelongsToTeam(2);
	team2player2->setPoint(new Point2D(i, j));
	team2player2->setMode(rand() % 3 + 1);

}


bool checkOverlappingWithOtherPlayers(int i, int j)
{
	if ((team1player1->getPoint()->getRow() == i && team1player1->getPoint()->getCol() == j) || (team1player2->getPoint()->getRow() == i && team1player2->getPoint()->getCol() == j)
		|| (team2player1->getPoint()->getRow() == i && team2player1->getPoint()->getCol() == j) || (team2player2->getPoint()->getRow() == i && team2player2->getPoint()->getCol() == j))
	{
		return true;
	}
	else
	{
		return false;
	}
}


void generateSafetyMapFromPlayer(Player* enemyPlayer)
{
	enemyPlayer->shootSimulation(maze);
	enemyPlayer->throwGranadeSimulation(maze);
}


// one step adds 20 to g
Node* aStar(Player* playerInTurn, Node* target)
{
	Node* current = new Node();
	Node* nextStep = new Node();
	Node* next = new Node();
	int i;
	pqAstar.push(&maze[playerInTurn->getPoint()->getRow()][playerInTurn->getPoint()->getCol()]);
	maze[playerInTurn->getPoint()->getRow()][playerInTurn->getPoint()->getCol()].setG(maze[playerInTurn->getPoint()->getRow()][playerInTurn->getPoint()->getCol()].getDangerLevel());
	vector <Node*> blackedNodes;
	while (!pqAstar.empty())
	{
		current = pqAstar.top();
		pqAstar.pop();
		if (current->getPoint().getRow() == target->getPoint().getRow() && current->getPoint().getCol() == target->getPoint().getCol())
		{
			nextStep = getNextStep(playerInTurn, current);
			if (nextStep->getPoint().getRow() == 0 && nextStep->getPoint().getCol() == 0)
			{
				cout << " " << endl;
			}
			break;
		}
		current->SetValue(BLACK);
		blackedNodes.push_back(current);
		//One Step UP
		if (current->getPoint().getRow() + 1 < MSZ - 1)
		{
			next = &maze[current->getPoint().getRow() + 1][current->getPoint().getCol()];
			if (next->GetValue() == SPACE)
			{
				next->SetValue(GRAY);
				next->computeH();
				next->setG(current->getG() + NUM_OF_MAX_STEPS);
				next->setParent(current);
				pqAstar.push(next);
			}
			else if (next->GetValue() == GRAY)
			{
				if ( current->getG() + next->getH() < next->getF())
				{
					next->setG(current->getG() + NUM_OF_MAX_STEPS);
					next->setParent(current);
				}
			}
			if (next->getPoint().getRow() == target->getPoint().getRow() && next->getPoint().getCol() == target->getPoint().getCol())
			{
				next->setParent(current);
				nextStep = getNextStep(playerInTurn, next);
				if (nextStep->getPoint().getRow() == 0 && nextStep->getPoint().getCol() == 0)
				{
					cout << " " << endl;
				}
				break;
			}
		}
		//One Step Down
		if (current->getPoint().getRow() - 1 > 0)
		{
			next = &maze[current->getPoint().getRow() - 1][current->getPoint().getCol()];
			if (next->GetValue() == SPACE)
			{
				next->SetValue(GRAY);
				next->computeH();
				next->setG(current->getG() + NUM_OF_MAX_STEPS + next->getDangerLevel());
				next->setParent(current);
				pqAstar.push(next);
			}
			else if (next->GetValue() == GRAY)
			{
				if ( current->getG() + next->getH() < next->getF())
				{
					next->setG(current->getG() + NUM_OF_MAX_STEPS + next->getDangerLevel());
					next->setParent(current);
				}
			}
			if (next->getPoint().getRow() == target->getPoint().getRow() && next->getPoint().getCol() == target->getPoint().getCol())
			{
				next->setParent(current);
				nextStep = getNextStep(playerInTurn, next);
				if (nextStep->getPoint().getRow() == 0 && nextStep->getPoint().getCol() == 0)
				{
					cout << " " << endl;
				}
				break;
			}
		}
		//One Step left
		if (current->getPoint().getCol() - 1 > 0)
		{
			next = &maze[current->getPoint().getRow()][current->getPoint().getCol() - 1];
			if (next->GetValue() == SPACE)
			{
				next->SetValue(GRAY);
				next->computeH();
				next->setG(current->getG() + NUM_OF_MAX_STEPS + next->getDangerLevel());
				next->setParent(current);
				pqAstar.push(next);
			}
			else if (next->GetValue() == GRAY)
			{
				if (current->getG() + next->getH() < next->getF())
				{
					next->setG(current->getG() + NUM_OF_MAX_STEPS + next->getDangerLevel());
					next->setParent(current);
				}
			}
			if (next->getPoint().getRow() == target->getPoint().getRow() && next->getPoint().getCol() == target->getPoint().getCol())
			{
				next->setParent(current);
				nextStep = getNextStep(playerInTurn, next);
				if (nextStep->getPoint().getRow() == 0 && nextStep->getPoint().getCol() == 0)
				{
					cout << " " << endl;
				}
				break;
			}
		}
		//One Step Right
		if (current->getPoint().getCol() + 1 < MSZ - 1)
		{
			next = &maze[current->getPoint().getRow()][current->getPoint().getCol() + 1];
			if (next->GetValue() == SPACE)
			{
				next->SetValue(GRAY);
				next->computeH();
				next->setG(current->getG() + NUM_OF_MAX_STEPS + next->getDangerLevel());
				next->setParent(current);
				pqAstar.push(next);
			}
			else if (next->GetValue() == GRAY)
			{
				if ( current->getG() + next->getH() < next->getF())
				{
					next->setG(current->getG() + NUM_OF_MAX_STEPS + next->getDangerLevel());
					next->setParent(current);
				}
			}
			if (next->getPoint().getRow() == target->getPoint().getRow() && next->getPoint().getCol() == target->getPoint().getCol())
			{
				next->setParent(current);
				nextStep = getNextStep(playerInTurn, next);
				if (nextStep->getPoint().getRow() == 0 && nextStep->getPoint().getCol() == 0)
				{
					cout << " " << endl;
				}
				break;
			}
		}
	}

	// resetting all nodes in mazes to their original values
	while (!pqAstar.empty())
	{
		current = pqAstar.top();
		current->setParent(nullptr);
		current->SetValue(SPACE);
		pqAstar.pop();
	}
	for (i = 0; i < blackedNodes.size(); i++)
	{
		blackedNodes[i]->setParent(nullptr);
		blackedNodes[i]->SetValue(SPACE);
	}
	return nextStep;
}

Node* getNextStep(Player* playerInTurn, Node* finalNode)
{
	Node* current = finalNode;
	while (current->getParent() != &maze[playerInTurn->getPoint()->getRow()][playerInTurn->getPoint()->getCol()])
	{
		if (current->getParent() == nullptr)
			break;
		current = current->getParent();
		numOfSteps++;
	}
	return current;
}

int checkPlayersInTheSamRoom(Player* player1, Player* player2)
{
	int i;
	for (i = 0; i < NUM_ROOMS; i++)
	{
		if (rooms[i].checkPlayersInTheSameRoom(player1, player2) == true)
		{
			return i;
		}
	}
	return -1;
}

void throwGranadeAtTarget(Player* player, Room* roomIn)
{
	double angle = player->getAngleToNode(&maze[roomIn->getCenter().getRow()][roomIn->getCenter().getCol()]);
	Granade* gd = player->throwGranade(angle, maze);

}

int getRoomNumber(Player* player)
{
	int i;
	for (i = 0; i < NUM_ROOMS; i++)
	{
		if (rooms[i].checkIfPlayerInRoom(player) == true)
		{
			return i;
		}
	}
	return -1;
}

Node* getNextStepLookingForTheEnemy(Player* player)
{
	int i;
	int roomNumToGo = 0, minSteps;
	Node* nextStep;
	Node* currentStep;
	for (i = 0; i < NUM_ROOMS; i++)
	{
		if (player->isVisitedTheRoom(i) == false)
		{
			roomNumToGo = i;
			break;
		}
	}
	nextStep = aStar(player, &maze[rooms[roomNumToGo].getHallwayExits()[0].getRow()][rooms[roomNumToGo].getHallwayExits()[0].getCol()]);
	minSteps = numOfSteps;
	numOfSteps = 0;
	for (i = 1; i < rooms[roomNumToGo].getNumOfExits(); i++)
	{
		currentStep = aStar(player, &maze[rooms[roomNumToGo].getHallwayExits()[i].getRow()][rooms[roomNumToGo].getHallwayExits()[i].getCol()]);
		if (minSteps > numOfSteps)
		{
			nextStep = currentStep;
		}
		numOfSteps = 0;
	}
	return nextStep;
}

void executeDecision(Player* player, int decision)
{
	Node* target;
	switch (decision)
	{
		//the player search Fight
	case 1:
	{
		cout << "option 1 was chosen" << endl;
		cout << "the player life is" << player->getHp() << endl;
		cout << "before search and fight " << endl;
		searchAndFight(player);
		cout << "after search and fight " << endl;
		break;
	}
	//the player understand who is the player in front of him, find him and search the way to him (Astar) 
	case 2:
	{
		cout << "option 2 was chosen" << endl;
		Node* closestNode = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
		Node* targetNode = aStar(player, closestNode);
		player->setPoint(new Point2D(targetNode->getPoint().getRow(), targetNode->getPoint().getCol()));
		break;
	}
	//the player searching for heal spot and set is it
	case 3:
	{
		cout << "option 3 was chosen" << endl;
		target = getCloserHealSpot(player);
		Node* healSpotTarget = aStar(player, target);
		if (healSpotTarget->getSpecialSpot() == HEAL)
		{
			player->setHp(100);
			cout << "the player has healed" << endl;
		}
		player->setPoint(new Point2D(healSpotTarget->getPoint().getRow(), healSpotTarget->getPoint().getCol()));
		break;
	}


		//the player searching for AMMO spot and reload it
	case 5:
	{
		cout << "option 5 was chosen" << endl;
		cout << "player hp is: " << player->getHp() << endl;
		target = getCloserAmmoSpot(player);
		Node* ammoSpotTarget = aStar(player, target);
		if (ammoSpotTarget->getSpecialSpot() == AMMO)
		{
			player->reload();
			cout << "the player has reloaded " << endl;
		}
		player->setPoint(new Point2D(ammoSpotTarget->getPoint().getRow(), ammoSpotTarget->getPoint().getCol()));
		break;
	}
	default:
		break;
	}
}


void searchAndFight(Player* player)
{
	Node* target;
	Node* goToNext;
	//Player Belongs to team 1
	if (player->getBelongsToTeam() == 1)
	{
		cout << "player belonged to team 1 " << endl;
		//Checks if the Player is in the room with 2 others players from the oposite team
		if (checkPlayersInTheSamRoom(player, team2player1) != -1 && checkPlayersInTheSamRoom(player, team2player2) != -1)
		{
			cout << "the player is in the same as both enemies " << endl;
			//Attack Mode
			if (player->getMode() == 1)
			{
				//checks who got the lowest hp and hit him
				if (checkWhoCanBeShot(player) == team2player1->getId())
					hitTarget(player, team2player1);
				else if (checkWhoCanBeShot(player) == team2player2->getId())
					hitTarget(player, team2player2);
				else
				{
					target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
					goToNext = aStar(player, target);
					player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
				}
			}
			//Defensive Mode
			else if (player->getMode() == 2)
			{
				if (player->getHp() <= DEF_HEAL_NUM)
				{
					//find cover
					if (isThereCover(&rooms[getRoomNumber(player)]) == true)
					{
						target = getCover(&rooms[getRoomNumber(player)]);
						if (player->getPoint()->getRow() != target->getPoint().getRow() || player->getPoint()->getCol() != target->getPoint().getCol())
						{
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
							player->addToVisited(getRoomNumber(player));
						}
					}
					else
					{
						//checks who got the lowest hp and hit him
						if (checkWhoCanBeShot(player) == team2player1->getId())
							hitTarget(player, team2player1);
						else if (checkWhoCanBeShot(player) == team2player2->getId())
							hitTarget(player, team2player2);
						else
						{
							target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
						}
					}
				}
				else
				{
					//checks who got the lowest hp and hit him
					if (checkWhoCanBeShot(player) == team2player1->getId())
						hitTarget(player, team2player1);
					else if (checkWhoCanBeShot(player) == team2player2->getId())
						hitTarget(player, team2player2);
					else
					{
						target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
						goToNext = aStar(player, target);
						player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
					}
				}
			}
			//Defensive- searching Heal Mode 
			else if (player->getMode() == 3)
			{
				if (player->getHp() <= DEF_NUM)
				{
					//find cover
					if (isThereCover(&rooms[getRoomNumber(player)]) == true)
					{
						target = getCover(&rooms[getRoomNumber(player)]);
						if ((player->getPoint()->getRow() != target->getPoint().getRow()) || (player->getPoint()->getCol() != target->getPoint().getCol()))
						{
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
							player->addToVisited(getRoomNumber(player));
						}
					}
					else
					{
						//checks who got the lowest hp and hit him
						executeDecision(player, 3);
					}
				}
				else
				{
					//checks who got the lowest hp and hit him
					if (checkWhoCanBeShot(player) == team2player1->getId())
						hitTarget(player, team2player1);
					else if (checkWhoCanBeShot(player) == team2player2->getId())
						hitTarget(player, team2player2);
					else
					{
						target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
						goToNext = aStar(player, target);
						player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
					}
				}
			}
		}
		//Checks if the Player is in the room with 1 other player from the oposite team
		else if (checkPlayersInTheSamRoom(player, team2player1) != -1)
		{
			cout << "the player is at the same room as player 1" << endl;
			//Attack Mode
			if (player->getMode() == 1)
			{
				//hit him
				if (checkWhoCanBeShot(player) == team2player1->getId())
					hitTarget(player, team2player1);
				else
				{
					target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
					goToNext = aStar(player, target);
					player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
				}
			}
			//Defensive Mode
			else if (player->getMode() == 2)
			{
				if (player->getHp() <= DEF_HEAL_NUM)
				{
					//find cover
					if (isThereCover(&rooms[getRoomNumber(player)]) == true)
					{
						target = getCover(&rooms[getRoomNumber(player)]);
						if (player->getPoint()->getRow() != target->getPoint().getRow() || player->getPoint()->getCol() != target->getPoint().getCol())
						{
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
							player->addToVisited(getRoomNumber(player));
						}
					}
					else
					{
						//hit him
						if (checkWhoCanBeShot(player) == team2player1->getId())
							hitTarget(player, team2player1);
						else
						{
							target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
						}
					}
				}
				else
				{
					//hit him
					if (checkWhoCanBeShot(player) == team2player1->getId())
						hitTarget(player, team2player1);
				}
			}
			//Defensive- searching Heal Mode
			else if (player->getMode() == 3)
			{
				if (player->getHp() <= DEF_NUM)
				{
					//find cover
					if (isThereCover(&rooms[getRoomNumber(player)]) == true)
					{
						target = getCover(&rooms[getRoomNumber(player)]);
						if (player->getPoint()->getRow() != target->getPoint().getRow() || player->getPoint()->getCol() != target->getPoint().getCol())
						{
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
						}
					}
					else
					{
						executeDecision(player, 3);
					}
				}
				//hit him
				else
				{
					if (checkWhoCanBeShot(player) == team2player1->getId())
						hitTarget(player, team2player1);
					else
					{
						target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
						goToNext = aStar(player, target);
						player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
					}
				}
			}
		}
		//Checks if the Player is in the room with 1 other player from the oposite team
		else if (checkPlayersInTheSamRoom(player, team2player2) != -1)
		{
			cout << "the player is at the same room as player 2" << endl;
			//Attack Mode
			if (player->getMode() == 1)
			{
				//hit him
				if (checkWhoCanBeShot(player) == team2player2->getId())
					hitTarget(player, team2player2);
				else
				{
					target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
					goToNext = aStar(player, target);
					player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
				}
			}
			//Defensive Mode
			else if (player->getMode() == 2)
			{
				if (player->getHp() <= DEF_HEAL_NUM)
				{
					//find cover
					if (isThereCover(&rooms[getRoomNumber(player)]) == true)
					{
						target = getCover(&rooms[getRoomNumber(player)]);
						if (player->getPoint()->getRow() != target->getPoint().getRow() || player->getPoint()->getCol() != target->getPoint().getCol())
						{
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
							player->addToVisited(getRoomNumber(player));
						}
					}
					else
					{
						//hit him
						if (checkWhoCanBeShot(player) == team2player2->getId())
							hitTarget(player, team2player2);
						else
						{
							target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
						}
					}
				}
				else
				{
					//hit him
					if (checkWhoCanBeShot(player) == team2player2->getId())
						hitTarget(player, team2player2);
					else
					{
						target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
						goToNext = aStar(player, target);
						player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
					}
				}
			}
			//Defensive- searching Heal Mode
			else if (player->getMode() == 3)
			{
				if (player->getHp() <= DEF_NUM)
				{
					//find cover
					if (isThereCover(&rooms[getRoomNumber(player)]) == true)
					{
						target = getCover(&rooms[getRoomNumber(player)]);
						if (player->getPoint()->getRow() != target->getPoint().getRow() || player->getPoint()->getCol() != target->getPoint().getCol())
						{
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
							player->addToVisited(getRoomNumber(player));
						}
					}
					else
					{
						executeDecision(player, 3);
					}
				}
				else
				{
					//hit him
					if (checkWhoCanBeShot(player) == team2player2->getId())
						hitTarget(player, team2player2);
					else
					{
						target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
						goToNext = aStar(player, target);
						player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
					}
				}
			}
		}
		else
		{
			//There is No one in the room keep searching for enemy
			cout << "keep searching " << endl;
			target = getNextStepLookingForTheEnemy(player);
			player->setPoint(new Point2D(target->getPoint().getRow(), target->getPoint().getCol()));
			player->addToVisited(getRoomNumber(player));
		}
	}
	//Player Belongs to team 2
	else if (player->getBelongsToTeam() == 2)
	{
		cout << "the player belongs to team 2 " << endl;
		if (checkPlayersInTheSamRoom(player, team1player1) != -1 && checkPlayersInTheSamRoom(player, team1player2) != -1)
		{
			cout << "the player is at the same room as both players" << endl;
			if (player->getMode() == 1)
			{
				if (checkWhoCanBeShot(player) == team1player1->getId())
					hitTarget(player, team1player1);
				else if (checkWhoCanBeShot(player) == team1player2->getId())
					hitTarget(player, team1player2);
			}
			else if (player->getMode() == 2)
			{
				if (player->getHp() <= DEF_HEAL_NUM)
				{
					if (isThereCover(&rooms[getRoomNumber(player)]) == true)
					{
						target = getCover(&rooms[getRoomNumber(player)]);
						if (player->getPoint()->getRow() != target->getPoint().getRow() || player->getPoint()->getCol() != target->getPoint().getCol())
						{
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
						}
					}
					else
					{
						if (checkWhoCanBeShot(player) == team1player1->getId())
							hitTarget(player, team1player1);
						else if (checkWhoCanBeShot(player) == team1player2->getId())
							hitTarget(player, team1player2);
						else
						{
							target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
						}
					}
				}
				else
				{
					if (checkWhoCanBeShot(player) == team1player1->getId())
						hitTarget(player, team1player1);
					else if (checkWhoCanBeShot(player) == team1player2->getId())
						hitTarget(player, team1player2);
					else
					{
						target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
						goToNext = aStar(player, target);
						player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
					}
				}
			}
			else if (player->getMode() == 3)
			{
				if (player->getHp() <= DEF_NUM)
				{
					if (isThereCover(&rooms[getRoomNumber(player)]) == true)
					{
						target = getCover(&rooms[getRoomNumber(player)]);
						if (player->getPoint()->getRow() != target->getPoint().getRow() || player->getPoint()->getCol() != target->getPoint().getCol())
						{
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
						}
					}
					else
					{
						executeDecision(player, 3);
					}
				}
				else
				{
					if (checkWhoCanBeShot(player) == team1player1->getId())
						hitTarget(player, team1player1);
					else if (checkWhoCanBeShot(player) == team1player2->getId())
						hitTarget(player, team1player2);
					else
					{
						target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
						goToNext = aStar(player, target);
						player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
					}
				}
			}
		}
		else if (checkPlayersInTheSamRoom(player, team1player1) != -1)
		{
			cout << "the player is at the same room as player 1" << endl;
			if (player->getMode() == 1)
			{
				if (checkWhoCanBeShot(player) == team1player1->getId())
					hitTarget(player, team1player1);
			}
			else if (player->getMode() == 2)
			{
				if (player->getHp() <= DEF_HEAL_NUM)
				{
					if (isThereCover(&rooms[getRoomNumber(player)]) == true)
					{
						target = getCover(&rooms[getRoomNumber(player)]);
						if (player->getPoint()->getRow() != target->getPoint().getRow() || player->getPoint()->getCol() != target->getPoint().getCol())
						{
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
						}
					}
					else
					{
						if (checkWhoCanBeShot(player) == team1player1->getId())
							hitTarget(player, team1player1);
						else
						{
							target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
						}
					}
				}
				else
				{
					if (checkWhoCanBeShot(player) == team1player1->getId())
						hitTarget(player, team1player1);
					else
					{
						target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
						goToNext = aStar(player, target);
						player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
					}
				}
			}
			else if (player->getMode() == 3)
			{
				if (player->getHp() <= DEF_NUM)
				{
					if (isThereCover(&rooms[getRoomNumber(player)]) == true)
					{
						target = getCover(&rooms[getRoomNumber(player)]);
						if ((player->getPoint()->getRow() != target->getPoint().getRow()) || (player->getPoint()->getCol() != target->getPoint().getCol()))
						{
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
						}
					}
					else
					{
						executeDecision(player, 3);
					}
				}
				else
				{
					if (checkWhoCanBeShot(player) == team1player1->getId())
						hitTarget(player, team1player1);
					else
					{
						target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
						goToNext = aStar(player, target);
						player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
					}
				}
			}
		}
		else if (checkPlayersInTheSamRoom(player, team1player2) != -1)
		{
			cout << "the player is at the same room as player 2" << endl;
			if (player->getMode() == 1)
			{
				if (checkWhoCanBeShot(player) == team1player2->getId())
					hitTarget(player, team1player2);
				else
				{
					target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
					goToNext = aStar(player, target);
					player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
				}
			}
			else if (player->getMode() == 2)
			{
				if (player->getHp() <= DEF_HEAL_NUM)
				{
					if (isThereCover(&rooms[getRoomNumber(player)]) == true)
					{
						target = getCover(&rooms[getRoomNumber(player)]);
						if (player->getPoint()->getRow() != target->getPoint().getRow() || player->getPoint()->getCol() != target->getPoint().getCol())
						{
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
						}
					}
					else
					{
						if (checkWhoCanBeShot(player) == team1player2->getId())
							hitTarget(player, team1player2);
						else
						{
							target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
						}
					}
				}
				else
				{
					if (checkWhoCanBeShot(player) == team1player2->getId())
						hitTarget(player, team1player2);
					else
					{
						target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
						goToNext = aStar(player, target);
						player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
					}
				}
			}
			else if (player->getMode() == 3)
			{
				if (player->getHp() <= DEF_NUM)
				{
					if (isThereCover(&rooms[getRoomNumber(player)]) == true)
					{
						target = getCover(&rooms[getRoomNumber(player)]);
						if ((player->getPoint()->getRow() != target->getPoint().getRow()) || (player->getPoint()->getCol() != target->getPoint().getCol()))
						{
							goToNext = aStar(player, target);
							player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
						}
					}
					else
					{
						executeDecision(player, 3);
					}
				}
				else
				{
					if (checkWhoCanBeShot(player) == team1player2->getId())
						hitTarget(player, team1player2);
					else
					{
						target = getclosetNodeFromOpposingTeam(player->getBelongsToTeam());
						goToNext = aStar(player, target);
						player->setPoint(new Point2D(goToNext->getPoint().getRow(), goToNext->getPoint().getCol()));
					}
				}
			}
		}
		else
		{
			cout << "keep searching " << endl;
			target = getNextStepLookingForTheEnemy(player);
			player->setPoint(new Point2D(target->getPoint().getRow(), target->getPoint().getCol()));
			player->addToVisited(getRoomNumber(player));
		}
	}
}

bool isThereCover(Room* roomIn)
{
	int i, j;
	for (i = roomIn->getLeftTop().getRow(); i <= roomIn->getRightBottom().getRow(); i++)
	{
		for (j = roomIn->getLeftTop().getCol(); j <= roomIn->getRightBottom().getCol(); j++)
		{
			if (maze[i][j].getDangerLevel() == 0)
				return true;
		}
	}
	return false;
}

// Looking for a safe place in the room and if there is then this func returns it
Node* getCover(Room* roomIn)
{
	int i, j;
	for (i = roomIn->getLeftTop().getRow(); i <= roomIn->getRightBottom().getRow(); i++)
	{
		for (j = roomIn->getLeftTop().getCol(); j <= roomIn->getRightBottom().getCol(); j++)
		{
			if (maze[i][j].getDangerLevel() == 0)
				return &maze[i][j];
		}
	}
	return nullptr;
}
Node* getCloserHealSpot(Player* playerInTurn)
{
	Node* chosenHealSpot = healNodes[0];
	double minDistance = sqrt(pow(playerInTurn->getPoint()->getCol() - healNodes[0]->getPoint().getCol(), 2) +
		pow(playerInTurn->getPoint()->getRow() - healNodes[0]->getPoint().getRow(), 2));
	double tempDistance;
	int i;
	for (i = 1; i < NUM_OF_HEAL_SPOT; i++)
	{
		tempDistance = sqrt(pow(playerInTurn->getPoint()->getCol() - healNodes[i]->getPoint().getCol(), 2) +
			pow(playerInTurn->getPoint()->getRow() - healNodes[i]->getPoint().getRow(), 2));
		if (tempDistance < minDistance)
		{
			chosenHealSpot = healNodes[i];
		}
	}
	return chosenHealSpot;
}
Node* getCloserAmmoSpot(Player* playerInTurn)
{
	Node* chosenAmmoSpot = ammoNodes[0];
	double minDistance = sqrt(pow(playerInTurn->getPoint()->getCol() - ammoNodes[0]->getPoint().getCol(), 2) +
		pow(playerInTurn->getPoint()->getRow() - ammoNodes[0]->getPoint().getRow(), 2));
	double tempDistance;
	int i;
	for (i = 1; i < NUM_OF_HEAL_SPOT; i++)
	{
		tempDistance = sqrt(pow(playerInTurn->getPoint()->getCol() - ammoNodes[i]->getPoint().getCol(), 2) +
			pow(playerInTurn->getPoint()->getRow() - ammoNodes[i]->getPoint().getRow(), 2));
		if (tempDistance < minDistance)
		{
			chosenAmmoSpot = ammoNodes[i];
		}
	}
	return chosenAmmoSpot;
}

//searching for the closest enemy
Node* getclosetNodeFromOpposingTeam(int yourTeam)
{
	double distanceFromPlayer1, distanceFromPlayer2;
	Node* minDistanceNode = &maze[0][0];
	double minDistance = sqrt(pow(maze[MSZ - 1][MSZ - 1].getPoint().getCol() - maze[0][0].getPoint().getCol(), 2) +
		pow(maze[MSZ - 1][MSZ - 1].getPoint().getRow() - maze[0][0].getPoint().getRow(), 2)) * 2;
	int i, j;
	if (yourTeam == 1)
	{
		for (i = 0; i < MSZ; i++)
		{
			for (j = 0; j < MSZ; j++)
			{
				if (maze[i][j].GetValue() == SPACE)
				{
					distanceFromPlayer1 = sqrt(pow(team2player1->getPoint()->getCol() - maze[i][j].getPoint().getCol(), 2) +
						pow(team2player1->getPoint()->getRow() - maze[i][j].getPoint().getRow(), 2));
					distanceFromPlayer2 = sqrt(pow(team2player2->getPoint()->getCol() - maze[i][j].getPoint().getCol(), 2) +
						pow(team2player2->getPoint()->getRow() - maze[i][j].getPoint().getRow(), 2));
					if (distanceFromPlayer1 + distanceFromPlayer2 < minDistance)
					{
						minDistanceNode = &maze[i][j];
					}
				}
			}
		}
	}
	else
	{
		for (i = 0; i < MSZ; i++)
		{
			for (j = 0; j < MSZ; j++)
			{
				if (maze[i][j].GetValue() == SPACE)
				{
					distanceFromPlayer1 = sqrt(pow(team1player1->getPoint()->getCol() - maze[i][j].getPoint().getCol(), 2) +
						pow(team1player1->getPoint()->getRow() - maze[i][j].getPoint().getRow(), 2));
					distanceFromPlayer2 = sqrt(pow(team1player2->getPoint()->getCol() - maze[i][j].getPoint().getCol(), 2) +
						pow(team1player2->getPoint()->getRow() - maze[i][j].getPoint().getRow(), 2));
					if (distanceFromPlayer1 + distanceFromPlayer2 < minDistance)
					{
						minDistanceNode = &maze[i][j];
					}
				}
			}
		}
	}
	return minDistanceNode;
}

// check if the node is a player (generally)
bool checkOtherPlayerOnNode(Node* node)
{
	if (team1player1->getPoint()->getRow() == node->getPoint().getRow() && team1player1->getPoint()->getCol() == node->getPoint().getCol())
	{
		return true;
	}
	else if (team1player2->getPoint()->getRow() == node->getPoint().getRow() && team1player2->getPoint()->getCol() == node->getPoint().getCol())
	{
		return true;
	}
	else if (team2player1->getPoint()->getRow() == node->getPoint().getRow() && team2player1->getPoint()->getCol() == node->getPoint().getCol())
	{
		return true;
	}
	else if (team2player2->getPoint()->getRow() == node->getPoint().getRow() && team2player2->getPoint()->getCol() == node->getPoint().getCol())
	{
		return true;
	}
	return false;
}

bool checkBulletHit(Bullet* bt, Player* targetplayer)
{
	while (bt->simulateMove(maze) == true)
	{
		if (targetplayer->checkIfHit(bt) == true)
		{
			return true;
		}
	}
	return false;
}

//check Who Can Be Shooted
int checkWhoCanBeShot(Player* player)
{
	bool player1CanBeShot = false;
	bool player2CanBeShot = false;
	Bullet* bt = new Bullet(player->calcX(), player->calcY());
	if (player->getBelongsToTeam() == 1)
	{
		bt->SetDir(player->getAngle(team2player1));
		player1CanBeShot = checkBulletHit(bt, team2player1);
		delete bt;
		bt = new Bullet(player->calcX(), player->calcY());
		bt->SetDir(player->getAngle(team2player2));
		player2CanBeShot = checkBulletHit(bt, team2player2);
		//the case of two enemy players in the same room so i choose to shoot on the player with the lowest hp
		if (player1CanBeShot == true && player2CanBeShot == true)
		{
			if (0 < team2player1->getHp() <= team2player2->getHp())
			{
				return team2player1->getId();
			}
			else if (0 < team2player2->getHp() <= team2player1->getHp())
			{
				return team2player2->getId();
			}
		}
		// the case if player 1 from the opposite team in my room 
		else if (player1CanBeShot == true)
		{
			return team2player1->getId();
		}
		// the case if player 2 from the opposite team in my room
		else if (player2CanBeShot == true)
		{
			return team2player2->getId();
		}
		else
		{
			return 0;
		}
	}
	else if (player->getBelongsToTeam() == 2)
	{
		bt->SetDir(player->getAngle(team1player1));
		player1CanBeShot = checkBulletHit(bt, team1player1);
		delete bt;
		bt = new Bullet(player->calcX(), player->calcY());
		bt->SetDir(player->getAngle(team1player2));
		player2CanBeShot = checkBulletHit(bt, team1player2);
		// the case of two enemy players in the same room so i choose to shoot on the player with the lowest hp
		if (player1CanBeShot == true && player2CanBeShot == true)
		{
			if (0 < team1player1->getHp() <= team1player2->getHp())
			{
				return team1player1->getId();
			}
			else if (0 < team1player2->getHp() <= team1player1->getHp())
			{
				return team1player2->getId();
			}
		}
		// the case if player 1 from the opposite team in my room 
		else if (player1CanBeShot == true)
		{
			return team1player1->getId();
		}
		// the case if player 2 from the opposite team in my room
		else if (player2CanBeShot == true)
		{
			return team1player2->getId();
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

void DrawMaze()
{
	int i, j;
	double sz, x, y;

	for (i = 0; i < MSZ; i++)
		for (j = 0; j < MSZ; j++)
		{
			// set color
			switch (maze[i][j].GetValue())
			{
			case SPACE:
				glColor3d(1, 1, 1); // white
				if (maze[i][j].getSpecialSpot() == HEAL)
					glColor3d(1, 0, 0); //red
				else if (maze[i][j].getSpecialSpot() == AMMO)
					glColor3d(1, 1, 0); //yellow
				break;
			case PATH:
				glColor3d(1, 1, 1); // white
				break;
			case WALL:
				glColor3d(0.4, 0, 0); // dark red
				break;
			case TEAM1PLAYER:
				glColor3d(0, 0, 1); //  blue
				break;
			case TEAM2PLAYER:
				glColor3d(0, 1, 0); // green
				break;
			}
			// draw rectangle
			sz = 2.0 / MSZ;
			x = j * sz - 1;
			y = i * sz - 1;

			glBegin(GL_POLYGON);
			glVertex2d(x, y);
			glVertex2d(x + sz, y);
			glVertex2d(x + sz, y + sz);
			glVertex2d(x, y + sz);

			glEnd();
		}
}

void DrawMap()
{
	int i, j;
	double sz, xx, yy;

	for (i = 0; i < MSZ; i++)
		for (j = 0; j < MSZ; j++)
		{
			if (maze[i][j].GetValue() == SPACE)
			{
				double c;
				c = (double)maze[i][j].getDangerLevel() / 100;// 1(white) is very safe, 0(black) is very dangerous
				glColor3d(c, c, c);
				// draw rectangle
				sz = 2.0 / MSZ;
				xx = (j * sz - 1);
				yy = i * sz - 1;

				glBegin(GL_POLYGON);
				glVertex2d(xx, yy);
				glVertex2d(xx + sz, yy);
				glVertex2d(xx + sz, yy + sz);
				glVertex2d(xx, yy + sz);
				glEnd();
			}
		}
}


bool checkGameEnd()
{
	if ((team1player1->getHp() <= 0 && team1player2->getHp() <= 0) || (team2player1->getHp() <= 0 && team2player2->getHp() <= 0))
	{
		return true;
	}
	else
	{
		return false;
	}
}
void resetMazeDangerLevel()
{
	int i, j;
	for (i = 0; i < MSZ; i++)
	{
		for (j = 0; j < MSZ; j++)
		{
			maze[i][j].setDangerLevel(0, 0, 0);
		}
	}
}

// shoot and check if its hit until the enemy player die or run away
void hitTarget(Player* playerInTurn, Player* player)
{
	double angle = playerInTurn->getAngle(player);
	Bullet* bt = playerInTurn->shoot(angle, maze);
	
	while (player->checkIfHit(bt) == false)
	{
		bt->move(maze);
		displayBullet(bt);
	}
	player->setHp(player->getHp() - bt->getPower());
	if (player->getHp() <= 0)
	{
		player->setPoint(new Point2D(0, 0));
		player->setMode(0);
	}
}


void gameStart()
{
	generatePlayers();
	display();
	int i, j;
	resetMazeDangerLevel();
	while (!checkGameEnd())
	{
		if (team1player1 == nullptr || team1player2 == nullptr || team2player1 == nullptr || team2player2 == nullptr)\
		{
			cout << " " << endl;
		}
		//genarate players start loc 
		if (team2player1->getMode() != 0)
			generateSafetyMapFromPlayer(team2player1);
		if (team2player2->getMode() != 0)
			generateSafetyMapFromPlayer(team2player2);
		if (team1player1->getMode() != 0)
			executeDecision(team1player1, team1player1->decideNextMove());
		if (team1player1->allVisited())
			team1player1->resetVisited();
		if (team1player1 == nullptr || team1player2 == nullptr || team2player1 == nullptr || team2player2 == nullptr)\
		{
			cout << " " << endl;
		}
		display();
		if (team1player2->getMode() != 0)
			executeDecision(team1player2, team1player2->decideNextMove());
		if (team1player2->allVisited())
			team1player2->resetVisited();
		if (team1player1 == nullptr || team1player2 == nullptr || team2player1 == nullptr || team2player2 == nullptr)\
		{
			cout << " " << endl;
		}
		display();
		resetMazeDangerLevel();
		if (team1player1->getMode() != 0)
			generateSafetyMapFromPlayer(team1player1);
		if (team1player2->getMode() != 0)
			generateSafetyMapFromPlayer(team1player2);
		if (team2player1->getMode() != 0)
			executeDecision(team2player1, team2player1->decideNextMove());
		if (team2player1->allVisited())
			team2player1->resetVisited();
		if (team1player1 == nullptr || team1player2 == nullptr || team2player1 == nullptr || team2player2 == nullptr)\
		{
			cout << " " << endl;
		}
		display();
		if (team2player2->getMode() != 0)
			executeDecision(team2player2, team2player2->decideNextMove());
		if (team2player2->allVisited())
			team2player2->resetVisited();
		if (team1player1 == nullptr || team1player2 == nullptr || team2player1 == nullptr || team2player2 == nullptr)\
		{
			cout << " " << endl;
		}
		display();
		resetMazeDangerLevel();
	}
	cout << "game has ended" << endl;


}
void display()
{
	// clean frame buffer
	glClear(GL_COLOR_BUFFER_BIT);

	DrawMaze();

	if (team1player1 != nullptr)
	{
		team1player1->showPlayer();
		team1player2->showPlayer();
		team2player1->showPlayer();
		team2player2->showPlayer();
	}
	// show what was drawn in "frame buffer"
	glutSwapBuffers();
}

void displayBullet(Bullet* bt)
{
	// clean frame buffer
	glClear(GL_COLOR_BUFFER_BIT);
	DrawMaze();
	bt->showMe();
	if (team1player1 != nullptr)
	{
		team1player1->showPlayer();
		team1player2->showPlayer();
		team2player1->showPlayer();
		team2player2->showPlayer();
	}
	// show what was drawn in "frame buffer"
	glutSwapBuffers();
}


void displayMap()
{
	// clean frame buffer
	glClear(GL_COLOR_BUFFER_BIT);
	DrawMaze();
	DrawMap();
	// show what was drawn in "frame buffer"
	glutSwapBuffers();
}

// checks if dx,dy is on SPACE in maze
bool CheckIsSpace(double dx, double dy)
{
	int i, j;
	i = MSZ * (dy + 1) / 2;
	j = MSZ * (dx + 1) / 2;
	return  maze[i][j].GetValue() == SPACE;
}

void idle()
{
	// calls indirectly to display
	glutPostRedisplay();
}


void Menu(int choice)
{
	// start game
	if (choice == 1)
	{
		gameStart();
	}
}


void main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(W, H);
	glutInitWindowPosition(200, 100);
	glutCreateWindow("Dungeon ");
	// refresh function
	glutDisplayFunc(display);
	// idle: when nothing happens
	glutIdleFunc(idle);
	// menu
	glutCreateMenu(Menu);
	glutAddMenuEntry("Start", 1);
	glutAddMenuEntry("Explode", 2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	init();
	glutMainLoop();
}