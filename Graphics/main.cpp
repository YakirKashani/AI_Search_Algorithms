
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "glut.h"
#include <queue>
#include <vector>
#include "Cell.h"
#include <iostream>
#include "Definitions.h"
#include "Node.h"
#include "Edge.h"
#include "CompareNodes.h"
#include "CompareCells.h"

using namespace std;

const int WIDTH = 900;
const int HEIGHT = 900;
const int MSZ = 100; // maze-size

const int NUM_NODES = 400;
const int NUM_EDGES = 1200;

Node nodes[NUM_NODES];
Edge edges[NUM_EDGES];

priority_queue<Node*, vector<Node*>, CompareNodes> pq;
priority_queue<Cell*, vector<Cell*>, CompareCells> BFSpq;
priority_queue<Cell*, vector<Cell*>, CompareCells> AStarPQ;

int maze[MSZ][MSZ] = {0};

bool runBFS = false;
bool runDFS = false;
bool runUCS = false;
bool runBestFirstSearch = false;
bool runAStar = false;

queue <Cell*> grays;
vector<Cell*> dfs_grays;

int startRow, startCol;
int targetRow, targetCol;

void SetupMaze()
{
	int i, j;
	for (i = 0;i < MSZ;i++) { // outer margins
		maze[0][i] = WALL; // first line
		maze[MSZ - 1][i] = WALL; // last line
		maze[i][0] = WALL; // left column
		maze[i][MSZ - 1] = WALL; // right column
	}

	for (i = 1;i < MSZ - 1;i++) { //inner space
		for (j = 1;j < MSZ - 1;j++) {
			if (i % 2 == 1) { //mostly spaces
				if (rand() % 10 > 1) // 80%
					maze[i][j] = SPACE;
				else // 20%
					maze[i][j] = WALL;
			}
			else { //mostly walls
				if (rand() % 10 >= 3) //70%
					maze[i][j] = WALL;
				else // 30%
					maze[i][j] = SPACE;

			}

		}
	}
	startRow = MSZ / 2;
	startCol = MSZ / 2;
	maze[startRow][startCol] = START; // Middle poinmt
	targetRow = rand() % MSZ;
	targetCol = rand() % MSZ;
	maze[targetRow][targetCol] = TARGET; // target point - random
}

double Distance(double x1, double y1, double x2, double y2) {
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

void SetupGraph() {
	//setup random nodes
	int i, index1, index2;
	int max_dist = 13;
	double dist;
	//nodes
	for (i = 0;i < NUM_NODES-1;i++) {
		nodes[i].SetX(rand() % MSZ);
		nodes[i].SetY(rand() % MSZ);
	}
	//define node[0] as START
	nodes[0].SetColor(START);
	nodes[0].SetG(0); //init G of the start node to 0
	pq.push(&nodes[0]); // prepare priority queue (ndoes[0])
	//define node[1] as START
	nodes[1].SetColor(WHITE_TARGET);


	//edges
	for (i = 0;i < NUM_EDGES;i+=2) {
		index1 = rand() % NUM_NODES;
		edges[i].SetNode1(&nodes[index1]);
		do {
			index2 = rand() % NUM_NODES;
			dist = Distance(nodes[index1].GetX(), nodes[index1].GetY(), nodes[index2].GetX(), nodes[index2].GetY());
		} while (dist > max_dist);
		edges[i].SetNode2(&nodes[index2]);
		edges[i].setCost(dist); // update cost
		//add edges in both directions
		edges[i+1].SetNode1(&nodes[index2]);
		edges[i+1].SetNode2(&nodes[index1]);
		edges[i + 1].setCost(dist); // update cost
		// add edges tp putgoing of both nodes
		nodes[index1].AddEdge(&edges[i]);
		nodes[index2].AddEdge(&edges[i + 1]);
	}
}

void init()
{
	glClearColor(0.5,0.5,0.5,0);// color of window background

	glOrtho(0, MSZ, 0, MSZ, -1, 1); // set the coordinates system

	srand(time(0));

	SetupMaze();
	SetupGraph();
}

void ShowMaze() {
	int i, j;
	for (i = 0;i < MSZ;i++) {
		for (j = 0;j < MSZ;j++) {
			// 1. set color of cell
			switch (maze[i][j])
			{
			case SPACE:
				glColor3d(1, 1, 1); //white
				break;
			case WALL:
				glColor3d(0, 0, 0);//black
				break;
			case START:
				glColor3d(0.6, 0.6, 1);//light-blue
				break;
			case TARGET:
				glColor3d(1, 0, 0);//red
				break;
			case PATH:
				glColor3d(1, 0, 1);//magenta
				break;
			case GRAY:
				glColor3d(0, 1, 0);//green
				break;
			case BLACK:
				glColor3d(0.6, 0.6, 0.6);//gray
				break;
			}
			//show cell
			glBegin(GL_POLYGON);
			glVertex2d(j, i);
			glVertex2d(j, i + 1);
			glVertex2d(j + 1, i + 1);
			glVertex2d(j + 1, i);
			glEnd();
		}
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT); // clean frame buffer
	ShowMaze();
	glutSwapBuffers(); // show all
}

void ShowGraph(){
	int i;
	//edges
	for (i = 0;i < NUM_EDGES;i++) {
		edges[i].Show();
	}
	//nodes
	for (i = 0;i < NUM_NODES;i++) {
		nodes[i].Show();
	}
}

void displayGraph()
{
	glClear(GL_COLOR_BUFFER_BIT); // clean frame buffer

	ShowGraph();

	glutSwapBuffers(); // show all
}

void RestorePath(Cell* pc) {
	while (pc != nullptr) {
		maze[pc->getRow()][pc->getCol()] = PATH;
		pc = pc->getParent();
	}
}

bool CheckNeighbor(int row, int col, Cell* pCurrent, bool isBFS) {
	if (maze[row][col] == TARGET) {
		if (isBFS) {
			runBFS = false;
		}
		else {
			runDFS = false;
		}
		cout << "The solution has been found.\n";
		RestorePath(pCurrent);
		return false;
	}
	else { // maze[row][col] must be SPACE (WHITE)
		Cell* pc = new Cell(row, col, pCurrent);
		maze[row][col] = GRAY;
		if (isBFS)
			grays.push(pc);
		else
			dfs_grays.push_back(pc);
		return true;
	}
}

bool CheckNeighborBestFirstSearch(int row, int col, Cell* pCurrent) {
	if (maze[row][col] == TARGET) {
		runBestFirstSearch = false;
		cout << "The solution has been found.\n";
		RestorePath(pCurrent);
		return false;
	}
	else {
		Cell* pc = new Cell(row, col, pCurrent);
		pc->setH(Distance(row, col, targetRow, targetCol));
		pc->setF(pc->getH()); // F = H
		maze[row][col] = GRAY;
		BFSpq.push(pc);
		return true;
	}
}

bool CheckNeighborAStar(int row, int col, Cell* pCurrent) {
	if (maze[row][col] == TARGET) {
		runAStar = false;
		cout << "Solution found\n";
		RestorePath(pCurrent);
		return false;
	}
	else {
		Cell* neighbor = new Cell(row, col, pCurrent);
		neighbor->setG(Distance(row, col,startRow,startCol));
		neighbor->setH(Distance(row, col, targetRow, targetCol));
		neighbor->setF(neighbor->getG() + neighbor->getH()); // F = G + H

		maze[row][col] = GRAY;
		AStarPQ.push(neighbor);
		return true;
	}
}

void RunBFSIteration() {
	Cell* pCurrent;
	int row, col;
	bool go_on = true;

	if (grays.empty())
	{
		runBFS = false;
		cout << "There is no solution. Grays is empty\n";
		return;
	}
	else { // grays is not empty
		pCurrent = grays.front();
		grays.pop(); // extract the first elemnt from grays
		// 1. paint pCurrent black
		row = pCurrent->getRow();
		col = pCurrent->getCol();
		if (maze[row][col] != START)
			maze[row][col] = BLACK;
		// 2. check all the neighbors of pCurrent
		// go up
		if (maze[row + 1][col] == SPACE || maze[row + 1][col] == TARGET) {
			go_on = CheckNeighbor(row + 1, col, pCurrent,true);
		}
		//down
		if (go_on && (maze[row - 1][col] == SPACE || maze[row - 1][col] == TARGET)) {
			go_on = CheckNeighbor(row - 1, col, pCurrent,true);
		}
		//left
		if (go_on && (maze[row][col - 1] == SPACE || maze[row][col - 1] == TARGET)) {
			go_on = CheckNeighbor(row, col - 1, pCurrent,true);
		}
		//right
		if (go_on && (maze[row][col + 1] == SPACE || maze[row][col + 1] == TARGET)) {
			go_on = CheckNeighbor(row, col + 1, pCurrent,true);
		}
	}
}

void RunDFSIteration() {
	Cell* pCurrent;
	int row, col;
	bool go_on = true;
	if (dfs_grays.empty()) {
		runDFS = false;
		cout << "There is no solution. Grays is empty\n";
		return;
	}
	else {
		pCurrent = dfs_grays.back();
		dfs_grays.pop_back();
		//1. Paint pCurrent black
		row = pCurrent->getRow();
		col = pCurrent->getCol();
		if (maze[row][col] != START)
			maze[row][col] = BLACK;
		//2. Add all white or target neighbors dfs_grays
		// What is the order of neighbors? Is the order matter?
		// Let's make it random
		int directions[4] = { IS_FREE,IS_FREE,IS_FREE,IS_FREE };
		int index;
		for (int dir = 0;dir < 4;dir++) {
			do {
				index = rand() % 4;
			} while (directions[index] != IS_FREE);
			directions[index] = dir;
		}
		//now let's check random directions
		for (int dir = 0;dir < 4 && go_on;dir++) {
			switch (directions[dir]) {
			case UP:
				// try to go up
				if (go_on && maze[row + 1][col] == SPACE || maze[row + 1][col] == TARGET) {
					go_on = CheckNeighbor(row + 1, col, pCurrent, false);
				}
				break;
			case DOWN:
				// try to go down
				if (go_on && (maze[row - 1][col] == SPACE || maze[row - 1][col] == TARGET)) {
					go_on = CheckNeighbor(row - 1, col, pCurrent, false);
				}
				break;
			case LEFT:
				// try to go left
				if (go_on && (maze[row][col - 1] == SPACE || maze[row][col - 1] == TARGET)) {
					go_on = CheckNeighbor(row, col - 1, pCurrent, false);
				}
				break;
			case RIGHT:
				// try to go right
				if (go_on && (maze[row][col + 1] == SPACE || maze[row][col + 1] == TARGET)) {
					go_on = CheckNeighbor(row, col + 1, pCurrent, false);
				}
				break;
			}
		}
	}
}

// note: pq is passed by reference
void UpdateG(Node* pNeighbor,
	priority_queue<Node*, vector<Node*>, CompareNodes>& pq)
{
	vector<Node*> tmp;
	while (pq.top() != pNeighbor) {
		tmp.push_back(pq.top());
		pq.pop();
	}
	//now pq.top() is pNeighbor
	pq.pop();
	pq.push(pNeighbor);
	while (!tmp.empty()) {
		pq.push(tmp.back());
		tmp.pop_back();
	}
}

void RestorePathInGraph(Node* pn) {
	while (pn != nullptr) {
		pn->SetColor(PATH);
		pn = pn->GetParent();
	}
}

void RunUCSIteration() {
	Node* pCurrent;
	Node* pNeighbor;
	double NewG;
	if (pq.empty()) {
		runUCS = false;
		cout << "There is no solution\n";
		return;
	}
	else { //pq is not empty: pick next
		pCurrent = pq.top();
		//check for success: the first node in pq is GRAY_TARGET
		if (pCurrent->GetColor() == GRAY_TARGET) {
			runUCS = false;
			cout << "The solution has been found!\n";
			RestorePathInGraph(pCurrent);
			return;
		}
		pq.pop(); //remove the node from pq
		if(pCurrent->GetColor()!=START)
			pCurrent->SetColor(BLACK);
		// scan the neighbors of pCurrent
		for (auto it : pCurrent->GetOutgoing()) // it is ierator the iterates over outgoing
		{
			pNeighbor = it->GetNode2();
			switch (pNeighbor->GetColor()) {
			case WHITE:
				pNeighbor->SetColor(GRAY);
				pNeighbor->SetG(pCurrent->GetG() + it->GetCost());
				pNeighbor->SetParent(pCurrent);
				pq.push(pNeighbor);
				break;
			case GRAY:
				NewG = pCurrent->GetG() + it->GetCost();
				if (NewG < pNeighbor->GetG()) // update G of pNeighbor
				{
					pNeighbor->SetG(NewG);
					pNeighbor->SetParent(pCurrent);
					UpdateG(pNeighbor, pq);
				}
				break;
			case WHITE_TARGET:
				pNeighbor->SetColor(GRAY_TARGET);
				pNeighbor->SetG(pCurrent->GetG() + it->GetCost());
				pNeighbor->SetParent(pCurrent);
				pq.push(pNeighbor);
				break;
			}
		}
	}
}

void RunBestFirstSearchIteration() {
	Cell* pCurrent;
	int row, col;
	bool go_on = true;

	if (BFSpq.empty()) {
		runBestFirstSearch = false;
		cout << "There is no solution. Priority queue is empty.\n";
		return;
	}
	else {
		pCurrent = BFSpq.top();
		BFSpq.pop();
		row = pCurrent->getRow();
		col = pCurrent->getCol();
		if (maze[row][col] != START) //Paint pCurrent BLACK
			maze[row][col] = BLACK;
		if (maze[row + 1][col] == SPACE || maze[row + 1][col] == TARGET) {
			go_on = CheckNeighborBestFirstSearch(row + 1, col, pCurrent);
		}
		if (go_on && (maze[row - 1][col] == SPACE || maze[row - 1][col] == TARGET)) {
			go_on = CheckNeighborBestFirstSearch(row - 1, col, pCurrent);
		}
		if (go_on && (maze[row][col-1] == SPACE || maze[row][col-1] == TARGET)) {
			go_on = CheckNeighborBestFirstSearch(row, col-1, pCurrent);
		}
		if (go_on && (maze[row][col + 1] == SPACE || maze[row][col + 1] == TARGET)) {
			go_on = CheckNeighborBestFirstSearch(row, col + 1, pCurrent);
		}
	}
}

void RunAStarIteration() {
	Cell* pCurrent;
	int row, col;
	bool go_on = true;

	if (AStarPQ.empty()) {
		runAStar = false;
		cout << "No solution. Priority queue is empty.\n";
		return;
	}
	else {
		pCurrent = AStarPQ.top();
		AStarPQ.pop();
		row = pCurrent->getRow();
		col = pCurrent->getCol();
		if (maze[row][col] != START)
			maze[row][col] = BLACK;
		if (maze[row + 1][col] == SPACE || maze[row + 1][col] == TARGET) {
			go_on = CheckNeighborAStar(row + 1, col, pCurrent);
		}
		if (go_on && (maze[row - 1][col] == SPACE || maze[row - 1][col] == TARGET)) {
			go_on = CheckNeighborAStar(row - 1, col, pCurrent);
		}
		if (go_on && (maze[row][col - 1] == SPACE || maze[row][col - 1] == TARGET)) {
			go_on = CheckNeighborAStar(row, col - 1, pCurrent);
		}
		if (go_on && (maze[row][col + 1] == SPACE || maze[row][col + 1] == TARGET)) {
			go_on = CheckNeighborAStar(row, col + 1, pCurrent);
		}
	}
}

void idle() // always running
{
	if (runBFS)
		RunBFSIteration();
	if(runDFS)
		RunDFSIteration();
	if (runUCS)
		RunUCSIteration();
	if (runBestFirstSearch)
		RunBestFirstSearchIteration();
	if (runAStar)
		RunAStarIteration();
	glutPostRedisplay(); // indirect call to display

}

void menu(int choice) {
	Cell* pc = new Cell(MSZ / 2, MSZ / 2, nullptr);
	switch (choice)
	{
	case 1: //BFS
		glutDisplayFunc(display);
		grays.push(pc);
		runBFS = true;
		break;
	case 2: //DFS
		glutDisplayFunc(display);
		dfs_grays.push_back(pc);
		runDFS = true;
		break;
	case 3: //UCS
		glutDisplayFunc(displayGraph);
		break;
	case 4: //Best First Search
		glutDisplayFunc(display);
		BFSpq.push(pc);
		runBestFirstSearch = true;
		break;
	case 5: //A*
		glutDisplayFunc(display);
		AStarPQ.push(pc);
		runAStar = true;
		break;
	}
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == ' ')
		runUCS = true;
}

void main(int argc, char* argv[]) 
{
	glutInit(&argc, argv);
	// definitions for visual memory (Frame buffer) and double buffer
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(400, 100);
	glutCreateWindow("BFS - DFS");
	// display is a refresh function
	glutDisplayFunc(display);
	// idle is a update function
	glutIdleFunc(idle);

	glutKeyboardFunc(keyboard);

	//add menu
	glutCreateMenu(menu);
	glutAddMenuEntry("BFS", 1);
	glutAddMenuEntry("DFS", 2);
	glutAddMenuEntry("Uniform Cost Search", 3);
	glutAddMenuEntry("Best First Search", 4);
	glutAddMenuEntry("A*", 5);
	

	glutAttachMenu(GLUT_RIGHT_BUTTON);

	init();

	glutMainLoop();
}