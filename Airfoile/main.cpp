/*
--- 기초 ---
Array로 하면 편하겠지만, 그럴 경우 Memory 초과가 난다는 박사님의 말에 따라
Class로 구현함.

정사각형을 Cell, 사각형을 이루는 변을 Face, 각 꼭지점을 Node라 정의함.

--- 전체 흐름 설명 ---
각 네모를 Cell이라 부르며
네모를 이루는 상/하/좌/우의 벽을 Face라 부른다.
네모의 각 꼭지점을 Node라 부른다.

편의를 위해, 전체 네모가 Cell들을 참조할 수 있다.( Rectangle->initCell[i][j] )
Cell에서 Face 밑 Node를 접근할 수 있다.
( Cell-> LFace,  DFace,  RFace,  UFace );
( Cell->LUNode, LDNode, RDNode, RUNode );

Face에서 인접해있는 Cell과 Node를 접근할 수 있다.
( Face-> sNode, eNode [startNode, endNode의 줄임말] )

--- 출력 순서와 관련된 부분 ---
각 부분에 대해, 파일로 출력할 때, 먼저 출력되는 부분에 대한 설정이 필요하다.
특히 Face를 출력할 때 sNode, eNode, sCell, eCell 순서로 출력하는 부분이 있는데,
기역자(?)로 항상 그리면서 출력하는 내용은 박사님께 직접 전해듣기를 바란다.

판을 그릴 때, (0, 0)에 가까운 부분을 start로 하고, (width, height)에 가까울수록 end로 설정한다.
출력할 때, 끝 부분이 NULL인 경우에만 순서를 거꾸로 출력하도록 설정하였다.
예를 들어, Face(변)에 대해 생각해보면
(0, 0) ~ (100, 0)을 잇는 Face가 있다고 생각할 때,
그 Face의 sNode는 (0, 0)이고, eNode는 (100, 0)이다.

본인은, 고려하기 쉽게 좌표평면상의 점으로 생각한 후 작성하였다.


전체 판을 만들기 위해
1. 가로, 세로 크기에 대한 입력을 받는다.
2. 가로/세로 GCD를 구해서, 그 GCD에 맞도록 판을 쪼갠다.
3. 각 Cell에 대한 정보를 입력한다.
*/
#include <iostream>
#include <vector>
#include <cstdio>
#include <algorithm>
#include <cstdio>
#include <queue>
#include <set>
#include <map>
#include <cmath>

#pragma warning(disable:4996)

#define EPSILON 0.00001 // 재: double 비교

using namespace std;
class CellShape; // 재
class Cell; // 재
class CellRectangle; // 재
class CellTriangle; // 재
class Face;
class Node;
class Rectangle;

typedef pair<double, double> pdd;//진
typedef pair<double, int> pdi;
typedef pair<pair<int, int>, pair<int, int> > fi;
typedef pair<int, Face*> detface;//진 delete
typedef pair<int, Cell*> detcell;//진 delete
typedef pair<int, CellShape*> node_of_cell;//진:tail

vector<Node*> nodeVector;
vector<Cell*> cellVector;
vector<Face*> faceVector;
vector<Face*> wallVector; //0208
vector<fi>wall; // < <삼각형인지 사각형인지 , 도형의 id > < snode, enode> >


Rectangle *initRectangle;
vector<pdd> mininode[2];//진 //vector<CellShape*> minimalCellVector_rec;
vector<CellShape*> minimalCellVector_tri; // 재 : 이 곳에 CellRectangle과 CellTriangle 삽입 2017-01-12
map< pdd, pair< int, void* > > mininodePlace; // 재 : mininode를 포함한 face나 node의 주소를 저장한다.( 0 : Node, 1 : Face) 2017-01-12
map< pdd, map < pdd, CellShape* > > mininodeCell; // 재 : map 2017-01-12
vector<detface> detfaceVector;
vector<detcell> detcellVector;
//vector<int> detinitcellVector; // 재 : 2017_03_17

vector< pair<pdd, pdd> > inputLine; // 재 : 입력받는 선들을 저장 2017_02_08
int initCellMaxID = 0; // 재 : initCell에 대한 접근 방지용, 어디까지 initCell인지 저장 2017_02_19
double  S_N = 0.000000001;
FILE *fp; // 선언만 하고 메인에서 인스턴스화

bool double_Compare(double d1, double d2) {
	return fabs(d1 - d2) < EPSILON;
}
class Node {
public:
	double x, y;
	int id;
	static int nodeCount; // node 개수를 count
	int inout; // 재 :
			   /* attribute
			   -2: line (도형위)
			   -1: inner (도형의 안쪽)
			   0 : outflow  (맨 오른쪽)
			   1 : inlet    (맨 왼쪽)
			   2 : top      (맨 위)
			   3 : bottom   (맨 아래)
			   4 : interrior(중간 Node들)
			   5 : unknown (미확인)
			   */
public:
	Node() {
		x = -99999;
		y = -99999;
		id = ++nodeCount;
		inout = 5; // 재 : 초기화 2017_02_13
	}
	Node(double x, double y) {
		this->x = x; this->y = y;
		id = ++nodeCount;
		inout = 5; // 재 : inout 초기화, 기본적으론 5로 되어있음 2017_02_05
	}
	~Node() {
		nodeCount--;
	}
	void setPoint(double x, double y) {
		this->x = x;
		this->y = y;
	}
	double getX() {
		return x;
	}
	double getY() {
		return y;
	}
	int getID() {
		return id;
	}
	int getCount() {
		return nodeCount;
	}
	int getInout() { // 재 : inout 추가로 인한 함수 추가2017_02_05
		return inout;
	}
	bool operator==(Node* n) { // 재 : node와 node의 비교 2017-01-12
		if (x == n->getX()) {
			if (y == n->getY()) {
				if (id == n->getID())
					return true;
			}
		}
		return false;
	}
	node_of_cell findtailCell(pdd n); // 진 : tail 설정  2017-03-07
	CellShape* findCell(Node* n); // 재 : Node와 Node일때 mininodeCell 설정 2017-01-12
	CellShape* findCell(Face* f); // 재 : Node와 face일때 mininodeCell 설정 2017-01-12
};
class Face {
public:
	Node * sNode, *eNode, *centerNode;
	CellShape *sCell, *eCell; // 재 : Cell => CellShape
	Face *Child1, *Child2, *curParent;
	int id, attribute;
	int depth;
	int printID, myPrintID;
	bool childOrNot;
	/* attribute
	0 : outflow  (맨 오른쪽)
	1 : inlet    (맨 왼쪽)
	2 : top      (맨 위)
	3 : bottom   (맨 아래)
	4 : interrior(중간 Face들) */
	static int faceCount;
	static int maxDepth;
public:
	Face() {
		sNode = eNode = centerNode = NULL;
		sCell = eCell = NULL;
		Child1 = Child2 = curParent = NULL;
		id = ++faceCount;
		attribute = 4; // default = interrior
		depth = 0;
		printID = 0;
		childOrNot = false;
	}
	Face(Node *sNode, Node *eNode) {
		centerNode = NULL;
		this->sNode = sNode; this->eNode = eNode;
		sCell = eCell = NULL;
		Child1 = Child2 = curParent = NULL;
		id = ++faceCount;
		attribute = 4; // default = interrior
		depth = 0;
		printID = 0;
		childOrNot = false;
	}
	~Face() { // delete face
		faceCount--;
		faceVector.erase(faceVector.begin() + id - 1);
	}
	void setAttribute(int attribute) {
		this->attribute = attribute;
	}
	void setStartCell(CellShape *cell) { // 재 : Cell => CellShape 2017-01-12
		this->sCell = cell;
	}
	void setEndCell(CellShape *cell) { // 재 : Cell => CellShape 2017-01-12
		this->eCell = cell;
	}
	void setStartNode(Node *node) {
		this->sNode = node;
	}
	void setEndNode(Node *node) {
		this->eNode = node;
	}
	void setCenterNode(Node *node) {
		centerNode = node;
	}
	void setChilds(Face *c1, Face *c2) {
		Child1 = c1;
		Child2 = c2;

		int childDepth = depth + 1;
		c1->depth = childDepth;    c1->setParent(this);
		c2->depth = childDepth;    c2->setParent(this);

		if (childDepth > maxDepth) {
			maxDepth = childDepth;
		}
	}
	void setID(int newID) {
		id = newID;
	}
	void setDepth(int depth) {
		this->depth = depth;
	}
	void setPrintID(int newID) {
		this->printID = newID;
	}
	void setMyPrintID(int newID) {
		myPrintID = newID;
	}
	void setParent(Face *parent) {
		curParent = parent;
	}
	int getMyPrintID() {
		return myPrintID;
	}
	int getMaxDepth() {
		return maxDepth;
	}
	int getPrintID() {
		return this->printID;
	}
	int getID() {
		if (id>0)return id;
		return NULL;
	}
	int getAttribute() {
		return attribute;
	}
	int getCount() {
		return faceCount;
	}
	Node* getStartNode() {
		return sNode;
	}
	Node* getEndNode() {
		return eNode;
	}
	Node* getCenterNode() {
		return centerNode;
	}
	CellShape* getStartCell() { // 재 : CellShape로 반환값 변경  2017-01-12
		return sCell;
	}
	CellShape* getEndCell() { // 재 : CellShape로 반환값 변경  2017-01-12
		return eCell;
	}
	Face* getChild1() {
		return Child1;
	}
	Face* getChild2() {
		return Child2;
	}
	int getDepth() {
		return depth;
	}
	bool isParent() {
		return ((Child1 != NULL) && (Child2 != NULL));
	}
	bool isChild() {
		return curParent != NULL;
	}
	bool operator==(Face& f) { // 재 : face와 face의 일치 비교 2017-01-12
		if (this->sNode == f.getStartNode()) {
			if (this->eNode == f.getEndNode()) {
				if (this->id == f.getID()) {
					if (this->depth == f.getDepth())
						return true;
				}
			}
		}
		return false;
	}
	CellShape* findCell(Node* n); // 재 : face와 Node일때 mininodeCell 설정 2017-01-12
	CellShape* findCell(Face* f); // 재 : face와 face일때 mininodeCell 설정 2017-01-12
};
class CellShape { // 재 : Cell 기본형
public:
	double cellSize;
	int id, depth;
	static int cellCount;
	static int triCount; //0208
	bool iscut;

	int getID() {
		if (id>0)return id;
		return NULL;
	}
	void setID(int newID) {
		this->id = newID;
	}
};

class Cell : public CellShape { // 재 : 수정하였음 2017-01-12
public:
	Face * LFace, *DFace, *RFace, *UFace;
	Node *LUNode, *LDNode, *RDNode, *RUNode;
	CellShape *LUCell, *LDCell, *RDCell, *RUCell;
	void setChildNodes(Node *L, Node *D, Node *R, Node *U, Node *C, int partialtype) { // 진: tail처리_수정 0307
		if (partialtype == 0 || partialtype == 3) {
			((Cell*)LUCell)->setLUNode(LUNode);     ((Cell*)LUCell)->setLDNode(L);
			((Cell*)LUCell)->setRDNode(C);         ((Cell*)LUCell)->setRUNode(U);
		}
		if (partialtype == 0 || partialtype == 2) {
			((Cell*)LDCell)->setLUNode(L);         ((Cell*)LDCell)->setLDNode(LDNode);
			((Cell*)LDCell)->setRDNode(D);         ((Cell*)LDCell)->setRUNode(C);
		}
		if (partialtype != 3) {
			((Cell*)RDCell)->setLUNode(C);         ((Cell*)RDCell)->setLDNode(D);
			((Cell*)RDCell)->setRDNode(RDNode);     ((Cell*)RDCell)->setRUNode(R);
		}
		if (partialtype != 2) {
			((Cell*)RUCell)->setLUNode(U);         ((Cell*)RUCell)->setLDNode(C);
			((Cell*)RUCell)->setRDNode(R);         ((Cell*)RUCell)->setRUNode(RUNode);
		}
	}
	void setNewFaces(Face *from, Face *child1, Face *child2, Node *Start, Node *Center, Node *End) {
		if (!from->isParent()) {
			child1 = new Face(Start, Center);
			child2 = new Face(Center, End);
			faceVector.push_back(child1);
			faceVector.push_back(child2);
			child1->setAttribute(LFace->attribute);
			/*
			왜 Lface 의 attribute를 따르는지
			*/
			child2->setAttribute(LFace->attribute);
			from->setChilds(child1, child2);
			from->setCenterNode(Center);
		}
		else {
			child1 = from->Child1;
			child2 = from->Child2;
		}
		child1->curParent = from;
		child2->curParent = from;
	}
	void setupMininodePlace(double x, double y) { // 재 : dotpoint함수가 너무 길어지는 것 같아서 분리시켰습니다. 2017-01-12
												  // mininodePlace[make_pair(x, y)] = pair< 0 || 1, void*>; // 재 : map[x,y] = 0과 Node의 주소 또는 1과 Face의 주소.
												  // 0 : Node, 1 : Face
		if (LDNode->getX() == x) { // 재 : x, y가 왼쪽 LFace에 있을때
			if (LDNode->getY() == y)
				mininodePlace[make_pair(x, y)] = make_pair(0, LDNode);
			else if (LUNode->getY() == y)
				mininodePlace[make_pair(x, y)] = make_pair(0, LUNode);
			else
				mininodePlace[make_pair(x, y)] = make_pair(1, LFace);

		}
		else if (LDNode->getY() == y) { // 재 : x, y가  DFace에 있을때
			if (RDNode->getX() == x)
				mininodePlace[make_pair(x, y)] = make_pair(0, RDNode);
			else
				mininodePlace[make_pair(x, y)] = make_pair(1, DFace);
		}
		else if (RDNode->getX() == x) { // 재 : x, y가 왼쪽 RFace에 있을때
			if (RUNode->getY() == y)
				mininodePlace[make_pair(x, y)] = make_pair(0, RUNode);
			else
				mininodePlace[make_pair(x, y)] = make_pair(1, RFace);
		}
		else if (RUNode->getY() == y) { // 재 : x, y가 왼쪽 UFace에 있을때
			if (LUNode->getX() == x)
				mininodePlace[make_pair(x, y)] = make_pair(0, LUNode);
			else
				mininodePlace[make_pair(x, y)] = make_pair(1, UFace);
		}
	}
public:
	Cell() {
		LUCell = LDCell = RDCell = RUCell = NULL;
		LUNode = LDNode = RDNode = RUNode = NULL;
		LFace = DFace = RFace = UFace = NULL;
		id = ++cellCount;
		depth = 0;
		iscut = false;
	}
	~Cell() {//진delete
		cellCount--;
		cellVector.erase(cellVector.begin() + id - 1);
	}
	bool getiscut() {
		return iscut;
	}
	void setiscut(bool cut) {
		iscut = cut;
	}
	int getNodeCount() {
		return LUNode->getCount();
	}
	int getCellCount() {
		return cellCount;
	}
	int getFaceCount() {
		return LFace->getCount();
	}
	int getID() {//진delete
		if (id>0)return id;
		return NULL;
	}
	Node* getLUNode() {
		return LUNode;
	}
	Node* getLDNode() {
		return LDNode;
	}
	Node* getRDNode() {
		return RDNode;
	}
	Node* getRUNode() {
		return RUNode;
	}
	bool isParent() {
		return (LUCell != NULL);
	}
	double getSize() {
		return this->cellSize;
	}
	int getDepth() {
		return depth;
	}
	void setDepth(int newDepth) {
		depth = newDepth;
	}
	void setID(int newID) {
		this->id = newID;
	}
	void setLUNode(Node *node) {
		LUNode = node;
	}
	void setLDNode(Node *node) {
		LDNode = node;
	}
	void setRDNode(Node *node) {
		RDNode = node;
	}
	void setRUNode(Node *node) {
		RUNode = node;
	}
	void setSize(double size) {
		cellSize = size;
	}
	void dotPoint(double x, double y, int layer, int depth) { // 재 : LUCell, LDCell, RUCell, RDCell 모두 CellShape로 변환, Cell의 멤버함수에 접근 필요시 ((Cell*) __) 조치 2017-01-12
		const int OUTFLOW = 0, INLET = 1, TOP = 2, BOTTOM = 3, INTERRIOR = 4;
		/*
		안쪼개는 경우는 없는지 궁금
		*/
		if (LUCell == NULL) {    // 아직 이 Cell이 쪼개지지 않았으면, 쪼개라!
								 //( 쪼개졌으면 LUCell, LDCell, RDCell, RUCell이 NULL이 아닐거니까 그중 하나만 체크한거임 )
								 // 내부 Cell(Default)를 먼저 생성해둔다.
			LUCell = new Cell();    ((Cell*)LUCell)->setDepth(depth + 1);
			LDCell = new Cell();    ((Cell*)LDCell)->setDepth(depth + 1);
			RDCell = new Cell();    ((Cell*)RDCell)->setDepth(depth + 1);
			RUCell = new Cell();    ((Cell*)RUCell)->setDepth(depth + 1);
			cellVector.push_back(((Cell*)LUCell));    cellVector.push_back(((Cell*)LDCell));
			cellVector.push_back(((Cell*)RDCell));    cellVector.push_back(((Cell*)RUCell));

			// 새로 만들어지는 Node를 먼저 생성한다.
			Node *LeftCenter, *DownCenter, *RightCenter, *UpCenter, *cellCenter;
			if (!LFace->isParent()) { // parent가 아니면 쪼개져있지않으면
				LeftCenter = new Node(((LUNode->x + LDNode->x) / 2.0), ((LUNode->y + LDNode->y) / 2.0));
				nodeVector.push_back(LeftCenter);
			}
			else {
				LeftCenter = LFace->centerNode;
			}
			if (!DFace->isParent()) {
				DownCenter = new Node((this->LDNode->x + this->RDNode->x) / 2.0, (this->LDNode->y + this->RDNode->y) / 2.0);
				nodeVector.push_back(DownCenter);
			}
			else {
				DownCenter = DFace->centerNode;
			}
			if (!RFace->isParent()) {
				RightCenter = new Node((this->RDNode->x + this->RUNode->x) / 2.0, (this->RDNode->y + this->RUNode->y) / 2.0);
				nodeVector.push_back(RightCenter);
			}
			else {
				RightCenter = RFace->centerNode;
			}
			if (!UFace->isParent()) {
				UpCenter = new Node((this->RUNode->x + this->LUNode->x) / 2.0, (this->RUNode->y + this->LUNode->y) / 2.0);
				nodeVector.push_back(UpCenter);
			}
			else {
				UpCenter = UFace->centerNode;
			}
			cellCenter = new Node((this->LDNode->x + this->RUNode->x) / 2.0, (this->LDNode->y + this->RUNode->y) / 2.0);
			nodeVector.push_back(cellCenter);

			// 내부 Cell의 Node를 세팅한다.
			setChildNodes(LeftCenter, DownCenter, RightCenter, UpCenter, cellCenter, 0);

			// 내부 Cell Size 값 전달.
			double ChildSize = cellSize / 2.0;
			((Cell*)LUCell)->setSize(ChildSize);        ((Cell*)LDCell)->setSize(ChildSize);
			((Cell*)RDCell)->setSize(ChildSize);        ((Cell*)RUCell)->setSize(ChildSize);

			// 내부 Face 12개를 만들겁니다.
			Face *LU = 0, *LD = 0, *DL = 0, *DR = 0, *RD = 0, *RU = 0;
			Face *UR = 0, *UL = 0, *IL = 0, *ID = 0, *IR = 0, *IU = 0;

			// Cell 만들 때 새로 만들어지는 Face중 외부 8개 먼저 셋팅
			// LD LU
			if (!LFace->isParent()) {
				LD = new Face(LDNode, LeftCenter);    faceVector.push_back(LD);
				LU = new Face(LeftCenter, LUNode);    faceVector.push_back(LU);
				LD->setAttribute(LFace->attribute);
				LU->setAttribute(LFace->attribute);
				LFace->setChilds(LD, LU);
				LFace->setCenterNode(LeftCenter);
			}
			else {
				LD = LFace->Child1;
				LU = LFace->Child2;
			}
			// DL DR
			if (!DFace->isParent()) {
				DL = new Face(LDNode, DownCenter);    faceVector.push_back(DL);
				DR = new Face(DownCenter, RDNode);    faceVector.push_back(DR);
				DL->setAttribute(DFace->attribute);
				DR->setAttribute(DFace->attribute);
				DFace->setChilds(DL, DR);
				DFace->setCenterNode(DownCenter);
			}
			else {
				DL = DFace->Child1;
				DR = DFace->Child2;
			}
			// RD RU
			if (!RFace->isParent()) {
				RD = new Face(RDNode, RightCenter);    faceVector.push_back(RD);
				RU = new Face(RightCenter, RUNode);    faceVector.push_back(RU);
				RD->setAttribute(RFace->attribute);
				RU->setAttribute(RFace->attribute);
				RFace->setChilds(RD, RU);
				RFace->setCenterNode(RightCenter);
			}
			else {
				RD = RFace->Child1;
				RU = RFace->Child2;
			}
			// UL UR
			if (!UFace->isParent()) {
				UL = new Face(LUNode, UpCenter);    faceVector.push_back(UL);
				UR = new Face(UpCenter, RUNode);    faceVector.push_back(UR);
				UL->setAttribute(UFace->attribute);
				UR->setAttribute(UFace->attribute);
				UFace->setChilds(UL, UR);
				UFace->setCenterNode(UpCenter);
			}
			else {
				UL = UFace->Child1;
				UR = UFace->Child2;
			}

			IL = new Face(LeftCenter, cellCenter);    IL->setAttribute(INTERRIOR);    IL->setDepth(UL->depth);    faceVector.push_back(IL);
			ID = new Face(DownCenter, cellCenter);    ID->setAttribute(INTERRIOR);    ID->setDepth(UL->depth);    faceVector.push_back(ID);
			IR = new Face(cellCenter, RightCenter);    IR->setAttribute(INTERRIOR);    IR->setDepth(UL->depth);    faceVector.push_back(IR);
			IU = new Face(cellCenter, UpCenter);        IU->setAttribute(INTERRIOR);    IU->setDepth(UL->depth);    faceVector.push_back(IU);

			// 각 내부 cell이 face를 가리키도록
			((Cell*)LUCell)->LFace = LU;    ((Cell*)LUCell)->DFace = IL;    ((Cell*)LUCell)->RFace = IU;    ((Cell*)LUCell)->UFace = UL;
			((Cell*)LDCell)->LFace = LD;    ((Cell*)LDCell)->DFace = DL;    ((Cell*)LDCell)->RFace = ID;    ((Cell*)LDCell)->UFace = IL;
			((Cell*)RDCell)->LFace = ID;    ((Cell*)RDCell)->DFace = DR;    ((Cell*)RDCell)->RFace = RD;    ((Cell*)RDCell)->UFace = IR;
			((Cell*)RUCell)->LFace = IU;    ((Cell*)RUCell)->DFace = IR;    ((Cell*)RUCell)->RFace = RU;    ((Cell*)RUCell)->UFace = UR;

			// 각 Face의 startCell / endCell 셋팅 // 재 : Face의 sCell과 eCell도 CellShape로 변경하였기 때문에 밑에 코드도 약간 수정했습니다.  2017-01-12

			// LU, LD

			LU->setEndCell(((Cell*)LUCell));    LD->setEndCell(((Cell*)LDCell));
			if (LFace->attribute != INLET) {
				if (((Cell*)(LFace->sCell))->LUCell == NULL) {
					LU->setStartCell(LFace->sCell);
					LD->setStartCell(LFace->sCell);
				}
				else {
					LU->setStartCell(((Cell*)(LFace->sCell))->RUCell);
					LD->setStartCell(((Cell*)(LFace->sCell))->RDCell);
				}
			}
			// DL, DR
			DL->setStartCell(((Cell*)LDCell));    DR->setStartCell(((Cell*)RDCell));
			if (DFace->attribute != BOTTOM) {
				if (((Cell*)(DFace->eCell))->LUCell == NULL) {
					DL->setEndCell(DFace->eCell);
					DR->setEndCell(DFace->eCell);
				}
				else {
					DL->setEndCell(((Cell*)(DFace->eCell))->LUCell);
					DR->setEndCell(((Cell*)(DFace->eCell))->RUCell);
				}
			}
			// RD, RU
			RD->setStartCell(((Cell*)RDCell));    RU->setStartCell(((Cell*)RUCell));
			if (RFace->attribute != OUTFLOW) {
				if (((Cell*)(RFace->eCell))->LUCell == NULL) {
					RD->setEndCell(RFace->eCell);
					RU->setEndCell(RFace->eCell);
				}
				else {
					RD->setEndCell(((Cell*)(RFace->eCell))->LDCell);
					RU->setEndCell(((Cell*)(RFace->eCell))->LUCell);
				}
			}
			// UR, UL
			UR->setEndCell(((Cell*)RUCell));    UL->setEndCell(((Cell*)LUCell));
			if (UFace->attribute != TOP) {
				if (((Cell*)(UFace->sCell))->LUCell == NULL) {
					UR->setStartCell(UFace->sCell);
					UL->setStartCell(UFace->sCell);
				}
				else {
					UR->setStartCell(((Cell*)(UFace->sCell))->RDCell);
					UL->setStartCell(((Cell*)(UFace->sCell))->LDCell);
				}
			}
			// INNER Faces
			IL->setStartCell(((Cell*)LUCell));    IL->setEndCell(((Cell*)LDCell));
			ID->setStartCell(((Cell*)LDCell));    ID->setEndCell(((Cell*)RDCell));
			IR->setStartCell(((Cell*)RUCell));    IR->setEndCell(((Cell*)RDCell));
			IU->setStartCell(((Cell*)LUCell));    IU->setEndCell(((Cell*)RUCell));
		}

		// layer spread // 재 : SpreadCell 타입 CellShape로 변경하고 그에따라 밑에 코드 수정했습니다.  2017-01-12
		if (layer > 0) {
			// [0] [1] [2] [3] [4] [5] [6] [7]
			// LU,  L, LD,  D, RD,  R, RU,  U
			CellShape *SpreadCell[8] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL };
			bool isLeft = (LFace->attribute == INLET);
			bool isBottom = (DFace->attribute == BOTTOM);
			bool isRight = (RFace->attribute == OUTFLOW);
			bool isTop = (UFace->attribute == TOP);

			if (!isLeft) {
				SpreadCell[1] = LFace->sCell;
				if (!isTop)    SpreadCell[0] = ((Cell*)SpreadCell[1])->UFace->sCell;
				if (!isBottom)    SpreadCell[2] = ((Cell*)SpreadCell[1])->DFace->eCell;
			}
			if (!isBottom) {
				SpreadCell[3] = DFace->eCell;
				if (!isRight)    SpreadCell[4] = ((Cell*)SpreadCell[3])->RFace->eCell;
			}
			if (!isRight) {
				SpreadCell[5] = RFace->eCell;
				if (!isTop)    SpreadCell[6] = ((Cell*)SpreadCell[5])->UFace->sCell;
			}
			if (!isTop) SpreadCell[7] = UFace->sCell;
			// spread by layer
			for (int i = 0; i < 8; i++) {
				if (SpreadCell[i] != NULL) {
					((Cell*)SpreadCell[i])->dotPoint(x, y, layer - 1, 0);
				}
			}
		}

		// depth 내려가면서 더 쪼개는 부분
		if (depth > 0) {  // 재 : depth가 0일 때 쪼개는 걸로 알고 있었는데 그렇게 되면 mininode의 간격과 다라서 depth가 0일 때 쪼개지 않는 것으로 했습니다.
			Node *CellCenterNode = ((Cell*)LUCell)->RDNode;
			double centerX = CellCenterNode->x;
			double centerY = CellCenterNode->y;
			if (x > centerX) {
				if (y > centerY) {
					if (depth > 1)
						((Cell*)RUCell)->dotPoint(x, y, layer, depth - 1);
					else
						((Cell*)RUCell)->setupMininodePlace(x, y);
				}
				else {
					if (depth > 1)
						((Cell*)RDCell)->dotPoint(x, y, layer, depth - 1);
					else
						((Cell*)RDCell)->setupMininodePlace(x, y);
				}
			}
			else {
				if (y > centerY) {
					if (depth > 1)
						((Cell*)LUCell)->dotPoint(x, y, layer, depth - 1);
					else
						((Cell*)LUCell)->setupMininodePlace(x, y);
				}
				else {
					if (depth > 1)
						((Cell*)LDCell)->dotPoint(x, y, layer, depth - 1);
					else
						((Cell*)LDCell)->setupMininodePlace(x, y);
				}
			}
		}
	}
	void printCell() {
		// 그냥 개발자가 확인하려고 만들어둔 부분
		cout << this->id << " cell information --\n";
		cout << "  node  \n";
		cout << "LU = " << this->LUNode->id;
		cout << " LD = " << this->LDNode->id;
		cout << " RD = " << this->RDNode->id;
		cout << " RU = " << this->RUNode->id << endl;

		cout << "  face  \n";
		cout << "L = " << this->LFace->id;
		cout << " D = " << this->DFace->id;
		cout << " R = " << this->RFace->id;
		cout << " U = " << this->UFace->id << endl;
		cout << endl;

		// deldeldel
		if (LFace->Child1 != NULL) {
			if (LFace->Child1->Child1 != NULL) {
				cout << "!!!!!!!!!!!!!!!!!!!!!!!! ";
				cout << LFace->id << " == " << LFace->Child1->id << " == (" << LFace->Child1->Child1->id << " " << LFace->Child1->Child2->id << ")\n";
			}
			if (LFace->Child2->Child1 != NULL) {
				cout << "!!!!!!!!!!!!!!!!!!!!!!!! ";
				cout << LFace->id << " == " << LFace->Child2->id << " == (" << LFace->Child2->Child1->id << " " << LFace->Child2->Child2->id << ")\n";
			}
		}
		if (DFace->Child1 != NULL) {
			if (DFace->Child1->Child1 != NULL) {
				cout << "!!!!!!!!!!!!!!!!!!!!!!!! ";
				cout << DFace->id << " == " << DFace->Child1->id << " == (" << DFace->Child1->Child1->id << " " << DFace->Child1->Child2->id << ")\n";
			}
			if (DFace->Child2->Child1 != NULL) {
				cout << "!!!!!!!!!!!!!!!!!!!!!!!! ";
				cout << DFace->id << " == " << DFace->Child2->id << " == (" << DFace->Child2->Child1->id << " " << DFace->Child2->Child2->id << ")\n";
			}
		}
		if (RFace->Child1 != NULL) {
			if (RFace->Child1->Child1 != NULL) {
				cout << "!!!!!!!!!!!!!!!!!!!!!!!! ";
				cout << RFace->id << " == " << RFace->Child1->id << " == (" << RFace->Child1->Child1->id << " " << RFace->Child1->Child2->id << ")\n";
			}
			if (RFace->Child2->Child1 != NULL) {
				cout << "!!!!!!!!!!!!!!!!!!!!!!!! ";
				cout << RFace->id << " == " << RFace->Child2->id << " == (" << RFace->Child2->Child1->id << " " << RFace->Child2->Child2->id << ")\n";
			}
		}
		if (UFace->Child1 != NULL) {
			if (UFace->Child1->Child1 != NULL) {
				cout << "!!!!!!!!!!!!!!!!!!!!!!!! ";
				cout << UFace->id << " == " << UFace->Child1->id << " == (" << UFace->Child1->Child1->id << " " << UFace->Child1->Child2->id << ")\n";
			}
			if (UFace->Child2->Child1 != NULL) {
				cout << "!!!!!!!!!!!!!!!!!!!!!!!! ";
				cout << UFace->id << " == " << UFace->Child2->id << " == (" << UFace->Child2->Child1->id << " " << UFace->Child2->Child2->id << ")\n";
			}
		}
		system("pause");
	}
	void confirmAll() { // 재 : Face의 eCell과 sCell을 CellShape로 변경해서 이 부분도 수정이 필요해 수정했습니다.  2017-01-12
						// 그냥 개발자가 확인하려고 만들어 둔 부분
		if (LUCell != NULL) {
			if (((Cell*)LUCell)->LUCell != NULL) {
				cout << "Start Left : " << ((Cell*)LUCell)->LFace->id << "    ";
				cout << ((Cell*)LUCell)->LFace->Child2->id << " " << ((Cell*)LUCell)->LFace->Child1->id << " __ " <<
					((Cell*)LUCell)->DFace->Child1->id << " " << ((Cell*)LUCell)->DFace->Child2->id << " __ " <<
					((Cell*)LUCell)->RFace->Child1->id << " " << ((Cell*)LUCell)->RFace->Child2->id << " __ " <<
					((Cell*)LUCell)->UFace->Child2->id << " " << ((Cell*)LUCell)->UFace->Child1->id << " __    " <<
					((Cell*)((Cell*)LUCell)->LUCell)->DFace->id << " " << ((Cell*)((Cell*)LUCell)->LDCell)->RFace->id << " " <<
					((Cell*)((Cell*)LUCell)->RDCell)->UFace->id << " " << ((Cell*)((Cell*)LUCell)->RUCell)->LFace->id << "\n\n";
			}
			if (((Cell*)LDCell)->LUCell != NULL) {
				cout << "Start Left : " << ((Cell*)LDCell)->LFace->id << "    ";
				cout << ((Cell*)LDCell)->LFace->Child2->id << " " << ((Cell*)LDCell)->LFace->Child1->id << " __ " <<
					((Cell*)LDCell)->DFace->Child1->id << " " << ((Cell*)LDCell)->DFace->Child2->id << " __ " <<
					((Cell*)LDCell)->RFace->Child1->id << " " << ((Cell*)LDCell)->RFace->Child2->id << " __ " <<
					((Cell*)LDCell)->UFace->Child2->id << " " << ((Cell*)LDCell)->UFace->Child1->id << " __    " <<
					((Cell*)((Cell*)LDCell)->LUCell)->DFace->id << " " << ((Cell*)((Cell*)LDCell)->LDCell)->RFace->id << " " <<
					((Cell*)((Cell*)LDCell)->RDCell)->UFace->id << " " << ((Cell*)((Cell*)LDCell)->RUCell)->LFace->id << "\n\n";
			}
			if (((Cell*)RDCell)->RDCell != NULL) {
				cout << "Start Left : " << ((Cell*)RDCell)->LFace->id << "    ";
				cout << ((Cell*)RDCell)->LFace->Child2->id << " " << ((Cell*)RDCell)->LFace->Child1->id << " __ " <<
					((Cell*)RDCell)->DFace->Child1->id << " " << ((Cell*)RDCell)->DFace->Child2->id << " __ " <<
					((Cell*)RDCell)->RFace->Child1->id << " " << ((Cell*)RDCell)->RFace->Child2->id << " __ " <<
					((Cell*)RDCell)->UFace->Child2->id << " " << ((Cell*)RDCell)->UFace->Child1->id << " __    " <<
					((Cell*)((Cell*)RDCell)->LUCell)->DFace->id << " " << ((Cell*)((Cell*)RDCell)->LDCell)->RFace->id << " "
					<< ((Cell*)((Cell*)RDCell)->RDCell)->UFace->id << " " << ((Cell*)((Cell*)RDCell)->RUCell)->LFace->id << "\n\n";
			}
			if (((Cell*)RUCell)->RUCell != NULL) {
				cout << "Start Left : " << ((Cell*)RUCell)->LFace->id << "    ";
				cout << ((Cell*)RUCell)->LFace->Child2->id << " " << ((Cell*)RUCell)->LFace->Child1->id << " __ " <<
					((Cell*)RUCell)->DFace->Child1->id << " " << ((Cell*)RUCell)->DFace->Child2->id << " __ " <<
					((Cell*)RUCell)->RFace->Child1->id << " " << ((Cell*)RUCell)->RFace->Child2->id << " __ " <<
					((Cell*)RUCell)->UFace->Child2->id << " " << ((Cell*)RUCell)->UFace->Child1->id << " __    " <<
					((Cell*)((Cell*)RUCell)->LUCell)->DFace->id << " " << ((Cell*)((Cell*)RUCell)->LDCell)->RFace->id << " " <<
					((Cell*)((Cell*)RUCell)->RDCell)->UFace->id << " " << ((Cell*)((Cell*)RUCell)->RUCell)->LFace->id << "\n\n";
			}

		}
		else {
			cout << "NO CHILD\n";
		}
	}
	bool checkNode(Node* n) { // 재 : Node를 인자로 주면 Cell이 인자를 가지고 있는지 확인하는 함수 2017-01-12
		if (LUNode == n || LDNode == n || RUNode == n || RDNode == n)
			return true;
		return false;
	}
	bool checkFace(Face* f) { // 재 : Face를 인자로 주면 Cell이 인자를 가지고 있는지 확인하는 함수 2017-01-12
		if (LFace == f || UFace == f || RFace == f || DFace == f)
			return true;
		return false;
	}
	void splitCell(int layer, int depth) { // 재 : Cell split 구현, depth 추가 2017-01-19
		const int OUTFLOW = 0, INLET = 1, TOP = 2, BOTTOM = 3, INTERRIOR = 4;
		if (LUCell == NULL) {    // 아직 이 Cell이 쪼개지지 않았으면, 쪼개라!
								 //( 쪼개졌으면 LUCell, LDCell, RDCell, RUCell이 NULL이 아닐거니까 그중 하나만 체크한거임 )
								 // 내부 Cell(Default)를 먼저 생성해둔다.
			LUCell = new Cell();    ((Cell*)LUCell)->setDepth(depth + 1);
			LDCell = new Cell();    ((Cell*)LDCell)->setDepth(depth + 1);
			RDCell = new Cell();    ((Cell*)RDCell)->setDepth(depth + 1);
			RUCell = new Cell();    ((Cell*)RUCell)->setDepth(depth + 1);
			cellVector.push_back(((Cell*)LUCell));    cellVector.push_back(((Cell*)LDCell));
			cellVector.push_back(((Cell*)RDCell));    cellVector.push_back(((Cell*)RUCell));

			// 새로 만들어지는 Node를 먼저 생성한다.
			Node *LeftCenter, *DownCenter, *RightCenter, *UpCenter, *cellCenter;
			if (!LFace->isParent()) { // parent가 아니면 쪼개져있지않으면
				LeftCenter = new Node(((LUNode->x + LDNode->x) / 2.0), ((LUNode->y + LDNode->y) / 2.0));
				nodeVector.push_back(LeftCenter);
			}
			else {
				LeftCenter = LFace->centerNode;
			}
			if (!DFace->isParent()) {
				DownCenter = new Node((this->LDNode->x + this->RDNode->x) / 2.0, (this->LDNode->y + this->RDNode->y) / 2.0);
				nodeVector.push_back(DownCenter);
			}
			else {
				DownCenter = DFace->centerNode;
			}
			if (!RFace->isParent()) {
				RightCenter = new Node((this->RDNode->x + this->RUNode->x) / 2.0, (this->RDNode->y + this->RUNode->y) / 2.0);
				nodeVector.push_back(RightCenter);
			}
			else {
				RightCenter = RFace->centerNode;
			}
			if (!UFace->isParent()) {
				UpCenter = new Node((this->RUNode->x + this->LUNode->x) / 2.0, (this->RUNode->y + this->LUNode->y) / 2.0);
				nodeVector.push_back(UpCenter);
			}
			else {
				UpCenter = UFace->centerNode;
			}
			cellCenter = new Node((this->LDNode->x + this->RUNode->x) / 2.0, (this->LDNode->y + this->RUNode->y) / 2.0);
			nodeVector.push_back(cellCenter);

			// 내부 Cell의 Node를 세팅한다.
			setChildNodes(LeftCenter, DownCenter, RightCenter, UpCenter, cellCenter, 0);

			// 내부 Cell Size 값 전달.
			double ChildSize = cellSize / 2.0;
			((Cell*)LUCell)->setSize(ChildSize);        ((Cell*)LDCell)->setSize(ChildSize);
			((Cell*)RDCell)->setSize(ChildSize);        ((Cell*)RUCell)->setSize(ChildSize);

			// 내부 Face 12개를 만들겁니다.
			Face *LU = 0, *LD = 0, *DL = 0, *DR = 0, *RD = 0, *RU = 0;
			Face *UR = 0, *UL = 0, *IL = 0, *ID = 0, *IR = 0, *IU = 0;

			// Cell 만들 때 새로 만들어지는 Face중 외부 8개 먼저 셋팅
			// LD LU
			if (!LFace->isParent()) {
				LD = new Face(LDNode, LeftCenter);    faceVector.push_back(LD);
				LU = new Face(LeftCenter, LUNode);    faceVector.push_back(LU);
				LD->setAttribute(LFace->attribute);
				LU->setAttribute(LFace->attribute);
				LFace->setChilds(LD, LU);
				LFace->setCenterNode(LeftCenter);
			}
			else {
				LD = LFace->Child1;
				LU = LFace->Child2;
			}
			// DL DR
			if (!DFace->isParent()) {
				DL = new Face(LDNode, DownCenter);    faceVector.push_back(DL);
				DR = new Face(DownCenter, RDNode);    faceVector.push_back(DR);
				DL->setAttribute(DFace->attribute);
				DR->setAttribute(DFace->attribute);
				DFace->setChilds(DL, DR);
				DFace->setCenterNode(DownCenter);
			}
			else {
				DL = DFace->Child1;
				DR = DFace->Child2;
			}
			// RD RU
			if (!RFace->isParent()) {
				RD = new Face(RDNode, RightCenter);    faceVector.push_back(RD);
				RU = new Face(RightCenter, RUNode);    faceVector.push_back(RU);
				RD->setAttribute(RFace->attribute);
				RU->setAttribute(RFace->attribute);
				RFace->setChilds(RD, RU);
				RFace->setCenterNode(RightCenter);
			}
			else {
				RD = RFace->Child1;
				RU = RFace->Child2;
			}
			// UL UR
			if (!UFace->isParent()) {
				UL = new Face(LUNode, UpCenter);    faceVector.push_back(UL);
				UR = new Face(UpCenter, RUNode);    faceVector.push_back(UR);
				UL->setAttribute(UFace->attribute);
				UR->setAttribute(UFace->attribute);
				UFace->setChilds(UL, UR);
				UFace->setCenterNode(UpCenter);
			}
			else {
				UL = UFace->Child1;
				UR = UFace->Child2;
			}

			IL = new Face(LeftCenter, cellCenter);    IL->setAttribute(INTERRIOR);    IL->setDepth(UL->depth);    faceVector.push_back(IL);
			ID = new Face(DownCenter, cellCenter);    ID->setAttribute(INTERRIOR);    ID->setDepth(UL->depth);    faceVector.push_back(ID);
			IR = new Face(cellCenter, RightCenter);    IR->setAttribute(INTERRIOR);    IR->setDepth(UL->depth);    faceVector.push_back(IR);
			IU = new Face(cellCenter, UpCenter);        IU->setAttribute(INTERRIOR);    IU->setDepth(UL->depth);    faceVector.push_back(IU);

			// 각 내부 cell이 face를 가리키도록
			((Cell*)LUCell)->LFace = LU;    ((Cell*)LUCell)->DFace = IL;    ((Cell*)LUCell)->RFace = IU;    ((Cell*)LUCell)->UFace = UL;
			((Cell*)LDCell)->LFace = LD;    ((Cell*)LDCell)->DFace = DL;    ((Cell*)LDCell)->RFace = ID;    ((Cell*)LDCell)->UFace = IL;
			((Cell*)RDCell)->LFace = ID;    ((Cell*)RDCell)->DFace = DR;    ((Cell*)RDCell)->RFace = RD;    ((Cell*)RDCell)->UFace = IR;
			((Cell*)RUCell)->LFace = IU;    ((Cell*)RUCell)->DFace = IR;    ((Cell*)RUCell)->RFace = RU;    ((Cell*)RUCell)->UFace = UR;

			// 각 Face의 startCell / endCell 셋팅 // 재 : Face의 sCell과 eCell도 CellShape로 변경하였기 때문에 밑에 코드도 약간 수정했습니다.  2017-01-12

			// LU, LD

			LU->setEndCell(((Cell*)LUCell));    LD->setEndCell(((Cell*)LDCell));
			if (LFace->attribute != INLET) {
				if (((Cell*)(LFace->sCell))->LUCell == NULL) {
					LU->setStartCell(LFace->sCell);
					LD->setStartCell(LFace->sCell);
				}
				else {
					LU->setStartCell(((Cell*)(LFace->sCell))->RUCell);
					LD->setStartCell(((Cell*)(LFace->sCell))->RDCell);
				}
			}
			// DL, DR
			DL->setStartCell(((Cell*)LDCell));    DR->setStartCell(((Cell*)RDCell));
			if (DFace->attribute != BOTTOM) {
				if (((Cell*)(DFace->eCell))->LUCell == NULL) {
					DL->setEndCell(DFace->eCell);
					DR->setEndCell(DFace->eCell);
				}
				else {
					DL->setEndCell(((Cell*)(DFace->eCell))->LUCell);
					DR->setEndCell(((Cell*)(DFace->eCell))->RUCell);
				}
			}
			// RD, RU
			RD->setStartCell(((Cell*)RDCell));    RU->setStartCell(((Cell*)RUCell));
			if (RFace->attribute != OUTFLOW) {
				if (((Cell*)(RFace->eCell))->LUCell == NULL) {
					RD->setEndCell(RFace->eCell);
					RU->setEndCell(RFace->eCell);
				}
				else {
					RD->setEndCell(((Cell*)(RFace->eCell))->LDCell);
					RU->setEndCell(((Cell*)(RFace->eCell))->LUCell);
				}
			}
			// UR, UL
			UR->setEndCell(((Cell*)RUCell));    UL->setEndCell(((Cell*)LUCell));
			if (UFace->attribute != TOP) {
				if (((Cell*)(UFace->sCell))->LUCell == NULL) {
					UR->setStartCell(UFace->sCell);
					UL->setStartCell(UFace->sCell);
				}
				else {
					UR->setStartCell(((Cell*)(UFace->sCell))->RDCell);
					UL->setStartCell(((Cell*)(UFace->sCell))->LDCell);
				}
			}
			// INNER Faces
			IL->setStartCell(((Cell*)LUCell));    IL->setEndCell(((Cell*)LDCell));
			ID->setStartCell(((Cell*)LDCell));    ID->setEndCell(((Cell*)RDCell));
			IR->setStartCell(((Cell*)RUCell));    IR->setEndCell(((Cell*)RDCell));
			IU->setStartCell(((Cell*)LUCell));    IU->setEndCell(((Cell*)RUCell));
		}

		// layer spread // 재 : Cell split 에 layer기능 추가.  2017-01-22
		if (layer > 0) {
			// [0] [1] [2] [3] [4] [5] [6] [7]
			// LU,  L, LD,  D, RD,  R, RU,  U
			CellShape *SpreadCell[8] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL };
			bool isLeft = (LFace->attribute == INLET);
			bool isBottom = (DFace->attribute == BOTTOM);
			bool isRight = (RFace->attribute == OUTFLOW);
			bool isTop = (UFace->attribute == TOP);

			if (!isLeft) {
				SpreadCell[1] = LFace->sCell;
				if (!isTop)    SpreadCell[0] = ((Cell*)SpreadCell[1])->UFace->sCell;
				if (!isBottom)    SpreadCell[2] = ((Cell*)SpreadCell[1])->DFace->eCell;
			}
			if (!isBottom) {
				SpreadCell[3] = DFace->eCell;
				if (!isRight)    SpreadCell[4] = ((Cell*)SpreadCell[3])->RFace->eCell;
			}
			if (!isRight) {
				SpreadCell[5] = RFace->eCell;
				if (!isTop)    SpreadCell[6] = ((Cell*)SpreadCell[5])->UFace->sCell;
			}
			if (!isTop) SpreadCell[7] = UFace->sCell;
			// spread by layer
			for (int i = 0; i < 8; i++) {
				if (SpreadCell[i] != NULL) {
					((Cell*)SpreadCell[i])->splitCell(layer - 1, 0);
				}
			}
		}

		// depth 내려가면서 더 쪼개는 부분
		if (depth > 1) {
			((Cell*)RUCell)->splitCell(layer, depth - 1);
			((Cell*)RDCell)->splitCell(layer, depth - 1);
			((Cell*)LUCell)->splitCell(layer, depth - 1);
			((Cell*)LDCell)->splitCell(layer, depth - 1);
		}
	}

};
class CellRectangle : public CellShape { // 재 : child를 가지지 않는다.
public:
	Node * LUNode, *LDNode, *RDNode, *RUNode;
	CellRectangle() {
		LUNode = LDNode = RDNode = RUNode = NULL;
		iscut = false;
	}
};

class CellTriangle : public CellShape { // 재 : child를 가지지 않는다.
public: //0208
	Node * HONode, *VONode, *WONode; // opposite angle, 각 페이스의 대각에 위치한 점
	CellTriangle() {
		HONode = VONode = WONode = NULL;
		id = ++triCount;
		iscut = false;
	}
	CellTriangle(Node *HO, Node *VO, Node *WO) {
		HONode = HO;    VONode = VO;    WONode = WO;
		id = ++triCount;
		iscut = false;
	}
	~CellTriangle() {
		triCount--;
	}
	int getTriCount() {
		return triCount;
	}
};
class Rectangle {
public:
	double LDx, LDy, RUx, RUy;
	Cell **initCell;
	double initCellSize;
	int widthCount, heightCount;

public:
	Rectangle(double LDX, double LDY, double Cellsize, int WidthCount, int HeightCount) {//진_initcellsize설정
		LDx = LDX;    LDy = LDY;
		RUx = LDX + WidthCount * Cellsize;    RUy = LDY + HeightCount * Cellsize;//진_initcellsize설정

		initCellSize = Cellsize;//진_initcellsize설정

		widthCount = WidthCount;//진_initcellsize설정
		heightCount = HeightCount;//진_initcellsize설정

		Node **initNode = new Node*[widthCount + 1];
		for (int w = 0; w <= widthCount; w++) {
			double plusX = LDX + (w * initCellSize);
			initNode[w] = new Node[heightCount + 1];
			for (int h = 0; h <= heightCount; h++) {
				double plusY = LDY + (h * initCellSize);
				initNode[w][h].setPoint(plusX, plusY);
				nodeVector.push_back(&initNode[w][h]);
			}
		}

		const int OUTFLOW = 0, INLET = 1, TOP = 2, BOTTOM = 3, INTERRIOR = 4;
		initCell = new Cell*[widthCount];
		for (int i = 0; i<widthCount; i++) {
			initCell[i] = new Cell[heightCount];
			for (int j = 0; j<heightCount; j++) {
				initCell[i][j].setSize(initCellSize);
				cellVector.push_back(&initCell[i][j]);

				initCell[i][j].setLUNode(&initNode[i][j + 1]);
				initCell[i][j].setLDNode(&initNode[i][j]);
				initCell[i][j].setRDNode(&initNode[i + 1][j]);
				initCell[i][j].setRUNode(&initNode[i + 1][j + 1]);

				// Left Face
				if (i == 0) { // Left : inlet
					initCell[i][j].LFace = new Face(initCell[i][j].LDNode, initCell[i][j].LUNode);
					initCell[i][j].LFace->setAttribute(INLET);
					faceVector.push_back(initCell[i][j].LFace);
				}
				else {
					initCell[i][j].LFace = initCell[i - 1][j].RFace;
				}

				// Bottom Face
				if (j == 0) {
					initCell[i][j].DFace = new Face(initCell[i][j].LDNode, initCell[i][j].RDNode);
					initCell[i][j].DFace->setAttribute(BOTTOM);
					faceVector.push_back(initCell[i][j].DFace);
				}
				else {
					initCell[i][j].DFace = initCell[i][j - 1].UFace;
				}

				// Right Face
				initCell[i][j].RFace = new Face(initCell[i][j].RDNode, initCell[i][j].RUNode);
				if (i == widthCount - 1) {
					initCell[i][j].RFace->setAttribute(OUTFLOW);
				}
				faceVector.push_back(initCell[i][j].RFace);

				// Top Face
				initCell[i][j].UFace = new Face(initCell[i][j].LUNode, initCell[i][j].RUNode);
				if (j == heightCount - 1) {
					initCell[i][j].UFace->setAttribute(TOP);
				}
				faceVector.push_back(initCell[i][j].UFace);

				if (initCell[i][j].LFace->attribute != INLET)
					initCell[i][j].LFace->setStartCell(&initCell[i - 1][j]);
				initCell[i][j].LFace->setEndCell(&initCell[i][j]);

				initCell[i][j].DFace->setStartCell(&initCell[i][j]);
				if (initCell[i][j].DFace->attribute != BOTTOM)
					initCell[i][j].DFace->setEndCell(&initCell[i][j - 1]);

				initCell[i][j].RFace->setStartCell(&initCell[i][j]);
				if (initCell[i][j].RFace->attribute != OUTFLOW)
					initCell[i][j].RFace->setEndCell(&initCell[i + 1][j]);

				if (initCell[i][j].UFace->attribute != TOP)
					initCell[i][j].UFace->setStartCell(&initCell[i][j + 1]);
				initCell[i][j].UFace->setEndCell(&initCell[i][j]);
			}
		}
	}
	int getNodeCount() {
		return initCell[0][0].getLDNode()->getCount();
	}
	int getCellCount() {
		return initCell[0][0].getCellCount();
	}
	int getFaceCount() {
		return initCell[0][0].getFaceCount();
	}
	void printAll() {
		for (int w = 0; w<widthCount; w++) {
			for (int h = 0; h<heightCount; h++) {
				cout << "cell[" << w << "][" << h << "]\n";
				initCell[w][h].printCell();
			}
		}
	}
	void confirmAll() {
		for (int w = 0; w<widthCount; w++) {
			for (int h = 0; h<heightCount; h++) {
				cout << "cell[" << w << "][" << h << "]\n";
				initCell[w][h].confirmAll();
			}
		}
	}
	void dotPoint(double x, double y, int layer = 0, int depth = 0) { //진 : 경계선노드처리 수정
		int flag = 0;// flag 0: 기본 1: x경계에 걸쳐짐 2: y 경계에 걸쳐짐 3: 둘다 걸쳐짐
		int findX = 1, findY = 1;
		while (true) {
			if (((findX * initCellSize) + LDx) > x) {
				break;
			}
			else if (((findX * initCellSize) + LDx) == x) flag = 1;

			findX++;
		}
		while (true) {
			if (((findY * initCellSize) + LDy) > y) {
				break;
			}
			else if (((findY * initCellSize) + LDy) == y) {
				if (flag == 1) flag = 3;
				flag = 2;
			}
			findY++;
		}
		initCell[findX - 1][findY - 1].dotPoint(x, y, layer, depth);
		if ((flag == 1 || flag == 3) && findX > 1) initCell[findX - 2][findY - 1].dotPoint(x, y, layer, depth);
		else if ((flag == 2 || flag == 3) && findY > 1) initCell[findX - 1][findY - 2].dotPoint(x, y, layer, depth);


	}
	void splitCell(double, double, double, double, int, int, int); // 재 : Cell split 함수 입니다. 점 두 개와 layer, depth를 입력받습니다. (x1, y1, x2, y2, layer, depth) 2017-01-22
};

int Node::nodeCount = 0;
int Face::faceCount = 0;
int Face::maxDepth = 0;

int CellShape::cellCount = 0;
//int CellRectangle::rectCount = 0;
int CellShape::triCount = 0;
//int CellTriangle::triCount = 0;//0208

void DescribeMesh() { // 재 : Cell => CellShape 2017-01-12 //0208
	fprintf(fp, "(0 \"Grid:\")\n\n");

	fprintf(fp, "(0 \"Dimensions:\")\n");
	fprintf(fp, "(2 2)\n\n");

	fprintf(fp, "(12 (0 1 %x 0))\n", cellVector.size() + minimalCellVector_tri.size()); // Cell Count
	fprintf(fp, "(13 (0    1 %lx 0))\n", faceVector.size() + wallVector.size()); // Face Count
	fprintf(fp, "(10 (0 1 %x 0 2))\n\n", initRectangle->getNodeCount()); // Node Count

	int myID = 1;
	double printSize = cellVector[0]->getSize();
	int cellParentCount = 0;
	for (int i = 0; i< cellVector.size(); i++) {
		if (cellVector[i]->isParent()) {
			cellParentCount++;
		}
	}

	fprintf(fp, "(12 (%x 1 %lx 1 3))\n", myID++, cellVector.size() - cellParentCount); // Cells (if have child, it should be changed)
	if (cellParentCount != 0) {
		fprintf(fp, "(12 (%x %lx %x 20 3))\n", myID++, 1 + cellVector.size() - cellParentCount, cellVector.size());
	}
	if (minimalCellVector_tri.size() != 0) {
		fprintf(fp, "(12 (%x %lx %x 1 1))\n", myID++, cellVector.size() + 1, cellVector.size() + minimalCellVector_tri.size());
	}
	fprintf(fp, "\n");

	//triangle 정의

	int faceMaxDepth = faceVector[0]->getMaxDepth();
	int **FACount = new int*[faceMaxDepth + 1];
	for (int i = 0; i<faceMaxDepth + 1; i++) {
		const int ColumnCount = 10;
		FACount[i] = new int[ColumnCount];
		for (int j = 0; j<ColumnCount; j++) FACount[i][j] = 0;
	}

	// for get Count attributes
	const int OUTFLOW = 0, INLET = 1, TOP = 2, BOTTOM = 3, INTERRIOR = 4;
	int findDepth = faceVector[0]->getDepth();
	for (vector<Face*>::iterator it = faceVector.begin(); it != faceVector.end(); it++) {
		if ((*it)->depth != findDepth) {
			findDepth = (*it)->depth;
		}
		int att = (*it)->attribute;
		int plusAlpha = 0;
		if ((*it)->isParent()) plusAlpha = 5;
		if (att == INTERRIOR) {
			FACount[findDepth][0 + plusAlpha]++;
		}
		else if (att == BOTTOM) {
			FACount[findDepth][1 + plusAlpha]++;
		}
		else if (att == TOP) {
			FACount[findDepth][2 + plusAlpha]++;
		}
		else if (att == INLET) {
			FACount[findDepth][3 + plusAlpha]++;
		}
		else {
			FACount[findDepth][4 + plusAlpha]++;
		}
	}

	// start print faces
	queue< Face* > haveChild;
	int faceNumber = 1, lowCount = 0, di = faceVector[0]->depth;
	for (unsigned int k = 0; k<faceVector.size(); k++) {


		if (di != faceVector[k]->depth) {
			di = faceVector[k]->depth;
			lowCount = 0;

		}
		if (FACount[di][lowCount] == 0) {
			k--; lowCount++; continue;
		}
		int ParentChildNumber[2] = { faceNumber,FACount[di][lowCount] + faceNumber - 1 };
		fprintf(fp, "(13 (%x %x %x ", myID++, faceNumber, FACount[di][lowCount] + faceNumber - 1);
		if (faceVector[k]->isParent()) {
			fprintf(fp, "1f 2)(");
			faceVector[k]->myPrintID = (myID - 1);
		}
		else if (lowCount == 0) fprintf(fp, "2 2)(");//interior
		else if (lowCount <= 2) fprintf(fp, "7 2)(");//upp//lower
		else if (lowCount == 3) fprintf(fp, "a 2)(");//inlet
		else if (lowCount == 4) fprintf(fp, "24 2)(");//out

		faceNumber = FACount[di][lowCount] + faceNumber;
		for (int i = 0; i<FACount[di][lowCount]; i++) {
			if (faceVector[k]->isChild()) {
				faceVector[k]->myPrintID = (myID - 1);
			}
			if (faceVector[k]->isParent()) {
				faceVector[k]->myPrintID = (myID - 1);
				haveChild.push(faceVector[k]);
			}
			CellShape *s = faceVector[k]->sCell;
			CellShape *e = faceVector[k]->eCell;

			int sn = faceVector[k]->sNode->id;
			int en = faceVector[k]->eNode->id;
			int sfi = NULL;
			int efi = 0;

			if (s != NULL) sfi = s->getID();//0208
			if (e != NULL) efi = e->getID();//0208

											/* test
											vector<int>::iterator initID;
											for(initID = detinitcellVector.begin(); initID < detinitcellVector.end(); initID++) {
											cout << "initCellID :" <<(*initID) << endl;
											if (sfi == (*initID)) {
											sfi = 0;
											}
											if (efi == (*initID)) {
											efi = 0;
											}
											}*/

											/*
											if (efi > 204 || sfi > 204) {
											cout << "find" << endl;

											}
											*/
											/*
											if (faceVector[k]->eNode->getX() == -1.5 && faceVector[k]->eNode->getY() == 0)
											cout << "find2" << endl;
											else if (faceVector[k]->sNode->getX() == -1.5 && faceVector[k]->sNode->getY() == 0)
											cout << " find3" << endl;
											*/
			if (s == NULL || sfi == NULL) {//진delete
				fprintf(fp, "\n%x    %x    %x    %x", en, sn, efi, sfi);
			}
			else {
				fprintf(fp, "\n%x    %x    %x    %x", sn, en, sfi, efi);
			}
			k++;
		}
		fprintf(fp, "))\n\n");
		lowCount++;
		k--;
		if (!haveChild.empty()) {
			fprintf(fp, "(59 (%x %x %x %x)(", ParentChildNumber[0], ParentChildNumber[1], haveChild.front()->myPrintID, haveChild.front()->Child1->myPrintID);
			myID++;
			while (!haveChild.empty()) {
				fprintf(fp, "\n 2 %x %x", haveChild.front()->Child1->id, haveChild.front()->Child2->id);
				haveChild.pop();
			}
			fprintf(fp, "))\n\n");
		}
	} // end of faces

	  //print wall
	fprintf(fp, "(13 (%x %lx %lx 3 2)(", myID++, faceVector.size() + 1, faceVector.size() + wallVector.size());
	for (int i = 0; i < wallVector.size(); i++) {
		int sn = wallVector[i]->sNode->id;
		int en = wallVector[i]->eNode->id;
		int sfi = wallVector[i]->sCell->getID();
		fprintf(fp, "\n%x    %x    %x    0", sn, en, sfi);
	}
	fprintf(fp, "))\n\n");

	// print nodes
	int nodeCount = nodeVector.size();
	fprintf(fp, "(10 (%x 1 %x 1 2)\n(\n", myID++, nodeCount);
	for (int i = 0; i<nodeCount - 1; i++) {
		fprintf(fp, "%lf\t    %lf\n", nodeVector[i]->x, nodeVector[i]->y);
	}
	fprintf(fp, "%lf\t    %lf))\n", nodeVector[nodeCount - 1]->x, nodeVector[nodeCount - 1]->y);
	// end of nodes
}
int cnt = 0;
bool faceCompare(Face *f1, Face *f2) {
	//delete 된 face 들 처리
	if (f1->isParent()) {
		if (f1->Child1->getID() == NULL)f1->Child1 = f1->Child2;
		else if (f1->Child2->getID() == NULL) f1->Child2 = f1->Child1;
	}
	if (f2->isParent()) {
		if (f2->Child1->getID() == NULL) f2->Child1 = f2->Child2;
		else if (f2->Child2->getID() == NULL) f2->Child2 = f2->Child1;
	}
	int f1Depth = f1->depth;
	int f2Depth = f2->depth;
	if (f1Depth > f2Depth) {
		return true;
	}
	else if (f1Depth == f2Depth) {
		if ((f1->isParent() && f2->isParent())) {
			if (f1->attribute > f2->attribute)
				return true;
			else if (f1->attribute == f2->attribute) {
				return f1->Child1->myPrintID > f2->Child1->myPrintID;
			}
			else
				return false;
		}
		else if ((!f1->isParent() && !f2->isParent())) {
			return f1->attribute > f2->attribute;
		}
		else {
			return !f1->isParent();
		}

	}
	else {
		return false;
	}
}
bool cellCompare(Cell *c1, Cell *c2) {

	if ((c1->isParent() && c2->isParent()) || (!c1->isParent() && !c2->isParent())) {
		return false;
	}
	else {
		return !c1->isParent();
	}
}
bool nodeCompare(Node *n1, Node *n2) { // 재 : checkInner에서 사용
	if (n1->getID() < n2->getID())
		return true;
	return false;
}
bool nodeCompareXY(Node *n1, Node *n2) { // 재 : checkInner에서 사용
	if (n1->getX() == n2->getX()) {
		if (n1->getY() > n2->getY())
			return true;
	}
	else {
		if (n1->getX() < n2->getX()) return true;
	}
	return false;
}
int findn(double point, double size, int SorL) {//진
	if (SorL == 0) {//small
		if (point >= 0) {
			int n = ((point / size) - (int)(point / size) == 0) ? point / size : point / size + 1;
			return n;
		}
		int n = point / size;
		return n;
	}
	//large
	if (point < 0) {
		int n = ((point / size) - (int)(point / size) == 0) ? point / size : point / size - 1;
		return n;
	}
	int n = point / size;
	return n;
}

void findminnode(double x0, double y0, double x1, double y1, double size, int updown) {//진_01_22 수정                                                        // input2.txt로 실행시 mininode의 간격이 최소 cell사이즈보다 커서 findmininodeCell에서 cell을 찾지 못하는 문제가 있음.
	double gradient = (y1 - y0) / (x1 - x0);
	double sx, lx, sy, ly;
	sx = min(x0, x1);
	lx = max(x0, x1);
	sy = min(y0, y1);
	ly = max(y0, y1);
	if (gradient == 0) {
		for (int i = findn(sx, size, 0); i <= findn(lx, size, 1); i++) {
			mininode[updown].push_back(make_pair(i*size, y0));
		}
	}
	else {
		for (int i = findn(sx, size, 0); i <= findn(lx, size, 1); i++) {
			mininode[updown].push_back(make_pair(i*size, gradient* (i*size - x0) + y0));

		}
		for (int i = findn(sy, size, 0); i <= findn(ly, size, 1); i++) {
			double x = (i*size - y0) / gradient + x0;
			mininode[updown].push_back(make_pair(x, i*size));
		}
	}
	//8자리수 이하 오차 제거
	sort(mininode[updown].begin(), mininode[updown].end());
	vector<pdd>::iterator posy = unique(mininode[updown].begin(), mininode[updown].end());
	mininode[updown].erase(posy, mininode[updown].end());
}
bool innerpoint(Cell* c, pdd n) {
	if (c->getLDNode()->x <= n.first && n.first <= c->getRDNode()->x) {
		if (c->getLDNode()->y <= n.second && n.second <= c->getLUNode()->y) return true;
	}
	return false;
}

node_of_cell Node::findtailCell(pdd n) { // 진 : tail 설정  2017-03-07
	vector<Cell*>::iterator it = cellVector.begin();
	for (it; it < cellVector.end(); it++) {
		if ((*it)->getLUNode() == this) {
			if (innerpoint((*it), n)) return make_pair(1, (*it));
		}
		else if ((*it)->getLDNode() == this) {
			if (innerpoint((*it), n)) return make_pair(2, (*it));
		}
		else if ((*it)->getRUNode() == this) {
			if (innerpoint((*it), n)) return make_pair(3, (*it));
		}
		else if ((*it)->getRDNode() == this) {
			if (innerpoint((*it), n)) return make_pair(4, (*it));
		}
	}
	printf("\nerror_findtialCell_Node_Node\n");
	return make_pair(0, (*cellVector.begin()));
}

CellShape* Node::findCell(Node* n) { // 재 : Node와 Node일때 mininodeCell 설정 2017-01-12
	vector<Cell*>::iterator it = cellVector.begin();
	for (it; it < cellVector.end(); it++) {
		if ((*it)->getLUNode() == n) {
			if ((*it)->checkNode(this))
				return (*it);
		}
		else if ((*it)->getLDNode() == n) {
			if ((*it)->checkNode(this))
				return (*it);
		}
		else if ((*it)->getRUNode() == n) {
			if ((*it)->checkNode(this))
				return (*it);
		}
		else if ((*it)->getRDNode() == n) {
			if ((*it)->checkNode(this))
				return (*it);
		}
	}
	printf("\nerror_findCell_Node_Node\n");
	return NULL;
}
CellShape* Node::findCell(Face* f) { // 재 : Node와 face일때 mininodeCell 설정 2017-01-12
	if (f->eCell) {
		if (((Cell*)(f->eCell))->checkNode(this))
			return f->eCell;
	}
	if (f->sCell) {
		if (((Cell*)(f->sCell))->checkNode(this))
			return f->sCell;
	}
	else
		printf("\nerror_findCell_Node_Face\n");
	return NULL;
}
CellShape* Face::findCell(Node* n) { // 재 : face와 Node일때 mininodeCell 설정 2017-01-12
	if (this->eCell) {
		if (((Cell*)(this->eCell))->checkNode(n))
			return this->eCell;
	}
	if (this->sCell) {
		if (((Cell*)(this->sCell))->checkNode(n))
			return this->sCell;
	}
	else
		printf("\nerror_findCell_Face_Node\n");
	return NULL;
}
CellShape* Face::findCell(Face* f) { // 재 : face와 face일때 mininodeCell 설정 2017-01-12
	if (this->eCell) {
		if (((Cell*)(this->eCell))->checkFace(f))
			return this->eCell;
	}
	if (this->sCell) {
		if (((Cell*)(this->sCell))->checkFace(f))
			return this->sCell;
	}
	else
		printf("\nerror_findCell_Face_Face\n");
	return NULL;
}

bool setupmininodeCell(pdd p1, pdd p2) { // 재 : mininodePlace를 통해 mininodeCell를 설정 2017-01-12
	pair< int, void* > pre = mininodePlace[p1]; // main의 반복문에서 p1은 mininode[i-1]
	pair< int, void* > now = mininodePlace[p2]; // main의 반복문에서 p2는 mininode[i]
	if (pre.first == 0) {//node
		if (now.first == 0)
			mininodeCell[p1][p2] = ((Node*)pre.second)->findCell((Node*)now.second);
		else if (now.first == 1)
			mininodeCell[p1][p2] = ((Node*)pre.second)->findCell((Face*)now.second);
		else {
			printf("\nerror_setupmininodeCell_0\n");
			return false;
		}
	}
	else if (pre.first == 1) {//face
		if (now.first == 0)
			mininodeCell[p1][p2] = ((Face*)pre.second)->findCell((Node*)now.second);
		else if (now.first == 1)
			mininodeCell[p1][p2] = ((Face*)pre.second)->findCell((Face*)now.second);
		else {
			printf("\nerror_setupmininodeCell_1\n");
			return false;
		}
	}
	if (mininodeCell[p1][p2] == NULL)
		printf("\nerror_setupmininodeCell_2\n");
	//else printf("\nmininodeCell[%lf, %lf][%lf, %lf] = %d\n", p1.first, p1.second, p2.first, p2.second, ((Cell*)mininodeCell[p1][p2])->id); // 재 : 확인용
	return true;
}
void Rectangle::splitCell(double x1, double y1, double x2, double y2, int layer, int splitDepth, int depth) { // 재 : Cell split 함수 입니다. 점 두개와 layer, splitDepth, depth를 입력받습니다. 2017-02-20
	int findX1 = 1, findY1 = 1;
	int findX2 = 1, findY2 = 1;
	int flag = 0; // flag 0: 기본 1: x경계에 걸쳐짐 2: y 경계에 걸쳐짐 3: 둘다 걸쳐짐 // 재 : 범위의 왼쪽 밑 점이 경계에 걸칠 때 조치 2017-01-22

	if (splitDepth > depth) {
		cout << "split Cell의 depth가 main의 depth를 초과하여 main의 depth로 조정하겠습니다." << endl;
		splitDepth = depth;
	}
	while (true) {
		if (((findX1 * initCellSize) + LDx) > x1) {
			break;
		}
		else if (((findX1 * initCellSize) + LDx) == x1) flag = 1; // 재 : 범위의 왼쪽 밑 점이 경계에 걸칠 때 조치 2017-01-22

		findX1++;
	}
	while (true) {
		if (((findY1 * initCellSize) + LDy) > y1) {
			break;
		}
		else if (((findY1 * initCellSize) + LDy) == y1) { // 재 : 범위의 왼쪽 밑 점이 경계에 걸칠 때 조치 2017-01-22
			if (flag == 1) flag = 3;
			else flag = 2;
		}
		findY1++;
	}

	while (true) {
		if (initCell[findX2 - 1] == NULL) { // 재 : boundary error 수정 2017_01_31
			findX2--;
			break;
		}
		if (((findX2 * initCellSize) + LDx) > x2) {
			break;
		}
		findX2++;
	}
	while (true) {
		if (initCell[findY2 - 1] == NULL) { // 재 : boundary error 수정 2017_01_31
			findY2--;
			break;
		}if (((findY2 * initCellSize) + LDy) > y2) {
			break;
		}
		findY2++;
	}

	if (flag == 1 || flag == 3 || findX1 > 2) // 재 : 범위 관련 수정 2017-01-22
		findX1--;
	if (flag == 2 || flag == 3 || findY1 > 2) // 재 : 범위 관련 수정 2017-01-22
		findY1--;

	for (int i = findX1; i < findX2 + 1; i++) {
		for (int j = findY1; j < findY2 + 1; j++) {
			initCell[i - 1][j - 1].splitCell(layer, splitDepth); // 재 : depth 추가, Class Cell의 멤버함수 splitCell 사용 2017-01-22
		}
	}
}

double ccw(pdd a, pdd b) { //진_01_22
	return a.first*b.second - a.second*b.first;
}
double ccw(pdd a, pdd b, pdd p) {//진_01_22 :선분 ap 를 기준으로 위에 있으면 음수 밑에 있으면 양수 선분위의 점은 0 출력
	return ccw(make_pair(a.first - p.first, a.second - p.second), make_pair(b.first - p.first, b.second - p.second));
}
double distancePTP(pdd a, pdd b) { // 재 : 두점 사이의 거리의 제곱을 구하는 함수 2017_02_13
	return (((b.first - a.first) * (b.first - a.first)) + ((b.second - a.second) * (b.second - a.second)));
}
double ccwAddition(pdd a, pdd b, pdd p) { // 재 : 기존 ccw 사용시 반환값이 0일 때 문제가 발생하여 일부 수정된 함수를 만듬. 2017_02_13
	double temp = ccw(make_pair(a.first - p.first, a.second - p.second), make_pair(b.first - p.first, b.second - p.second));
	if (temp > 0)
		return 1;
	else if (temp < 0)
		return -1;
	else { // 재 : 점과 선분의 기울기가 같을 경우 점이 선분위에 있다면 0을 반환하고 그렇지 않다면 1을 반환 2017_02_13
		pdd middlePoint = make_pair((a.first + b.first) / 2, (a.second + b.second) / 2);  // 재 : 점 a와 점 b의 중간점. 2017_02_13
		double length = sqrt(distancePTP(a, b));
		if (distancePTP(p, middlePoint) > length)
			return 1;
	}
	return 0;
}
vector<pdd> centermaking(vector<pdd> centernode, pdd cnode, Cell* c, double mincellsize) { //진_cutcell수정:
	Face *  cface = (Face*)mininodePlace[cnode].second;
	//0 : left , 1: down 2: right, 3: up 4: center
	if (cface == c->LFace) {
		centernode[0] = cnode;
		centernode[2] = make_pair(cnode.first + mincellsize, cnode.second);
	}
	else if (cface == c->RFace) {
		centernode[2] = cnode;
		centernode[0] = make_pair(cnode.first - mincellsize, cnode.second);
	}
	else if (cface == c->DFace) {
		centernode[1] = cnode;
		centernode[3] = make_pair(cnode.first, cnode.second + mincellsize);
	}
	else if (cface == c->UFace) {
		centernode[3] = cnode;
		centernode[1] = make_pair(cnode.first, cnode.second - mincellsize);
	}
	else { cout << " error : centermaking _cell 안에 face가 없음" << endl; }
	return centernode;
}

void cuting(Cell* c, pdd fnode, pdd snode, double mincellsize, int updown, double gradient) {//오각형
	vector<pdd> centernode = vector<pdd>(5);//0 : left , 1: down 2: right, 3: up 4: center
	centernode = centermaking(centernode, fnode, c, mincellsize);
	centernode = centermaking(centernode, snode, c, mincellsize);
	centernode[4] = make_pair(centernode[1].first, centernode[0].second);

	int deletenode = 0; // LD=3 LU=6 UR=11 DR=8

	c->LUCell = new Cell();    ((Cell*)c->LUCell)->setDepth(c->depth + 1);
	c->LDCell = new Cell();    ((Cell*)c->LDCell)->setDepth(c->depth + 1);
	c->RDCell = new Cell();    ((Cell*)c->RDCell)->setDepth(c->depth + 1);
	c->RUCell = new Cell();    ((Cell*)c->RUCell)->setDepth(c->depth + 1);
	cellVector.push_back(((Cell*)c->LUCell));    cellVector.push_back(((Cell*)c->LDCell));
	cellVector.push_back(((Cell*)c->RDCell));    cellVector.push_back(((Cell*)c->RUCell));

	// 새로 만들어지는 Node를 먼저 생성한다.
	//0 : left , 1: down 2: right, 3: up 4: center
	Node *LeftCenter, *DownCenter, *RightCenter, *UpCenter, *cellCenter;
	if (!c->LFace->isParent()) {
		LeftCenter = new Node(centernode[0].first, centernode[0].second);
		nodeVector.push_back(LeftCenter);
	}
	else LeftCenter = c->LFace->centerNode;

	if (!c->DFace->isParent()) {
		DownCenter = new Node(centernode[1].first, centernode[1].second);
		nodeVector.push_back(DownCenter);
	}
	else DownCenter = c->DFace->centerNode;

	if (!c->RFace->isParent()) {
		RightCenter = new Node(centernode[2].first, centernode[2].second);
		nodeVector.push_back(RightCenter);
	}
	else RightCenter = c->RFace->centerNode;

	if (!c->UFace->isParent()) {
		UpCenter = new Node(centernode[3].first, centernode[3].second);
		nodeVector.push_back(UpCenter);
	}
	else UpCenter = c->UFace->centerNode;

	cellCenter = new Node(centernode[4].first, centernode[4].second);
	nodeVector.push_back(cellCenter);

	// 내부 Cell의 Node를 세팅한다.
	c->setChildNodes(LeftCenter, DownCenter, RightCenter, UpCenter, cellCenter, 0);

	// 내부 Face 12개를 만들겁니다.
	Face *LU = 0, *LD = 0, *DL = 0, *DR = 0, *RD = 0, *RU = 0;
	Face *UR = 0, *UL = 0, *IL = 0, *ID = 0, *IR = 0, *IU = 0;

	// Cell 만들 때 새로 만들어지는 Face중 외부 8개 먼저 셋팅
	// LD LU

	if (!c->LFace->isParent()) {
		LD = new Face(c->LDNode, LeftCenter);    faceVector.push_back(LD);
		LU = new Face(LeftCenter, c->LUNode);    faceVector.push_back(LU);
		if (updown*gradient < 0) {
			if (updown > 0) {
				detfaceVector.push_back(make_pair(LD->id, LD));
				LD = LU;
				deletenode += 2;
			}
			else {
				detfaceVector.push_back(make_pair(LU->id, LU));
				LU = LD;
				deletenode += 2;
			}
		}
		LD->setAttribute(c->LFace->attribute);
		LU->setAttribute(c->LFace->attribute);
		c->LFace->setChilds(LD, LU);
		c->LFace->setCenterNode(LeftCenter);
	}
	else {
		if (updown*gradient < 0) deletenode += 2;
		LD = c->LFace->Child1;
		LU = c->LFace->Child2;
	}

	// DL DR
	if (!c->DFace->isParent()) {
		DL = new Face(c->LDNode, DownCenter);    faceVector.push_back(DL);
		DR = new Face(DownCenter, c->RDNode);    faceVector.push_back(DR);
		if (updown > 0) {
			if (gradient > 0) {
				detfaceVector.push_back(make_pair(DR->id, DR));
				DR = DL;
				deletenode += 1;
			}
			else {
				detfaceVector.push_back(make_pair(DL->id, DL));
				DL = DR;
				deletenode += 1;
			}
		}
		DL->setAttribute(c->DFace->attribute);
		DR->setAttribute(c->DFace->attribute);
		c->DFace->setChilds(DL, DR);
		c->DFace->setCenterNode(DownCenter);
	}
	else {
		if (updown > 0) deletenode += 1;
		DL = c->DFace->Child1;
		DR = c->DFace->Child2;
	}

	// RD RU
	if (!c->RFace->isParent()) {
		RD = new Face(c->RDNode, RightCenter);    faceVector.push_back(RD);
		RU = new Face(RightCenter, c->RUNode);    faceVector.push_back(RU);
		if (gradient*updown > 0) {
			if (updown > 0) {
				detfaceVector.push_back(make_pair(RD->id, RD));
				RD = RU;
				deletenode += 7;
			}
			else {
				detfaceVector.push_back(make_pair(RU->id, RU));
				RU = RD;
				deletenode += 7;
			}
		}
		RD->setAttribute(c->RFace->attribute);
		RU->setAttribute(c->RFace->attribute);
		c->RFace->setChilds(RD, RU);
		c->RFace->setCenterNode(RightCenter);
	}
	else {
		if (gradient*updown > 0) deletenode += 7;
		RD = c->RFace->Child1;
		RU = c->RFace->Child2;
	}

	// UL UR
	if (!c->UFace->isParent()) {
		UL = new Face(c->LUNode, UpCenter);    faceVector.push_back(UL);
		UR = new Face(UpCenter, c->RUNode);    faceVector.push_back(UR);
		if (updown < 0) {
			if (gradient > 0) {
				detfaceVector.push_back(make_pair(UL->id, UL));
				UL = UR;
				deletenode += 4;
			}
			else {//오류 잡은부분
				detfaceVector.push_back(make_pair(UR->id, UR));
				UR = UL;
				deletenode += 4;
			}
		}
		UL->setAttribute(c->UFace->attribute);
		UR->setAttribute(c->UFace->attribute);
		c->UFace->setChilds(UL, UR);
		c->UFace->setCenterNode(UpCenter);
	}
	else {
		if (updown < 0)deletenode += 4;
		UL = c->UFace->Child1;
		UR = c->UFace->Child2;
	}


	IL = new Face(LeftCenter, cellCenter);    IL->setAttribute(4);    IL->setDepth(UL->depth);    faceVector.push_back(IL);
	ID = new Face(DownCenter, cellCenter);    ID->setAttribute(4);    ID->setDepth(UL->depth);    faceVector.push_back(ID);
	IR = new Face(cellCenter, RightCenter);    IR->setAttribute(4);    IR->setDepth(UL->depth);    faceVector.push_back(IR);
	IU = new Face(cellCenter, UpCenter);    IU->setAttribute(4);    IU->setDepth(UL->depth);    faceVector.push_back(IU);

	// 각 내부 cell이 face를 가리키도록
	((Cell*)c->LUCell)->LFace = LU;    ((Cell*)c->LUCell)->DFace = IL;    ((Cell*)c->LUCell)->RFace = IU;    ((Cell*)c->LUCell)->UFace = UL;
	((Cell*)c->LDCell)->LFace = LD;    ((Cell*)c->LDCell)->DFace = DL;    ((Cell*)c->LDCell)->RFace = ID;    ((Cell*)c->LDCell)->UFace = IL;
	((Cell*)c->RDCell)->LFace = ID;    ((Cell*)c->RDCell)->DFace = DR;    ((Cell*)c->RDCell)->RFace = RD;    ((Cell*)c->RDCell)->UFace = IR;
	((Cell*)c->RUCell)->LFace = IU;    ((Cell*)c->RUCell)->DFace = IR;    ((Cell*)c->RUCell)->RFace = RU;    ((Cell*)c->RUCell)->UFace = UR;

	// 각 Face의 startCell / endCell 셋팅 // 재 : Face의 sCell과 eCell도 CellShape로 변경하였기 때문에 밑에 코드도 약간 수정했습니다.  2017-01-12

	// LU, LD
	if (deletenode != 6)LU->setEndCell(((Cell*)c->LUCell));
	if (deletenode != 3)LD->setEndCell(((Cell*)c->LDCell));
	if (c->LFace->attribute != 1) {
		if (deletenode != 6 && LU->getStartCell() == NULL)LU->setStartCell(c->LFace->sCell);
		if (deletenode != 3 && LD->getStartCell() == NULL)LD->setStartCell(c->LFace->sCell);
	}
	// DL, DR
	if (deletenode != 3)DL->setStartCell(((Cell*)c->LDCell));
	if (deletenode != 8)DR->setStartCell(((Cell*)c->RDCell));
	if (c->DFace->attribute != 3) {
		if (deletenode != 3 && DL->getEndCell() == NULL)DL->setEndCell(c->DFace->eCell);
		if (deletenode != 8 && DR->getEndCell() == NULL)DR->setEndCell(c->DFace->eCell);
	}
	// RD, RU
	if (deletenode != 8)RD->setStartCell(((Cell*)c->RDCell));
	if (deletenode != 11)RU->setStartCell(((Cell*)c->RUCell));
	if (c->RFace->attribute != 0) {
		if (deletenode != 8 && RD->getEndCell() == NULL)RD->setEndCell(c->RFace->eCell);
		if (deletenode != 11 && RU->getEndCell() == NULL)RU->setEndCell(c->RFace->eCell);
	}
	// UR, UL
	if (deletenode != 11)UR->setEndCell(((Cell*)c->RUCell));
	if (deletenode != 6)UL->setEndCell(((Cell*)c->LUCell));
	if (c->UFace->attribute != 2) {
		if (deletenode != 11 && UR->getStartCell() == NULL)UR->setStartCell(c->UFace->sCell);
		if (deletenode != 6 && UL->getStartCell() == NULL)UL->setStartCell(c->UFace->sCell);
	}
	// INNER Faces
	IL->setStartCell(((Cell*)c->LUCell));    IL->setEndCell(((Cell*)c->LDCell));
	ID->setStartCell(((Cell*)c->LDCell));    ID->setEndCell(((Cell*)c->RDCell));
	IR->setStartCell(((Cell*)c->RUCell));    IR->setEndCell(((Cell*)c->RDCell));
	IU->setStartCell(((Cell*)c->LUCell));    IU->setEndCell(((Cell*)c->RUCell));

	//오각형 node inout설정 2017_03_14 재
	if (deletenode == 3) {//LD
						  //delete
		detcellVector.push_back(make_pair(c->LDCell->id, (Cell*)c->LDCell));
		//삼각형생성
		c->LDCell = new CellTriangle(DownCenter, LeftCenter, cellCenter);
		((Cell*)c->LUCell)->DFace->eCell = c->LDCell;
		((Cell*)c->RDCell)->LFace->sCell = c->LDCell;
		minimalCellVector_tri.push_back(c->LDCell);
		wall.push_back(make_pair(make_pair(1, c->LDCell->id - 1), make_pair(c->LFace->centerNode->id - 1, c->DFace->centerNode->id - 1)));
		//재 : 추가 2017_03_14
		LeftCenter->inout = -2;
		DownCenter->inout = -2;
	}
	else if (deletenode == 6) {//LU
							   //delete
		detcellVector.push_back(make_pair(c->LUCell->id, (Cell*)c->LUCell));
		//삼각형생성
		c->LUCell = new CellTriangle(UpCenter, LeftCenter, cellCenter);
		((Cell*)c->LDCell)->UFace->sCell = c->LUCell;
		((Cell*)c->RUCell)->LFace->sCell = c->LUCell;
		minimalCellVector_tri.push_back(c->LUCell);
		wall.push_back(make_pair(make_pair(1, c->LUCell->id - 1), make_pair(c->UFace->centerNode->id - 1, c->LFace->centerNode->id - 1)));
		//재 : 추가 2017_03_14
		LeftCenter->inout = -2;
		UpCenter->inout = -2;
	}
	else if (deletenode == 11) {//UR
								//delete
		detcellVector.push_back(make_pair(c->RUCell->id, (Cell*)c->RUCell));
		//삼각형생성
		c->RUCell = new CellTriangle(UpCenter, RightCenter, cellCenter);
		((Cell*)c->RDCell)->UFace->sCell = c->RUCell;
		((Cell*)c->LUCell)->RFace->eCell = c->RUCell;
		minimalCellVector_tri.push_back(c->RUCell);
		wall.push_back(make_pair(make_pair(1, c->RUCell->id - 1), make_pair(c->RFace->centerNode->id - 1, c->UFace->centerNode->id - 1)));
		//재 : 추가 2017_03_14
		RightCenter->inout = -2;
		UpCenter->inout = -2;
	}
	else if (deletenode == 8) {//DR
							   //delete
		detcellVector.push_back(make_pair(c->RDCell->id, (Cell*)c->RDCell));
		//삼각형생성
		c->RDCell = new CellTriangle(UpCenter, LeftCenter, cellCenter);
		((Cell*)c->LDCell)->RFace->eCell = c->RDCell;
		((Cell*)c->RUCell)->DFace->eCell = c->RDCell;
		minimalCellVector_tri.push_back(c->RDCell);
		wall.push_back(make_pair(make_pair(1, c->RDCell->id - 1), make_pair(c->DFace->centerNode->id - 1, c->RFace->centerNode->id - 1)));
		//재 : 추가 2017_03_14
		RightCenter->inout = -2;
		DownCenter->inout = -2;
	}

}
int cutface(pdd CenterPoint, Face* face, int child1or2) { //child1or2 = child1,2중 어떤 쪽을 살리는가
	Node * Centernode;
	Centernode = new Node(CenterPoint.first, CenterPoint.second);
	nodeVector.push_back(Centernode);
	Face * child;
	if (child1or2 == 1) child = new Face(face->sNode, Centernode);
	else child = new Face(Centernode, face->eNode);
	faceVector.push_back(child);
	child->setAttribute(face->attribute);
	face->setChilds(child, child);
	face->setCenterNode(Centernode);
	child->setStartCell(face->getStartCell());
	child->setEndCell(face->getEndCell());
	//추가 2017_03_15 재
	Centernode->inout = -2; // 도형의 위
	return Centernode->id;
}
void eraseRect(Cell* c, pdd fnode, pdd snode, int wopoint) { // 사각형 처리
															 // wopoint 1 = DFace , 2= RFace , 4=LFace , 5 = UFace
	int wallsid, walleid;
	int norf_f = mininodePlace[fnode].first;
	int norf_s = mininodePlace[snode].first;
	Face* face_f = NULL;
	Face* face_s = NULL;
	if (norf_f == 1) face_f = (Face*)mininodePlace[fnode].second;
	if (norf_s == 1) face_s = (Face*)mininodePlace[snode].second;

	//DFace이면 Dface가 없어지는 형식 centerNode가 없고 있고에 따라 다른 유형 2017_03_14 재
	if (wopoint == 1) {//DFACE
		if (norf_f == 1) { //face이면
			if (!face_f->isParent()) wallsid = cutface(fnode, face_f, 2);// child가 없으면 쪼개고 중간점을 wall의 snode
			else {
				wallsid = face_f->centerNode->id; //쪼개져있으면 centernode =  wall의 snode
												  //추가 2017_03_15 재
				face_f->centerNode->inout = -2; // 도형위의 점
			}
			c->LFace->eCell = NULL;
		}
		else {// node면 해당 node가 wall의 snode
			wallsid = ((Node*)mininodePlace[fnode].second)->id;
			//추가 2017_03_15 재
			((Node*)mininodePlace[fnode].second)->inout = -2;// 도형위의 점
		}

		if (norf_s == 1) {
			if (!face_s->isParent()) walleid = cutface(snode, face_s, 2);
			else {
				walleid = face_s->centerNode->id;
				//추가 2017_03_15 재
				face_s->centerNode->inout = -2; // 도형위의 점
			}
			c->RFace->sCell = NULL;
		}
		else {
			walleid = ((Node*)mininodePlace[snode].second)->id;
			//추가 2017_03_15 재
			((Node*)mininodePlace[snode].second)->inout = -2;// 도형위의 점
		}
		c->DFace->sCell = NULL;
	}
	else if (wopoint == 2) {//RFace
		if (norf_f == 1) { //face이면
			if (!face_f->isParent()) wallsid = cutface(fnode, face_f, 1);// child가 없으면 쪼개고 중간점을 wall의 snode
			else {
				wallsid = face_f->centerNode->id; //쪼개져있으면 centernode =  wall의 snode
												  //추가 2017_03_15 재
				face_f->centerNode->inout = -2; // 도형위의 점
			}
			c->DFace->sCell = NULL;
		}
		else {// node면 해당 node가 wall의 snode
			wallsid = ((Node*)mininodePlace[fnode].second)->id;
			//추가 2017_03_15 재
			((Node*)mininodePlace[fnode].second)->inout = -2; // 도형위의 점
		}

		if (norf_s == 1) {
			if (!face_s->isParent()) walleid = cutface(snode, face_s, 1);
			else {
				walleid = face_s->centerNode->id;

				//추가 2017_03_15 재
				face_s->centerNode->inout = -2; // 도형위의 점
			}
			c->UFace->eCell = NULL;
		}
		else {
			walleid = ((Node*)mininodePlace[snode].second)->id;
			//추가 2017_03_15 재
			((Node*)mininodePlace[snode].second)->inout = -2; // 도형위의 점
		}
		c->RFace->sCell = NULL;
	}
	else if (wopoint == 4) {//LFace
		if (norf_f == 1) { //face이면
			if (!face_f->isParent()) wallsid = cutface(fnode, face_f, 2);// child가 없으면 쪼개고 중간점을 wall의 snode
			else {
				wallsid = face_f->centerNode->id; //쪼개져있으면 centernode =  wall의 snode

												  //추가 2017_03_15 재
				face_f->centerNode->inout = -2; // 도형위의 점
			}
			c->UFace->eCell = NULL;
		}
		else {// node면 해당 node가 wall의 snode
			wallsid = ((Node*)mininodePlace[fnode].second)->id;
			//추가 2017_03_15 재
			((Node*)mininodePlace[fnode].second)->inout = -2; // 도형위의 점
		}

		if (norf_s == 1) {
			if (!face_s->isParent()) walleid = cutface(snode, face_s, 2);
			else {
				walleid = face_s->centerNode->id;
				//추가 2017_03_15 재
				face_s->centerNode->inout = -2; // 도형위의 점
			}
			c->DFace->sCell = NULL;
		}
		else {
			walleid = ((Node*)mininodePlace[snode].second)->id;
			//추가 2017_03_15 재
			((Node*)mininodePlace[snode].second)->inout = -2; // 도형위의 점
		}
		c->LFace->eCell = NULL;
	}
	else if (wopoint == 5) {//UFace
		if (norf_f == 1) { //face이면
			if (!face_f->isParent()) wallsid = cutface(fnode, face_f, 1);// child가 없으면 쪼개고 중간점을 wall의 snode
			else {
				wallsid = face_f->centerNode->id; //쪼개져있으면 centernode =  wall의 snode
												  //추가 2017_03_15 재
				face_s->centerNode->inout = -2; // 도형위의 점
			}
			c->RFace->sCell = NULL;
		}
		else {// node면 해당 node가 wall의 snode
			wallsid = ((Node*)mininodePlace[fnode].second)->id;
			//추가 2017_03_15 재
			((Node*)mininodePlace[fnode].second)->inout = -2; // 도형위의 점
		}

		if (norf_s == 1) {
			if (!face_s->isParent()) walleid = cutface(snode, face_s, 1);
			else {
				walleid = face_s->centerNode->id;
				//추가 2017_03_15 재
				face_s->centerNode->inout = -2; // 도형위의 점
			}
			c->LFace->eCell = NULL;
		}
		else {
			walleid = ((Node*)mininodePlace[snode].second)->id;
			//추가 2017_03_15 재
			((Node*)mininodePlace[snode].second)->inout = -2; // 도형위의 점
		}
		c->UFace->eCell = NULL;
	}
	else cout << "Eroor : eraseRect() " << endl;

	wall.push_back(make_pair(make_pair(0, c->id - 1), make_pair(wallsid - 1, walleid - 1)));
}

void eraseTri(Cell* c, pdd fnode, pdd snode, int wopoint) {//삼각형처리
														   //wopoint 0 = LUnode ,1 = RUnode , 2 = LDnode , 3 = RDnode
	int norf_f = mininodePlace[fnode].first;
	int norf_s = mininodePlace[snode].first;
	//원래 사각cell 삭제
	detcellVector.push_back(make_pair(c->id, c));

	if (wopoint == 0) {//LU
					   //삼각형의 HFace, VFace 생성과 HO,VO 생성
		Node* LeftCenter;    Node* UPCenter;
		Face* LeftFace;        Face* UpFace;
		if (norf_f == 1) {
			if (!c->LFace->isParent()) { //child 가없다면 쪼갠다.
				LeftCenter = new Node(fnode.first, fnode.second);
				nodeVector.push_back(LeftCenter);
				Face * LU = new Face(LeftCenter, c->LUNode);
				LU->setAttribute(c->LFace->attribute);
				LU->setStartCell(c->LFace->sCell);
				faceVector.push_back(LU);
				c->LFace->setChilds(LU, LU);
				c->LFace->setCenterNode(LeftCenter);
				LeftFace = ((Cell*)c->LFace->sCell)->RFace->Child1;
			}
			else {
				LeftCenter = c->LFace->centerNode;
				LeftFace = c->LFace->Child2;
			}
		}
		else {
			LeftCenter = c->LDNode;
			LeftFace = c->LFace;
		}

		if (norf_s == 1) {
			if (!c->UFace->isParent()) { //child 가없다면 쪼갠다.
				UPCenter = new Node(snode.first, snode.second);
				nodeVector.push_back(UPCenter);
				Face * UL = new Face(c->LUNode, UPCenter);
				UL->setAttribute(c->UFace->attribute);
				UL->setStartCell(c->UFace->sCell);
				faceVector.push_back(UL);
				c->UFace->setChilds(UL, UL);
				c->UFace->setCenterNode(UPCenter);
				UpFace = ((Cell*)c->UFace->sCell)->DFace->Child1;
			}
			else {
				UPCenter = c->UFace->centerNode;
				UpFace = c->UFace->Child1;
			}
		}
		else {
			UPCenter = c->RUNode;
			UpFace = c->UFace;

		}
		// 삼각형생성
		CellTriangle* tri = new CellTriangle(LeftCenter, UPCenter, c->LUNode);
		minimalCellVector_tri.push_back(tri);
		LeftFace->eCell = tri;
		UpFace->eCell = tri;
		wall.push_back(make_pair(make_pair(1, tri->id - 1), make_pair(LeftCenter->id - 1, UPCenter->id - 1)));
		//추가 2017_03_15 재
		tri->HONode->inout = -2;
		tri->VONode->inout = -2;
		//추가 2017_07_31 재
		c->RFace->eCell = NULL;
		c->DFace->eCell = NULL;
		c->RFace->sCell = NULL;
		c->DFace->sCell = NULL;
		//추가 2017_07_31 재
		if (norf_f == 1)
			c->LFace->eCell = NULL;
		else
			c->LFace->eCell = tri;
		if (norf_s == 1)
			c->UFace->eCell = NULL;
		else
			c->UFace->eCell = tri;
	}
	else if (wopoint == 1) {//RU
							//삼각형의 HFace, VFace 생성과 HO,VO 생성
		Node* RightCenter;    Node* UPCenter;
		Face* RightFace;        Face* UpFace;
		if (norf_f == 1) {
			if (!c->UFace->isParent()) { //child 가없다면 쪼갠다.
				UPCenter = new Node(fnode.first, fnode.second);
				nodeVector.push_back(UPCenter);
				Face * UR = new Face(UPCenter, c->RUNode);
				UR->setAttribute(c->UFace->attribute);
				UR->setStartCell(c->UFace->sCell);
				faceVector.push_back(UR);
				c->UFace->setChilds(UR, UR);
				c->UFace->setCenterNode(UPCenter);
				UpFace = ((Cell*)c->UFace->sCell)->DFace->Child1;
			}
			else {
				UPCenter = c->UFace->centerNode;
				UpFace = c->UFace->Child2;
			}
		}
		else {
			UPCenter = c->LUNode;
			UpFace = c->UFace;
		}

		if (norf_s == 1) {
			if (!c->RFace->isParent()) { //child 가없다면 쪼갠다.
				RightCenter = new Node(snode.first, snode.second);
				nodeVector.push_back(RightCenter);
				Face * RU = new Face(RightCenter, c->RUNode);
				RU->setAttribute(c->RFace->attribute);
				RU->setEndCell(c->RFace->eCell);
				faceVector.push_back(RU);
				c->RFace->setChilds(RU, RU);
				c->RFace->setCenterNode(RightCenter);
				RightFace = ((Cell*)c->RFace->eCell)->LFace->Child1;
			}
			else {
				RightCenter = c->RFace->centerNode;
				RightFace = c->RFace->Child2;
			}
		}
		else {
			RightCenter = c->RDNode;
			RightFace = c->RFace;

		}
		// 삼각형생성
		CellTriangle* tri = new CellTriangle(RightCenter, UPCenter, c->RUNode);
		minimalCellVector_tri.push_back(tri);
		RightFace->sCell = tri;
		UpFace->eCell = tri;
		wall.push_back(make_pair(make_pair(1, tri->id - 1), make_pair(UPCenter->id - 1, RightCenter->id - 1)));
		//추가 2017_03_15 재
		tri->HONode->inout = -2;
		tri->VONode->inout = -2;
		//추가 2017_07_31 재
		c->LFace->eCell = NULL;
		c->DFace->eCell = NULL;
		c->LFace->sCell = NULL;
		c->DFace->sCell = NULL;
		//추가 2017_07_31 재
		if (norf_f == 1)
			c->UFace->eCell = NULL;
		else
			c->UFace->eCell = tri;
		if (norf_s == 1)
			c->RFace->sCell = NULL;
		else
			c->RFace->sCell = tri;

	}
	else if (wopoint == 2) {//LD
							//삼각형의 HFace, VFace 생성과 HO,VO 생성
		Node* LeftCenter;    Node* DownCenter;
		Face* LeftFace;        Face* DownFace;
		if (norf_f == 1) {
			if (!c->DFace->isParent()) { //child 가없다면 쪼갠다.
				DownCenter = new Node(fnode.first, fnode.second);
				nodeVector.push_back(DownCenter);
				Face * DL = new Face(c->LDNode, DownCenter);
				DL->setAttribute(c->DFace->attribute);
				DL->setEndCell(c->DFace->eCell);
				faceVector.push_back(DL);
				c->DFace->setChilds(DL, DL);
				c->DFace->setCenterNode(DownCenter);
				DownFace = ((Cell*)c->DFace->eCell)->UFace->Child1;
			}
			else {
				DownCenter = c->DFace->centerNode;
				DownFace = c->DFace->Child1;
			}
		}
		else {
			DownCenter = c->RDNode;
			DownFace = c->DFace;
		}

		if (norf_s == 1) {
			if (!c->LFace->isParent()) { //child 가없다면 쪼갠다.
				LeftCenter = new Node(snode.first, snode.second);
				nodeVector.push_back(LeftCenter);
				Face * LD = new Face(c->LDNode, LeftCenter);
				LD->setAttribute(c->LFace->attribute);
				LD->setStartCell(c->LFace->sCell);
				faceVector.push_back(LD);
				c->LFace->setChilds(LD, LD);
				c->LFace->setCenterNode(LeftCenter);
				LeftFace = ((Cell*)c->LFace->sCell)->RFace->Child1;
			}
			else {
				LeftCenter = c->LFace->centerNode;
				LeftFace = c->LFace->Child1;
			}
		}
		else {
			LeftCenter = c->LUNode;
			LeftFace = c->LFace;

		}
		// 삼각형생성
		CellTriangle* tri = new CellTriangle(LeftCenter, DownCenter, c->LDNode);
		minimalCellVector_tri.push_back(tri);
		LeftFace->eCell = tri;
		DownFace->sCell = tri;
		wall.push_back(make_pair(make_pair(1, tri->id - 1), make_pair(DownCenter->id - 1, LeftCenter->id - 1)));
		//추가 2017_03_15 재
		tri->HONode->inout = -2;
		tri->VONode->inout = -2;
		//추가 2017_07_31 재
		c->RFace->eCell = NULL;
		c->UFace->eCell = NULL;
		c->RFace->sCell = NULL;
		c->UFace->sCell = NULL;
		//추가 2017_07_31 재
		if (norf_f == 1)
			c->DFace->sCell = NULL;
		else
			c->DFace->sCell = tri;
		if (norf_s == 1)
			c->LFace->eCell = NULL;
		else
			c->LFace->eCell = tri;
	}
	else if (wopoint == 3) {//RD
							//삼각형의 HFace, VFace 생성과 HO,VO 생성
		Node* RightCenter;    Node* DownCenter;
		Face* RightFace;    Face* DownFace;
		if (norf_f == 1) {
			if (!c->RFace->isParent()) { //child 가없다면 쪼갠다.
				RightCenter = new Node(fnode.first, fnode.second);
				nodeVector.push_back(RightCenter);
				Face * RD = new Face(c->RDNode, RightCenter);
				RD->setAttribute(c->RFace->attribute);
				RD->setEndCell(c->RFace->eCell);
				faceVector.push_back(RD);
				c->RFace->setChilds(RD, RD);
				c->RFace->setCenterNode(RightCenter);
				RightFace = ((Cell*)c->RFace->eCell)->LFace->Child1;
			}
			else {
				RightCenter = c->RFace->centerNode;
				RightFace = c->RFace->Child1;
			}
		}
		else {
			RightCenter = c->RUNode;
			RightFace = c->RFace;
		}

		if (norf_s == 1) {
			if (!c->DFace->isParent()) { //child 가없다면 쪼갠다.
				DownCenter = new Node(snode.first, snode.second);
				nodeVector.push_back(DownCenter);
				Face * DR = new Face(DownCenter, c->RDNode);
				DR->setAttribute(c->DFace->attribute);
				DR->setEndCell(c->DFace->eCell);
				faceVector.push_back(DR);
				c->DFace->setChilds(DR, DR);
				c->DFace->setCenterNode(DownCenter);
				DownFace = ((Cell*)c->DFace->eCell)->UFace->Child1;
			}
			else {
				DownCenter = c->DFace->centerNode;
				DownFace = c->DFace->Child2;
			}
		}
		else {
			DownCenter = c->LDNode;
			DownFace = c->DFace;

		}
		// 삼각형생성
		CellTriangle* tri = new CellTriangle(RightCenter, DownCenter, c->RDNode);
		minimalCellVector_tri.push_back(tri);
		RightFace->sCell = tri;
		DownFace->sCell = tri;
		wall.push_back(make_pair(make_pair(1, tri->id - 1), make_pair(RightCenter->id - 1, DownCenter->id - 1)));
		//추가 2017_03_15 재
		tri->HONode->inout = -2;
		tri->VONode->inout = -2;
		//추가 2017_07_31 재
		c->LFace->eCell = NULL;
		c->UFace->eCell = NULL;
		c->LFace->sCell = NULL;
		c->UFace->sCell = NULL;
		//추가 2017_07_31 재
		if (norf_f == 1)
			c->RFace->sCell = NULL;
		else
			c->RFace->sCell = tri;
		if (norf_s == 1)
			c->DFace->sCell = NULL;
		else
			c->DFace->sCell = tri;
	}
	else cout << "Error : eraseTRI()" << endl;

}

bool isshape(pdd fnode, pdd snode, int updown, double mincellsize) {//진_01_22 : 모양판별 함수 up = 1, down=-1
	Cell * c = (Cell *)mininodeCell[fnode][snode];
	if (c->getiscut() == true) {
		//이미쪼개진 사각형
		printf(" \n >>>>Please increase the depth of this programe <<<<< \n \t : There is two of line in one minicell ! \n ");
		return false;
	}
	else {
		if (c != NULL) { //직선이어서 null반환이 아닌경우
			vector<Node*> cnode;
			cnode.push_back(c->getLUNode());
			cnode.push_back(c->getRUNode());
			cnode.push_back(c->getLDNode());
			cnode.push_back(c->getRDNode());
			double gradient = (fnode.second - snode.second) / (fnode.first - snode.first);
			int wopoint = 0;
			int fpoint = 0;
			int ccwresult[3] = { 0,0,0 };
			for (int i = 0; i < 4; i++) {
				//cout << "node i : " << cnode[i]->x << " " << cnode[i]->y << endl;
				double r = ccw(fnode, make_pair(cnode[i]->x, cnode[i]->y), snode);
				//cout << "r : " << r << endl;
				if (r == 0) {
					ccwresult[1]++;
					fpoint += i;
				}
				else if (updown*r > 0) ccwresult[0]++;
				else {
					ccwresult[2]++;
					wopoint += i;
				}
			}
			//cout << "ccw result : " << ccwresult[0] << " " << ccwresult[1] << " " << ccwresult[2] << endl;
			if (ccwresult[0] == 1 && ccwresult[1] == 0 && ccwresult[2] == 3) { //오각형
				cuting(c, fnode, snode, mincellsize, updown, gradient);
				c->setiscut(true);
				//cout << " 오각형 " << endl;
				return true;
			}
			else if (ccwresult[2] == 2 && (ccwresult[0] + ccwresult[1] == 2)) {//사각형
				if (updown == 1) eraseRect(c, fnode, snode, wopoint);
				else eraseRect(c, snode, fnode, wopoint);
				c->setiscut(true);
				//cout << " 사각형 " << endl;
				return true;
			}
			else if (ccwresult[2] == 1 && (ccwresult[0] + ccwresult[1] == 3)) {//삼각형
				if (updown == 1) eraseTri(c, fnode, snode, wopoint);
				else eraseTri(c, snode, fnode, wopoint);
				c->setiscut(true);
				//cout << " 삼각형 " << endl;
				return true;
			}
			else if (ccwresult[2] == 0 && ccwresult[1] == 2 && ccwresult[0] == 2) {//직선
				if (fpoint == 1) {//uface
					if (updown == -1) {
						Face* wf = new Face(c->RUNode, c->LUNode);
						wf->setStartCell(c);
						wf->setEndCell(NULL);
						wf->setAttribute(c->UFace->attribute);
						faceVector.push_back(wf);
						c->UFace->Child1 = wf;
						c->UFace->Child2 = wf;
						//추가 수평 03_23
						c->RUNode->inout = -2;
						c->LUNode->inout = -2;
					}
					else {
						Face* wf = new Face(c->LUNode, c->RUNode);
						wf->setStartCell(c->UFace->sCell);
						wf->setEndCell(NULL);
						wf->setAttribute(c->UFace->attribute);
						faceVector.push_back(wf);
						c->UFace->Child1 = wf;
						c->UFace->Child2 = wf;
						//추가 수평 03_23
						c->RUNode->inout = -2;
						c->LUNode->inout = -2;
					}

					return true;
				}
				else if (fpoint == 5) {//dface
					if (updown == -1) {
						Face* wf = new Face(c->LDNode, c->RDNode);
						wf->setStartCell(c->DFace->eCell);
						wf->setEndCell(NULL);
						wf->setAttribute(c->UFace->attribute);
						faceVector.push_back(wf);
						c->DFace->Child1 = wf;
						c->DFace->Child2 = wf;
						//추가 수평 03_23
						c->RDNode->inout = -2;
						c->LDNode->inout = -2;
					}
					else {
						Face* wf = new Face(c->RDNode, c->LDNode);
						wf->setStartCell(c);
						wf->setEndCell(NULL);
						wf->setAttribute(c->UFace->attribute);
						faceVector.push_back(wf);
						c->DFace->Child1 = wf;
						c->DFace->Child2 = wf;
						//추가 수평 03_23
						c->RDNode->inout = -2;
						c->LDNode->inout = -2;
					}
					return true;
				}
				else {
					cout << "error: fpoint " << endl;
				}
			}
			else printf("error : ccwresult 판별 실패");
		}
		else {
			cout << "error: ccwresult null point " << endl;
		}
	}
	return false;
}
/*void checkInner(Cell* c, pdd p1, pdd p2) { // 재 : Cell의 Node들의 도형과의 상대적인 위치를 확인하고 face를 제거한다. 2017_02_05
// 재 : 도형을 이루는 선분들은 반시계방향으로 주어짐 2017_02_11
int flag;
if (c->LUNode->inout == 2 || c->LUNode->inout == -1) { // 재 : Node들 확인되지 않은 것만 확인 2017_02_05
flag = ccwAddition(p1, p2, make_pair(c->LUNode->getX(), c->LUNode->getY()));
if (flag == 1)
c->LUNode->inout = -1;
else if (flag == -1)
c->LUNode->inout = 1;
else
c->LUNode->inout = 0;
}
if (c->LDNode->inout == 2 || c->LDNode->inout == -1) {
flag = ccwAddition(p1, p2, make_pair(c->LDNode->getX(), c->LDNode->getY()));
if (flag == 1)
c->LDNode->inout = -1;
else if (flag == -1)
c->LDNode->inout = 1;
else
c->LDNode->inout = 0;
}
if (c->RUNode->inout == 2 || c->RUNode->inout == -1) {
flag = ccwAddition(p1, p2, make_pair(c->RUNode->getX(), c->RUNode->getY()));
if (flag == 1)
c->RUNode->inout = -1;
else if (flag == -1)
c->RUNode->inout = 1;
else
c->RUNode->inout = 0;
}
if (c->RDNode->inout == 2 || c->RDNode->inout == -1) {
flag = ccwAddition(p1, p2, make_pair(c->RDNode->getX(), c->RDNode->getY()));
if (flag == 1)
c->RDNode->inout = -1;
else if (flag == -1)
c->RDNode->inout = 1;
else
c->RDNode->inout = 0;
}
}*/

void checkInner(double LeftDownX, double LeftDownY, double initRCellsize, int Xcount, int Ycount, bool flag) {
	//nodeVector를 정렬, begin()은 좌측 최상부
	sort(nodeVector.begin(), nodeVector.end(), nodeCompareXY);
	vector<Node*>::iterator itTest, itTest2;
	/*
	for (itTest = nodeVector.begin(); itTest < --nodeVector.end(); itTest++) {
	itTest2 = itTest;
	itTest2++;
	if (fabs((*itTest2)->getY() - (*itTest)->getY()) < S_N)
	cout << "find";
	}*/
	vector <Node*>::iterator it, itT;
	double RightUpX = LeftDownX + (Xcount * initRCellsize);
	double RightUpY = LeftDownY + (Ycount * initRCellsize);
	int inout = 4;
	/* attribute
	-2: line (도형위)
	-1: inner (도형의 안쪽)
	0 : outflow  (맨 오른쪽)
	1 : inlet    (맨 왼쪽)
	2 : top      (맨 위)
	3 : bottom   (맨 아래)
	4 : interrior(중간 Node들)
	5 : unknown (미확인)
	*/
	itT = nodeVector.end();
	for (it = nodeVector.begin(); it < nodeVector.end(); it++) {
		// 가장자리 처리

		if (fabs((*it)->getY() - LeftDownY) < S_N) {
			(*it)->inout = 3; // bottom
		}
		else if (fabs((*it)->getY() - RightUpY) < S_N) {
			(*it)->inout = 2; // top
		}
		else if (fabs((*it)->getX() - LeftDownX) < S_N) {
			(*it)->inout = 1; // inlet
		}
		else if (fabs((*it)->getX() - RightUpX) < S_N) {
			(*it)->inout = 0; // outlet
		}

		/* initCell의 face 처리
		if (flag) {
		if ((*it)->getX() == airpoile.begin()->first && (*it)->getY() == airpoile.begin()->second)
		inout = -2;
		}*/

		if ((*it)->getInout() == 5)
			(*it)->inout = 4;
		else if ((*it)->getInout() == 3)
			itT = nodeVector.end();
		else if ((*it)->getInout() == -2) {
			if (itT == nodeVector.end())
				itT = it;
			else {
				itT++;
				for (itT; itT < nodeVector.end(); itT++) {
					if (itT == it)
						break;
					else {
						(*itT)->inout = -1;
					}
				}
				itT = nodeVector.end();
			}
		}

	}
}
void deleteInnerFace() {
	// 재 : face 제거 2017_02_05 // 경계선의 경우는 건드리지 않는다.

	/* test_jae_detface : fluent에서 #f error 발생
	vector<Face*>::iterator itf; // 재 : 수정 2017_02_19

	for (itf = faceVector.begin(); itf < faceVector.end(); itf++) {
	if ((*itf)->attribute == 4) {
	if ((*itf)->eCell != NULL && (*itf)->sCell != NULL) {
	if ((*itf)->eNode->inout == -1 && (*itf)->sNode->inout == -1 && (*itf)->eCell->getID() > initCellMaxID && (*itf)->sCell->getID() > initCellMaxID) {
	detfaceVector.push_back(make_pair((*itf)->getID(), (*itf)));
	//
	if ((*itf)->isChild()) {
	if ((*itf)->curParent->Child1 == (*itf))
	(*itf)->curParent->Child1 = NULL;
	if ((*itf)->curParent->Child2 == (*itf))
	(*itf)->curParent->Child2 = NULL;
	}//

	}
	if ((*itf)->eNode->inout == -1 && (*itf)->sNode->inout == 0 && (*itf)->eCell->getID() > initCellMaxID && (*itf)->sCell->getID() > initCellMaxID) {
	detfaceVector.push_back(make_pair((*itf)->getID(), (*itf)));
	//
	if ((*itf)->isChild()) {
	if ((*itf)->curParent->Child1 == (*itf))
	(*itf)->curParent->Child1 = NULL;
	if ((*itf)->curParent->Child2 == (*itf))
	(*itf)->curParent->Child2 = NULL;
	}//
	}
	if ((*itf)->eNode->inout == 0 && (*itf)->sNode->inout == -1 && (*itf)->eCell->getID() > initCellMaxID && (*itf)->sCell->getID() > initCellMaxID) {
	detfaceVector.push_back(make_pair((*itf)->getID(), (*itf)));
	//
	if ((*itf)->isChild()) {
	if ((*itf)->curParent->Child1 == (*itf))
	(*itf)->curParent->Child1 = NULL;
	if ((*itf)->curParent->Child2 == (*itf))
	(*itf)->curParent->Child2 = NULL;
	}//
	}
	}
	}
	}
	*/

	/* test_jae_0_0
	vector<Face*>::iterator itf; // 재 : 수정 2017_02_19
	for (itf = faceVector.begin(); itf < faceVector.end(); itf++) {
	if ((*itf)->eNode->inout == -1 && (*itf)->sNode->inout == -1) {
	(*itf)->eCell == NULL;
	(*itf)->sCell == NULL;
	}
	if ((*itf)->eNode->inout == -1 && (*itf)->sNode->inout == 0) {
	(*itf)->eCell == NULL;
	(*itf)->sCell == NULL;
	}
	if ((*itf)->eNode->inout == 0 && (*itf)->sNode->inout == -1) {
	(*itf)->eCell == NULL;
	(*itf)->sCell == NULL;
	}
	}*/

	vector<Cell*>::iterator itc;
	for (itc = cellVector.begin(); itc < cellVector.end(); itc++) {
		/*
		if ((*itc)->RUNode->inout == 0 || (*itc)->RUNode->inout == -1) { // 재 : 예비 2017_02_28
		if ((*itc)->LUNode->inout == 0 || (*itc)->LUNode->inout == -1) {
		if ((*itc)->LDNode->inout == 0 || (*itc)->LDNode->inout == -1) {
		if ((*itc)->RDNode->inout == 0 || (*itc)->RDNode->inout == -1) {
		// 재 : initCell은 제거해선 안되는 것으로 보임 2017_02_19
		if ((*itc)->getID() > initCellMaxID)
		detcellVector.push_back(make_pair((*itc)->getID(), (*itc)));
		}
		}
		}
		}
		*
		if ((*itc)->RUNode->inout != 1 && (*itc)->LUNode->inout != 1 && (*itc)->LDNode->inout != 1 && (*itc)->RDNode->inout != 1) {
		if ((*itc)->RUNode->inout == 2 || (*itc)->LUNode->inout == 2 || (*itc)->LDNode->inout == 2 || (*itc)->RDNode->inout == 2)
		cout << "error_checkInner_inout_2\n";
		if ((*itc)->getID() > initCellMaxID)
		detcellVector.push_back(make_pair((*itc)->getID(), (*itc)));
		}*/
		/* attribute
		-2: line (도형위)
		-1: inner (도형의 안쪽)
		0 : outflow  (맨 오른쪽)
		1 : inlet    (맨 왼쪽)
		2 : top      (맨 위)
		3 : bottom   (맨 아래)
		4 : interrior(중간 Node들, 도형의 바깥)
		5 : unknown (미확인)
		*/
		if (((*itc)->RUNode->inout == -2 || (*itc)->RUNode->inout == -1) && ((*itc)->RDNode->inout == -2 || (*itc)->RDNode->inout == -1) && ((*itc)->LUNode->inout == -2 || (*itc)->LUNode->inout == -1) && ((*itc)->LDNode->inout == -2 || (*itc)->LDNode->inout == -1)) {
			//if ( (*itc)->RUNode->inout == -1 &&(*itc)->RDNode->inout == -1 && (*itc)->LUNode->inout == -1 &&  (*itc)->LDNode->inout == -1) {
			if ((*itc)->RUNode->inout == 5 || (*itc)->LUNode->inout == 5 || (*itc)->LDNode->inout == 5 || (*itc)->RDNode->inout == 5)
				cout << "error_checkInner_inout_5\n";
			if ((*itc)->getID() > initCellMaxID) {
				detcellVector.push_back(make_pair((*itc)->getID(), (*itc)));
				(*itc)->LFace->eCell = NULL;
				(*itc)->UFace->eCell = NULL;
				(*itc)->RFace->sCell = NULL;
				(*itc)->DFace->sCell = NULL;

			}
			//else                detinitcellVector.push_back((*itc)->getID());
		}
	}
}

/*
void cutInnerCell() {
// 재 : 모든 Cell에 대한 도형 안의 face 제거 작업, 경계선에 대한 cut-Cell 작업을 먼저 실행 시킨 후 동작되어야함 2017_02_08
vector<Cell*>::iterator it;
vector<pair<pdd, pdd> >::iterator itPdd;
for (it = cellVector.begin(); it < cellVector.end(); it++) {
for (itPdd = inputLine.begin(); itPdd < inputLine.end(); itPdd++) {
checkInner(*it, itPdd->first, itPdd->second);
}
}
deleteInnerFace();
}*/
void tailcut_rect(Cell* c, pdd inode, pdd cnode, bool updown, int type) {// 진 : tail 설정  2017-03-07
																		 /*
																		 type 1 = Lface , type 2 = Uface , type 3 = Dface
																		 */

	c->LUCell = new Cell();    ((Cell*)c->LUCell)->setDepth(c->depth + 1);
	c->LDCell = new Cell();    ((Cell*)c->LDCell)->setDepth(c->depth + 1);
	c->RDCell = new Cell();    ((Cell*)c->RDCell)->setDepth(c->depth + 1);
	c->RUCell = new Cell();    ((Cell*)c->RUCell)->setDepth(c->depth + 1);
	cellVector.push_back(((Cell*)c->LUCell));    cellVector.push_back(((Cell*)c->LDCell));
	cellVector.push_back(((Cell*)c->RDCell));    cellVector.push_back(((Cell*)c->RUCell));

	// 새로 만들어지는 Node를 먼저 생성한다.
	//0 : left , 1: down 2: right, 3: up 4: center
	Node *LeftCenter, *DownCenter, *RightCenter, *UpCenter, *cellCenter;
	if (!c->LFace->isParent()) {
		if (type == 1) LeftCenter = new Node(inode.first, inode.second);
		else LeftCenter = new Node(c->LDNode->x, cnode.second);
		nodeVector.push_back(LeftCenter);
	}
	else LeftCenter = c->LFace->centerNode;

	if (!c->DFace->isParent()) {
		if (type == 3) DownCenter = new Node(inode.first, inode.second);
		else DownCenter = new Node(cnode.first, c->LDNode->y);
		nodeVector.push_back(DownCenter);
	}
	else DownCenter = c->DFace->centerNode;

	if (!c->RFace->isParent()) {
		RightCenter = new Node(c->RDNode->x, cnode.second);
		nodeVector.push_back(RightCenter);
	}
	else RightCenter = c->RFace->centerNode;

	if (!c->UFace->isParent()) {
		if (type == 2) UpCenter = new Node(inode.first, inode.second);
		else UpCenter = new Node(cnode.first, c->LUNode->y);
		nodeVector.push_back(UpCenter);
	}
	else UpCenter = c->UFace->centerNode;

	cellCenter = new Node(cnode.first, cnode.second);
	nodeVector.push_back(cellCenter);

	// 내부 Cell의 Node를 세팅한다.
	c->setChildNodes(LeftCenter, DownCenter, RightCenter, UpCenter, cellCenter, 0);

	//없어질 child 사각형을 정한다
	int deletecell_num = 0;
	if ((updown && type == 1) || (!updown && type == 3)) {//LDcell삭제
		deletecell_num = 1;
	}
	else if ((!updown&& type == 1) || (updown && type == 2)) {//LUcell삭제
		deletecell_num = 2;
	}
	else if (!updown && type == 2) {//RUcell삭제
		deletecell_num = 3;
	}
	else if (updown && type == 3) {//RDcell삭제
		deletecell_num = 4;
	}

	// 내부 Face 12개를 만들겁니다.
	Face *LU = 0, *LD = 0, *DL = 0, *DR = 0, *RD = 0, *RU = 0;
	Face *UR = 0, *UL = 0, *IL = 0, *ID = 0, *IR = 0, *IU = 0;

	// Cell 만들 때 새로 만들어지는 Face중 외부 8개 먼저 셋팅
	// LD LU

	if (!c->LFace->isParent()) {
		LD = new Face(c->LDNode, LeftCenter);    faceVector.push_back(LD);
		LU = new Face(LeftCenter, c->LUNode);    faceVector.push_back(LU);
		if (deletecell_num == 1) {
			detfaceVector.push_back(make_pair(LD->id, LD));
			LD = LU;
		}
		else if (deletecell_num == 2) {
			detfaceVector.push_back(make_pair(LU->id, LU));
			LU = LD;
		}
		LD->setAttribute(c->LFace->attribute);
		LU->setAttribute(c->LFace->attribute);
		c->LFace->setChilds(LD, LU);
		c->LFace->setCenterNode(LeftCenter);
	}
	else {
		LD = c->LFace->Child1;
		LU = c->LFace->Child2;
	}

	// DL DR
	if (!c->DFace->isParent()) {
		DL = new Face(c->LDNode, DownCenter);    faceVector.push_back(DL);
		DR = new Face(DownCenter, c->RDNode);    faceVector.push_back(DR);
		if (deletecell_num == 1) {
			detfaceVector.push_back(make_pair(DL->id, DL));
			DL = DR;
		}
		else if (deletecell_num == 4) {
			detfaceVector.push_back(make_pair(DR->id, DR));
			DR = DL;
		}
		DL->setAttribute(c->DFace->attribute);
		DR->setAttribute(c->DFace->attribute);
		c->DFace->setChilds(DL, DR);
		c->DFace->setCenterNode(DownCenter);
	}
	else {
		DL = c->DFace->Child1;
		DR = c->DFace->Child2;
	}

	// RD RU
	if (!c->RFace->isParent()) {
		RD = new Face(c->RDNode, RightCenter);    faceVector.push_back(RD);
		RU = new Face(RightCenter, c->RUNode);    faceVector.push_back(RU);
		if (deletecell_num == 3) {
			detfaceVector.push_back(make_pair(RU->id, RU));
			RU = RD;
		}
		else if (deletecell_num == 4) {
			detfaceVector.push_back(make_pair(RD->id, RD));
			RD = RU;
		}
		RD->setAttribute(c->RFace->attribute);
		RU->setAttribute(c->RFace->attribute);
		c->RFace->setChilds(RD, RU);
		c->RFace->setCenterNode(RightCenter);
	}
	else {
		RD = c->RFace->Child1;
		RU = c->RFace->Child2;
	}

	// UL UR
	if (!c->UFace->isParent()) {
		UL = new Face(c->LUNode, UpCenter);    faceVector.push_back(UL);
		UR = new Face(UpCenter, c->RUNode);    faceVector.push_back(UR);
		if (deletecell_num == 2) {
			detfaceVector.push_back(make_pair(UL->id, UL));
			UL = UR;
		}
		else if (deletecell_num == 3) {
			detfaceVector.push_back(make_pair(UR->id, UR));
			UR = UL;
		}
		UL->setAttribute(c->UFace->attribute);
		UR->setAttribute(c->UFace->attribute);
		c->UFace->setChilds(UL, UR);
		c->UFace->setCenterNode(UpCenter);
	}
	else {
		UL = c->UFace->Child1;
		UR = c->UFace->Child2;
	}


	IL = new Face(LeftCenter, cellCenter);    IL->setAttribute(4);    IL->setDepth(UL->depth);    faceVector.push_back(IL);
	ID = new Face(DownCenter, cellCenter);    ID->setAttribute(4);    ID->setDepth(UL->depth);    faceVector.push_back(ID);
	IR = new Face(cellCenter, RightCenter);    IR->setAttribute(4);    IR->setDepth(UL->depth);    faceVector.push_back(IR);
	IU = new Face(cellCenter, UpCenter);    IU->setAttribute(4);    IU->setDepth(UL->depth);    faceVector.push_back(IU);

	// 각 내부 cell이 face를 가리키도록
	((Cell*)c->LUCell)->LFace = LU;    ((Cell*)c->LUCell)->DFace = IL;    ((Cell*)c->LUCell)->RFace = IU;    ((Cell*)c->LUCell)->UFace = UL;
	((Cell*)c->LDCell)->LFace = LD;    ((Cell*)c->LDCell)->DFace = DL;    ((Cell*)c->LDCell)->RFace = ID;    ((Cell*)c->LDCell)->UFace = IL;
	((Cell*)c->RDCell)->LFace = ID;    ((Cell*)c->RDCell)->DFace = DR;    ((Cell*)c->RDCell)->RFace = RD;    ((Cell*)c->RDCell)->UFace = IR;
	((Cell*)c->RUCell)->LFace = IU;    ((Cell*)c->RUCell)->DFace = IR;    ((Cell*)c->RUCell)->RFace = RU;    ((Cell*)c->RUCell)->UFace = UR;

	// 각 Face의 startCell / endCell 셋팅 // 재 : Face의 sCell과 eCell도 CellShape로 변경하였기 때문에 밑에 코드도 약간 수정했습니다.  2017-01-12

	// LU, LD
	if (updown) LU->setEndCell(((Cell*)c->LUCell));
	else LD->setEndCell(((Cell*)c->LDCell));
	if (c->LFace->attribute != 1) {
		if (deletecell_num != 2 && LU->getStartCell() == NULL)LU->setStartCell(c->LFace->sCell);
		if (deletecell_num != 1 && LD->getStartCell() == NULL)LD->setStartCell(c->LFace->sCell);
	}
	// DL, DR
	if (!updown) DL->setStartCell(((Cell*)c->LDCell));
	DR->setStartCell(((Cell*)c->RDCell));
	if (c->DFace->attribute != 3) {
		if (deletecell_num != 1 && DL->getEndCell() == NULL)DL->setEndCell(c->DFace->eCell);
		if (deletecell_num != 4 && DR->getEndCell() == NULL)DR->setEndCell(c->DFace->eCell);
	}
	// RD, RU
	RD->setStartCell(((Cell*)c->RDCell));
	RU->setStartCell(((Cell*)c->RUCell));
	if (c->RFace->attribute != 0) {
		if (deletecell_num != 4 && RD->getEndCell() == NULL)RD->setEndCell(c->RFace->eCell);
		if (deletecell_num != 3 && RU->getEndCell() == NULL)RU->setEndCell(c->RFace->eCell);
	}
	// UR, UL
	UR->setEndCell(((Cell*)c->RUCell));
	if (updown) UL->setEndCell(((Cell*)c->LUCell));
	if (c->UFace->attribute != 2) {
		if (deletecell_num != 3 && UR->getStartCell() == NULL)UR->setStartCell(c->UFace->sCell);
		if (deletecell_num != 2 && UL->getStartCell() == NULL)UL->setStartCell(c->UFace->sCell);
	}
	// INNER Faces
	IL->setStartCell(((Cell*)c->LUCell));    IL->setEndCell(((Cell*)c->LDCell));
	ID->setStartCell(((Cell*)c->LDCell));    ID->setEndCell(((Cell*)c->RDCell));
	IR->setStartCell(((Cell*)c->RUCell));    IR->setEndCell(((Cell*)c->RDCell));
	IU->setStartCell(((Cell*)c->LUCell));    IU->setEndCell(((Cell*)c->RUCell));

	if (deletecell_num == 1) {
		detcellVector.push_back(make_pair(c->LDCell->id, (Cell*)c->LDCell));
		wall.push_back(make_pair(make_pair(0, c->LUCell->id - 1), make_pair(LeftCenter->id - 1, cellCenter->id - 1)));
		wall.push_back(make_pair(make_pair(0, c->RDCell->id - 1), make_pair(cellCenter->id - 1, DownCenter->id - 1)));
	}
	else if (deletecell_num == 2) {
		detcellVector.push_back(make_pair(c->LUCell->id, (Cell*)c->LUCell));
		wall.push_back(make_pair(make_pair(0, c->LDCell->id - 1), make_pair(cellCenter->id - 1, LeftCenter->id - 1)));
		wall.push_back(make_pair(make_pair(0, c->RUCell->id - 1), make_pair(UpCenter->id - 1, cellCenter->id - 1)));
	}
	else if (deletecell_num == 3) {
		detcellVector.push_back(make_pair(c->RUCell->id, (Cell*)c->RUCell));
		wall.push_back(make_pair(make_pair(0, c->RDCell->id - 1), make_pair(RightCenter->id - 1, cellCenter->id - 1)));
		wall.push_back(make_pair(make_pair(0, c->LUCell->id - 1), make_pair(cellCenter->id - 1, UpCenter->id - 1)));
	}
	else if (deletecell_num == 4) {
		detcellVector.push_back(make_pair(c->RDCell->id, (Cell*)c->RDCell));
		wall.push_back(make_pair(make_pair(0, c->LDCell->id - 1), make_pair(DownCenter->id - 1, cellCenter->id - 1)));
		wall.push_back(make_pair(make_pair(0, c->RUCell->id - 1), make_pair(cellCenter->id - 1, RightCenter->id - 1)));
	}
	else {
		printf("Error : tail_cut_rect - deletenum : %d \n", deletecell_num);
	}
}
void tailcut_tri(Cell* c, pdd inode, pdd cnode, bool updown, int type) {// 진 : tail 설정  2017-03-07
																		/*
																		type 1 = RU ,RDCell을 생성하는 경우
																		type 2 = LD, RDCell을 생성하는 경우
																		type 3 = LU, RUCell을 생성하는 경우
																		type 4 = 1과 동일 하지만 이미 쪼개져있음(UFace나 Dface가)
																		*/

	if (type == 3) {
		c->LUCell = new Cell();    ((Cell*)c->LUCell)->setDepth(c->depth + 1); cellVector.push_back(((Cell*)c->LUCell));
	}
	if (type == 2) {
		c->LDCell = new Cell();    ((Cell*)c->LDCell)->setDepth(c->depth + 1); cellVector.push_back(((Cell*)c->LDCell));
	}
	if (type != 3) {
		c->RDCell = new Cell();    ((Cell*)c->RDCell)->setDepth(c->depth + 1); cellVector.push_back(((Cell*)c->RDCell));
	}
	if (type != 2) {
		c->RUCell = new Cell();    ((Cell*)c->RUCell)->setDepth(c->depth + 1); cellVector.push_back(((Cell*)c->RUCell));
	}

	// 새로 만들어지는 Node를 먼저 생성한다.
	//0 : left , 1: down 2: right, 3: up 4: center
	Node *LeftCenter = NULL, *DownCenter = NULL, *RightCenter = NULL, *UpCenter = NULL, *cellCenter = NULL;
	if (type == 2 || type == 3) {
		if (!c->LFace->isParent()) {
			LeftCenter = new Node(c->LDNode->x, cnode.second);
			nodeVector.push_back(LeftCenter);
		}
		else LeftCenter = c->LFace->centerNode;
	}
	if (type != 3) {
		if (!c->DFace->isParent()) {
			DownCenter = new Node(cnode.first, c->LDNode->y);
			nodeVector.push_back(DownCenter);
		}
		else DownCenter = c->DFace->centerNode;
	}

	if (!c->RFace->isParent()) {
		RightCenter = new Node(c->RDNode->x, cnode.second);
		nodeVector.push_back(RightCenter);
	}
	else RightCenter = c->RFace->centerNode;

	if (type != 2) {
		if (!c->UFace->isParent()) {
			UpCenter = new Node(cnode.first, c->LUNode->y);
			nodeVector.push_back(UpCenter);
		}
		else UpCenter = c->UFace->centerNode;
	}

	cellCenter = new Node(cnode.first, cnode.second);
	nodeVector.push_back(cellCenter);

	// 내부 Cell의 Node를 세팅한다.
	c->setChildNodes(LeftCenter, DownCenter, RightCenter, UpCenter, cellCenter, type);

	// 내부 Face 12개를 만들겁니다.
	Face *LU = 0, *LD = 0, *DL = 0, *DR = 0, *RD = 0, *RU = 0;
	Face *UR = 0, *UL = 0, *IL = 0, *ID = 0, *IR = 0, *IU = 0;

	// Cell 만들 때 새로 만들어지는 Face중 외부 8개 먼저 셋팅
	// LD LU

	if (type != 1 && type != 4) {
		if (!c->LFace->isParent()) {
			LD = new Face(c->LDNode, LeftCenter);    faceVector.push_back(LD);
			LU = new Face(LeftCenter, c->LUNode);    faceVector.push_back(LU);
			if (type == 3 && !updown) {
				detfaceVector.push_back(make_pair(LD->id, LD));
				LD = LU;
			}
			else if (type == 2 && updown) {
				detfaceVector.push_back(make_pair(LU->id, LU));
				LU = LD;
			}
			LD->setAttribute(c->LFace->attribute);
			LU->setAttribute(c->LFace->attribute);
			c->LFace->setChilds(LD, LU);
			c->LFace->setCenterNode(LeftCenter);
		}
		else {
			LD = c->LFace->Child1;
			LU = c->LFace->Child2;
		}
	}

	// DL DR
	if (type != 3) {
		if (!c->DFace->isParent()) {
			DL = new Face(c->LDNode, DownCenter);    faceVector.push_back(DL);
			DR = new Face(DownCenter, c->RDNode);    faceVector.push_back(DR);
			if (type == 1 && updown) {
				detfaceVector.push_back(make_pair(DL->id, DL));
				DL = DR;
			}
			DL->setAttribute(c->DFace->attribute);
			DR->setAttribute(c->DFace->attribute);
			c->DFace->setChilds(DL, DR);
			c->DFace->setCenterNode(DownCenter);
		}
		else {
			DL = c->DFace->Child1;
			DR = c->DFace->Child2;
		}
	}
	// RD RU
	if (!c->RFace->isParent()) {
		RD = new Face(c->RDNode, RightCenter);    faceVector.push_back(RD);
		RU = new Face(RightCenter, c->RUNode);    faceVector.push_back(RU);
		if (type == 2 && !updown) {
			detfaceVector.push_back(make_pair(RU->id, RU));
			RU = RD;
		}
		else if (type == 3 && updown) {
			detfaceVector.push_back(make_pair(RD->id, RD));
			RD = RU;
		}
		RD->setAttribute(c->RFace->attribute);
		RU->setAttribute(c->RFace->attribute);
		c->RFace->setChilds(RD, RU);
		c->RFace->setCenterNode(RightCenter);
	}
	else {
		RD = c->RFace->Child1;
		RU = c->RFace->Child2;
	}

	// UL UR
	if (type != 2) {
		if (!c->UFace->isParent()) {
			UL = new Face(c->LUNode, UpCenter);    faceVector.push_back(UL);
			UR = new Face(UpCenter, c->RUNode);    faceVector.push_back(UR);
			if (type == 1 && !updown) {
				detfaceVector.push_back(make_pair(UL->id, UL));
				UL = UR;
			}
			UL->setAttribute(c->UFace->attribute);
			UR->setAttribute(c->UFace->attribute);
			c->UFace->setChilds(UL, UR);
			c->UFace->setCenterNode(UpCenter);
		}
		else {
			UL = c->UFace->Child1;
			UR = c->UFace->Child2;
		}
	}


	if (type != 1 && type != 4) { IL = new Face(LeftCenter, cellCenter);    IL->setAttribute(4);    IL->setDepth(RD->depth);    faceVector.push_back(IL); }
	if (type != 3) { ID = new Face(DownCenter, cellCenter);    ID->setAttribute(4);    ID->setDepth(RD->depth);    faceVector.push_back(ID); }
	IR = new Face(cellCenter, RightCenter);    IR->setAttribute(4);    IR->setDepth(RD->depth);    faceVector.push_back(IR);
	if (type != 2) { IU = new Face(cellCenter, UpCenter);    IU->setAttribute(4);    IU->setDepth(RD->depth);    faceVector.push_back(IU); }



	// 각 내부 cell이 face를 가리키도록
	if (type == 3) { ((Cell*)c->LUCell)->LFace = LU;    ((Cell*)c->LUCell)->DFace = IL;    ((Cell*)c->LUCell)->RFace = IU;    ((Cell*)c->LUCell)->UFace = UL; }
	if (type == 2) { ((Cell*)c->LDCell)->LFace = LD;    ((Cell*)c->LDCell)->DFace = DL;    ((Cell*)c->LDCell)->RFace = ID;    ((Cell*)c->LDCell)->UFace = IL; }
	if (type != 3) { ((Cell*)c->RDCell)->LFace = ID;    ((Cell*)c->RDCell)->DFace = DR;    ((Cell*)c->RDCell)->RFace = RD;    ((Cell*)c->RDCell)->UFace = IR; }
	if (type != 2) { ((Cell*)c->RUCell)->LFace = IU;    ((Cell*)c->RUCell)->DFace = IR;    ((Cell*)c->RUCell)->RFace = RU;    ((Cell*)c->RUCell)->UFace = UR; }

	// 각 Face의 startCell / endCell 셋팅 // 재 : Face의 sCell과 eCell도 CellShape로 변경하였기 때문에 밑에 코드도 약간 수정했습니다.  2017-01-12

	// LU, LD
	if (type != 1 && type != 4) {
		if (type == 3 || !updown) LU->setEndCell(((Cell*)c->LUCell));
		if (type == 2 || updown) LD->setEndCell(((Cell*)c->LDCell));
		if (c->LFace->attribute != 1) {
			if ((type == 3 || !updown) && LU->getStartCell() == NULL)LU->setStartCell(c->LFace->sCell);
			if ((type == 2 || updown > 0) && LD->getStartCell() == NULL)LD->setStartCell(c->LFace->sCell);
		}
	}
	// DL, DR
	if (type != 3) {
		if (type != 4 && (type == 2 || !updown)) DL->setStartCell(((Cell*)c->LDCell));
		DR->setStartCell(((Cell*)c->RDCell));
		if (c->DFace->attribute != 3) {
			if ((type != 4 && (type == 2 || !updown)) && DL->getEndCell() == NULL)DL->setEndCell(c->DFace->eCell);
			if (DR->getEndCell() == NULL)DR->setEndCell(c->DFace->eCell);
		}
	}
	// RD, RU
	if (!(type == 3 && updown)) RD->setStartCell(((Cell*)c->RDCell));
	if (!(type == 2 && !updown)) RU->setStartCell(((Cell*)c->RUCell));
	if (c->RFace->attribute != 0) {
		if (!(type == 3 && updown) && RD->getEndCell() == NULL)RD->setEndCell(c->RFace->eCell);
		if (!(type == 3 && updown) && RU->getEndCell() == NULL)RU->setEndCell(c->RFace->eCell);
	}
	// UR, UL
	if (type != 2) {
		UR->setEndCell(((Cell*)c->RUCell));
		if (type != 4 && (type == 2 || updown)) UL->setEndCell(((Cell*)c->LUCell));
		if (c->UFace->attribute != 2) {
			if (UR->getStartCell() == NULL)UR->setStartCell(c->UFace->sCell);
			if ((type != 4 && (type == 2 || updown)) && UL->getStartCell() == NULL)UL->setStartCell(c->UFace->sCell);
		}
	}

	// INNER Faces
	if (type == 3) {
		IL->setStartCell(((Cell*)c->LUCell));
		IU->setStartCell(((Cell*)c->LUCell));
	}
	if (type == 2) {
		IL->setEndCell(((Cell*)c->LDCell));
		ID->setStartCell(((Cell*)c->LDCell));
	}
	if (type != 3) {
		ID->setEndCell(((Cell*)c->RDCell));
		IR->setEndCell(((Cell*)c->RDCell));
	}
	if (type != 2) {
		IR->setStartCell(((Cell*)c->RUCell));
		IU->setEndCell(((Cell*)c->RUCell));
	}
	if (type == 1) {
		if (updown) {
			//삼각형생성
			c->LUCell = new CellTriangle(cellCenter, c->LUNode, UpCenter);
			c->UFace->Child1->setEndCell(c->LUCell);
			IU->setStartCell(c->LUCell);
			minimalCellVector_tri.push_back(c->LUCell);
			wall.push_back(make_pair(make_pair(1, c->LUCell->id - 1), make_pair(c->LUNode->id - 1, cellCenter->id - 1)));
			wall.push_back(make_pair(make_pair(0, c->RDCell->id - 1), make_pair(cellCenter->id - 1, DownCenter->id - 1)));
			detfaceVector.push_back(make_pair(ID->id, ID));

		}
		else {
			c->LDCell = new CellTriangle(cellCenter, c->LDNode, DownCenter);
			c->DFace->Child1->setStartCell(c->LDCell);
			ID->setStartCell(c->LDCell);
			minimalCellVector_tri.push_back(c->LDCell);
			wall.push_back(make_pair(make_pair(1, c->LDCell->id - 1), make_pair(cellCenter->id - 1, c->LDNode->id - 1)));
			wall.push_back(make_pair(make_pair(0, c->RUCell->id - 1), make_pair(UpCenter->id - 1, cellCenter->id - 1)));
			detfaceVector.push_back(make_pair(IU->id, IU));
		}
		c->LFace->Child1 = ID;
		c->LFace->Child2 = IU;
		c->LFace->centerNode = cellCenter;
	}
	else if (type == 2) {
		if (!updown) {
			//삼각형생성
			c->LUCell = new CellTriangle(c->LUNode, cellCenter, LeftCenter);
			c->LFace->Child2->setEndCell(c->LUCell);
			IL->setStartCell(c->LUCell);
			minimalCellVector_tri.push_back(c->LUCell);
			wall.push_back(make_pair(make_pair(1, c->LUCell->id - 1), make_pair(cellCenter->id - 1, c->LUNode->id - 1)));
			wall.push_back(make_pair(make_pair(0, c->RDCell->id - 1), make_pair(RightCenter->id - 1, cellCenter->id - 1)));
			detfaceVector.push_back(make_pair(IR->id, IR));
		}
		else {
			c->RUCell = new CellTriangle(c->RUNode, cellCenter, RightCenter);
			c->RFace->Child2->setStartCell(c->RUCell);
			IR->setStartCell(c->RUCell);
			minimalCellVector_tri.push_back(c->RUCell);
			wall.push_back(make_pair(make_pair(1, c->RUCell->id - 1), make_pair(c->RUNode->id - 1, cellCenter->id - 1)));
			wall.push_back(make_pair(make_pair(0, c->LDCell->id - 1), make_pair(cellCenter->id - 1, LeftCenter->id - 1)));
			detfaceVector.push_back(make_pair(IL->id, IL));
		}
		c->UFace->Child1 = IL;
		c->UFace->Child2 = IR;
		c->UFace->centerNode = cellCenter;
	}
	else if (type == 3) {
		if (updown) {
			//삼각형생성
			c->LDCell = new CellTriangle(c->LDNode, cellCenter, LeftCenter);
			c->LFace->Child1->setEndCell(c->LDCell);
			IL->setEndCell(c->LDCell);
			minimalCellVector_tri.push_back(c->LDCell);
			wall.push_back(make_pair(make_pair(1, c->LDCell->id - 1), make_pair(c->LDNode->id - 1, cellCenter->id - 1)));
			wall.push_back(make_pair(make_pair(0, c->RUCell->id - 1), make_pair(cellCenter->id - 1, RightCenter->id - 1)));
			detfaceVector.push_back(make_pair(IR->id, IR));
		}
		else {
			c->RDCell = new CellTriangle(c->RDNode, cellCenter, RightCenter);
			c->RFace->Child1->setStartCell(c->RDCell);
			IR->setEndCell(c->RDCell);
			minimalCellVector_tri.push_back(c->RDCell);
			wall.push_back(make_pair(make_pair(1, c->RDCell->id - 1), make_pair(c->RDNode->id - 1, cellCenter->id - 1)));
			wall.push_back(make_pair(make_pair(0, c->LUCell->id - 1), make_pair(cellCenter->id - 1, LeftCenter->id - 1)));
			detfaceVector.push_back(make_pair(IL->id, IL));
		}
		c->DFace->Child1 = IL;
		c->DFace->Child2 = IR;
		c->DFace->centerNode = cellCenter;
	}
	else if (type == 4) {
		c->LFace->Child1 = ID;
		c->LFace->Child2 = IU;
		c->LFace->centerNode = cellCenter;
		//wall 처리할것이 없음
	}
	else {
		printf("Error : tail_cut_tri_ num : %d \n", type);
	}
}

void tailmanager(pdd startnode, pdd endnode, double mincellsize, int layer, int depth) {// 진 : tail 설정  2017-03-07
	pdd inode1 = mininode[0][mininode[0].size() - 1];
	pdd inode2 = mininode[1][mininode[1].size() - 1];

	if (inode1 != startnode) {
		pair<int, void*> temp = mininodePlace[inode1];
		if (temp.first == 0) {//node
			node_of_cell cs = ((Node*)temp.second)->findtailCell(startnode);
			if (cs.first != 0) {
				if (startnode.first == endnode.first) tailcut_tri((Cell*)cs.second, inode1, startnode, true, 1);
				else if (cs.first % 2) tailcut_tri((Cell*)cs.second, inode1, startnode, true, 2);
				else tailcut_tri((Cell*)cs.second, inode1, startnode, true, 3);
			}
		}
		else {//face
			Face *f = (Face*)mininodePlace[inode1].second;
			Cell * c = (Cell*)f->eCell;
			if (f == c->LFace) tailcut_rect(c, inode1, startnode, true, 1);
			else if (f == c->UFace) {
				if (startnode.second == endnode.second) tailcut_rect(c, inode1, startnode, true, 2); // 에어포일이 세워진 상태
				else tailcut_tri(c, inode1, startnode, true, 4);
			}
			else printf("error tail!\n");
		}
	}
	if (inode2 != endnode) {
		pair<int, void*> temp = mininodePlace[inode2];
		if (temp.first == 0) {//node
			node_of_cell cs = ((Node*)temp.second)->findtailCell(endnode);
			if (cs.first != 0) {
				if (startnode.first == endnode.first) tailcut_tri((Cell*)cs.second, inode2, endnode, false, 1);
				else if (cs.first % 2) tailcut_tri((Cell*)cs.second, inode2, endnode, false, 2);
				else tailcut_tri((Cell*)cs.second, inode2, endnode, false, 3);
			}
		}
		else {//face
			Face *f = (Face*)mininodePlace[inode2].second;
			Cell * c = (Cell*)f->eCell;
			/*
			if (f->getStartCell() == NULL) c = (Cell*)f->eCell;
			else c = (Cell*)f->sCell;*/
			if (f == c->LFace) tailcut_rect(c, inode2, endnode, false, 1);
			else if (f == c->DFace) {
				if (startnode.second == endnode.second) tailcut_rect(c, inode2, endnode, false, 3); // 에어포일이 세워진 상태
				else tailcut_tri(c, inode2, endnode, false, 4);
			}
			else printf("error tail!\n");
		}
	}

	//수직부분 처리
	vector<Node*> tail_node;
	vector<pdi> result;
	for (int i = 0; i < nodeVector.size(); i++) {
		if (nodeVector[i]->x == startnode.first)     result.push_back(make_pair(nodeVector[i]->y, i));
	}

	for (int i = findn(endnode.second, mincellsize, 0); i <= findn(startnode.second, mincellsize, 1); i++) {
		double y = i * mincellsize;
		if (y != endnode.second && y != startnode.second) {
			bool exist = false;
			for (int k = 0; k < result.size(); k++) {
				if (result[k].first == y) {
					tail_node.push_back(nodeVector[result[k].second]);
					initRectangle->dotPoint(startnode.first, y, layer, depth);
					exist = true;
					// 추가 03_25
					nodeVector[result[k].second]->inout = -2;
				}
			}
			if (!exist) {
				Node * n = new Node(startnode.first, y);
				nodeVector.push_back(n);
				initRectangle->dotPoint(n->x, n->y, layer, depth);
				tail_node.push_back(n);
				// 추가 03_25
				n->inout = -2;
			}
		}

	}
	if (inode1 == startnode && inode2 == endnode && inode1.first == inode2.first) {//수직
		pair<int, void*> temp = mininodePlace[inode1];
		if (temp.first == 1) {
			Face * f = (Face *)temp.second;
			if (f->isParent()) {
				//wall.push_back(make_pair(make_pair(0, f->eCell->id - 1), make_pair(f->centerNode->id - 1, f->sNode->id - 1)));
				Face *nf = new Face(f->centerNode, f->sNode);
				nf->setAttribute(f->attribute);
				nf->setDepth(f->depth + 1);
				nf->sCell = f->eCell;
				f->Child1 = nf;
				faceVector.push_back(nf);
			}
		}
		temp = mininodePlace[inode2];
		if (temp.first == 1) {
			Face * f = (Face *)temp.second;
			if (f->isParent()) {
				Face *nf = new Face(f->eNode, f->centerNode);
				nf->setAttribute(f->attribute);
				nf->setDepth(f->depth + 1);
				nf->sCell = f->eCell;
				f->Child2 = nf;
				faceVector.push_back(nf);
			}
		}

	}
	else {
		for (int i = 1; i < tail_node.size(); i++) {
			pdd prev = make_pair(tail_node[i - 1]->x, tail_node[i - 1]->y);
			pdd nxt = make_pair(tail_node[i]->x, tail_node[i]->y);
			setupmininodeCell(prev, nxt);
			Cell * c = (Cell*)mininodeCell[prev][nxt];
			eraseRect(c, nxt, prev, 4);
		}
	}
}
bool cmp_vertical(const pdd &a, const pdd &b) {
	if (a.second == b.second) return a.first < b.first;
	return a.second < b.second;
}
int main() {
	double LeftDownX = -3, LeftDownY = -3;
	int layer = 3, depth = 7; //진
	double mincellsize = 0; //진
	int Xcount = 8, Ycount = 8;//진_initcellsize설정
	double initRCellsize = 1;//진_initcellsize설정
	FILE * inputfp;
	int n, uppern, lowern, tail;
	double x, y;
	vector<pdd> airpoile;
	bool vertical_horizontal = false;// true = vertical , false= horizontal
	bool y_increase = false;

	// 입력파일 읽어서 airpoile에 저장 C:\Users\준형\source\repos\Airfoile\Airfoile
	if ((inputfp = fopen("input.txt", "r")) == NULL) {
		cout << "input file을 찾을수 없습니다 " << endl;
		return 0;

	}
	fscanf(inputfp, "%d %d %d %d", &n, &uppern, &lowern, &tail);
	for (int i = 0; i < n; i++) {
		fscanf(inputfp, "%lf %lf", &x, &y);
		airpoile.push_back(make_pair(x, y));
	}

	//초기화
	initRectangle = new Rectangle(LeftDownX, LeftDownY, initRCellsize, Xcount, Ycount);//진_initcellsize설정
	initCellMaxID = initRectangle->initCell[initRectangle->widthCount - 1][initRectangle->heightCount - 1].getID();
	mincellsize = initRectangle->initCellSize / pow((double)2, (double)depth); // 재 : 잠정 중지 2017_02_20

																			   //splitcell
																			   //initRectangle->splitCell(-2, -2, 3, 3, 1, 2, depth);
																			   //    cout << "PHASE : splitcell 완료" << endl;
																			   // mininode 찾기
	int ud = 0;

	double length_prev = 0;
	int index = 0;
	for (int i = 0; i < n - 1; i++) {
		if (i == uppern - 1) {
			index = 0;
			ud = 1;
		}
		findminnode(airpoile[i].first, airpoile[i].second, airpoile[i + 1].first, airpoile[i + 1].second, mincellsize, ud);
		inputLine.push_back(make_pair(airpoile[i], airpoile[i + 1]));

	}


	cout << "PHASE : findmininode 완료" << endl;

	// vertical 인지 horizontal 인지 확인
	if (vertical_horizontal) {//세로형이면
		sort(mininode[0].begin(), mininode[0].end(), cmp_vertical);
		sort(mininode[1].begin(), mininode[1].end(), cmp_vertical);
		y_increase = airpoile[0].second < airpoile[uppern - 1].second;
	}

	//dotpoint 및 mininodecell 찾기
	for (int i = 0; i < 2; i++) {
		if (!mininode[i].empty()) {
			for (int k = 0; k < mininode[i].size(); k++) {
				initRectangle->dotPoint(mininode[i][k].first, mininode[i][k].second, layer, depth);
				if (k != 0) {
					setupmininodeCell(mininode[i][k - 1], mininode[i][k]);
				}
			}
		}
	}
	cout << "PHASE : setupmininodeCell 완료" << endl;
	// isshape 하면서 delete를 해버려서 seupmininodecell이 참조못하는 경우가 생겨서 분리함
	if (!vertical_horizontal) {
		for (int i = 0; i < 2; i++) {
			if (!mininode[i].empty()) {
				for (int k = 1; k < mininode[i].size(); k++) {
					if (i == 0) {
						if (!isshape(mininode[i][k - 1], mininode[i][k], 1, mincellsize)) return 0;
					}
					else {
						if (!isshape(mininode[i][k - 1], mininode[i][k], -1, mincellsize)) return 0;
					}
				}
			}
		}
	}
	else {
		for (int i = 0; i < 2; i++) {
			if (!mininode[i].empty()) {
				for (int k = 1; k < mininode[i].size(); k++) {
					if ((y_increase && i == 1) || (!y_increase && i == 0)) {
						if (!isshape(mininode[i][k - 1], mininode[i][k], 1, mincellsize)) return 0;
					}
					else {
						if (!isshape(mininode[i][k - 1], mininode[i][k], -1, mincellsize)) return 0;
					}
				}
			}
		}
	}
	cout << "PHASE : Isshape 완료" << endl;

	if (tail == 1) {// 진 : tail 설정  2017-03-07
		tailmanager(airpoile[0], airpoile[n - 1], mincellsize, layer, depth);

		cout << "tailcut 완료" << endl;
	}

	//cutInnerCell();
	bool flag = false;
	cout << "PHASE : manageEdge 완료" << endl;

	// 재: 2017_03_10 cutInnerCell ccw알고리즘 제거 및 vertical check 작업 추가
	// 재 : cutInnerCell 제거, ccw알고리즘 제거, checkInner 변경,
	checkInner(LeftDownX, LeftDownY, initRCellsize, Xcount, Ycount, flag);

	sort(nodeVector.begin(), nodeVector.end(), nodeCompare); // 2017_03_16 재
	deleteInnerFace();
	cout << "PHASE : cutinnetcell 완료" << endl;

	sort(detfaceVector.begin(), detfaceVector.end());

	vector<detface>::iterator pos = unique(detfaceVector.begin(), detfaceVector.end()); // 재 : detfaceVector에 face가 중복되서 들어가는 문제가 발생하여 코드 추가
	detfaceVector.erase(pos, detfaceVector.end());                                        // 재 : 중복되는 값을 제거 // 2017_02_13

	for (int i = detfaceVector.size() - 1; i >= 0; i--) {
		delete(detfaceVector[i].second);
	}

	for (int i = 0; i < wall.size(); i++) {
		Face* wf = new Face(nodeVector[wall[i].second.first], nodeVector[wall[i].second.second]);
		if (wall[i].first.first == 0) wf->sCell = cellVector[wall[i].first.second];
		if (wall[i].first.first == 1) wf->sCell = minimalCellVector_tri[wall[i].first.second];
		wallVector.push_back(wf);
	}

	sort(detcellVector.begin(), detcellVector.end());

	vector<detcell>::iterator posC = unique(detcellVector.begin(), detcellVector.end()); // 재 : detcellVector에 face가 중복되서 들어가는 문제가 발생하여 코드 추가
	detcellVector.erase(posC, detcellVector.end());                                        // 재 : 중복되는 값을 제거 // 2017_02_13

	for (int i = detcellVector.size() - 1; i >= 0; i--) {
		delete(detcellVector[i].second);
		detcellVector[i].second->setID(0);
	}



	sort(faceVector.begin(), faceVector.end(), faceCompare);
	sort(cellVector.begin(), cellVector.end(), cellCompare);

	for (unsigned int i = 0; i<faceVector.size(); i++) {
		if (faceVector[i]->id == 365)
			cout << endl;
		faceVector[i]->setID(i + 1);
	}

	for (unsigned int i = 0; i<cellVector.size(); i++) {
		cellVector[i]->setID(i + 1);
	}

	for (unsigned int i = 0; i < minimalCellVector_tri.size(); i++) {//0208
		minimalCellVector_tri[i]->setID(i + 1 + cellVector.size());
	}


	fp = fopen("/Users/rhino/Documents/Airfolie/Airfolie/Test_07_31_jae_.msh", "w");

	DescribeMesh();

	fclose(inputfp);
	fclose(fp); //진_cutcell수정

				//initRectangle->printAll();
				//initRectangle->confirmAll();


	return 0;
}