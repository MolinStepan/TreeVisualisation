#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <string>
#include <windows.h>
#include <chrono>
#include <stdlib.h>
#include <fstream>

const int height[7] = { 144,  368,  480,  536,  564, 578, 585 };
const int width[7] = { 64, 32, 16, 8, 4, 2, 1 };

int selectedDepth = -1;
long long selectedNumber = 0;  // номер отсчитывается от самого левого потенциально возможного элемента дерева на этой глубине 
int heightstNodeOnScreenDepth = 0;
long long heightstNodeOnScreenNumber = 0; // номер отсчитывается от самого левого потенциально возможного элемента дерева на этой глубине 
bool isRight = true;

// бинарное дерево
struct Tree
{
	short balance;
	Tree* Right;
	Tree* Left;
	unsigned long long value;
	Tree(unsigned long long f) { value = f; Left = Right = nullptr; balance = 0; }
};

// односвязный список, используется как стек, только для отображения конкретного звена дерева	
struct Line {
	Tree* node;
	Line* prev;
	Line(Line* f, Tree* m) { prev = f; node = m; }
};

//двусвязный список, 
struct List { 
	List* prev;
	Tree* root;
	List* next = nullptr;
	List(List* p) { prev = p; }
};

void deleteTree(List* p);


Tree* nodeAtTopOfScreen = nullptr;
Line* fin = nullptr;
Line* annihilator;
List* last = nullptr;
List* first = nullptr;
List* selectedList = nullptr;
List* LeftestTreeOnScreen = nullptr;
int selectedN;
int leftestRootOnScreen = 0;
int numberOfTrees = 0;

// рекурсивно удаляет дерево
void clear(Tree* f) { 
	if (f == nullptr)
		return;
	clear(f->Left);
	clear(f->Right);
	delete f;
	return;
}

// правый поворот вокруг p
void rotateright(Tree*& p) 
{
	Tree* q = p->Left;
	p->Left = q->Right;
	q->Right = p;
	p = q;
	return;
}

// правый поворот вокруг p
void rotateleft(Tree*& p) 
{
	Tree* q = p->Right;
	p->Right = q->Left;
	q->Left = p;
	p = q;
	return;
}

// большой правый поворот
void bigrotateright(Tree*& p) { 
	rotateleft(p->Left);
	rotateright(p);
	return;
}

// большой левый поворот
void bigrotateleft(Tree*& p) { 
	rotateright(p->Right);
	rotateleft(p);
	return;
}

// пересчитывает балансы и балансирует дерево
void rotate(Tree*& p) { 
	if (p->balance == -2) {
		if (p->Right->balance == 1)
		{
			if (p->Right->Left->balance == 1) {
				p->balance = 0;
				p->Right->balance = -1;
				p->Right->Left->balance = 0;
			}
			else if (p->Right->Left->balance == 0) {
				p->balance = 0;
				p->Right->balance = 0;
				p->Right->Left->balance = 0;
			}
			else {
				p->balance = 1;
				p->Right->balance = 0;
				p->Right->Left->balance = 0;
			}
			bigrotateleft(p);
		}
		else {
			if (p->Right->balance == 0) {
				p->balance = -1;
				p->Right->balance = 1;
			}
			else {
				p->balance = 0;
				p->Right->balance = 0;
			}
			rotateleft(p);
		}
	}
	else
	{
		if (p->Left->balance == -1)
		{
			if (p->Left->Right->balance == -1) {
				p->balance = 0;
				p->Left->balance = 1;
				p->Left->Right->balance = 0;
			}
			else if (p->Left->Right->balance == 0) {
				p->balance = 0;
				p->Left->balance = 0;
				p->Left->Right->balance = 0;
			}
			else {
				p->balance = -1;
				p->Left->balance = 0;
				p->Left->Right->balance = 0;
			}
			bigrotateright(p);
		}
		else {
			if (p->Left->balance == 0) {
				p->balance = 1;
				p->Left->balance = -1;
			}
			else {
				p->balance = 0;
				p->Left->balance = 0;
			}
			rotateright(p);
		}
	}
	return;
}

// считает, сколько деревьев правее самого левого на экране следует выводить
int countRight() {
	int k = 0;
	List* a = LeftestTreeOnScreen;
	while (a != nullptr && k < 8) {
		a = a->next;
		k++;
	}
	return k;
}

// очищает 
void deleteLine(Line* p) {
	if (p == nullptr)
		return;
	deleteLine(p->prev);
	delete(p);
}
void deleteLine() {
	deleteLine(fin);
	fin = nullptr;
	return;
}

void selectTree(List* p) {
	selectedList = p;
	deleteLine();
	selectedDepth = -1;
	selectedNumber = 0;
	heightstNodeOnScreenDepth = 0;
	heightstNodeOnScreenNumber = 0;
	if (p != nullptr) {
		fin = new Line(nullptr, selectedList->root);
		selectedList->root = selectedList->root;
		nodeAtTopOfScreen = selectedList->root;
	}
	else
		nodeAtTopOfScreen = nullptr;
	return;
}

// добавляет новое дерево, создаёт новый элемент списка для его хранения, выбирает новое дерево
void addTree(unsigned long long AValue) {
	if (first == nullptr) {
		last = new List(nullptr);
		first = last;
		LeftestTreeOnScreen = last;
		leftestRootOnScreen = 1;
	}
	else
	{
		last = new List(last);
		last->prev->next = last;
	}
	last->root = new Tree(AValue);
	selectTree(last);
	isRight = true;
	numberOfTrees++;
	return;
}

// удаляет дерево и элемент списка, выбирает последнее дерево
void deleteTree(List* p) {
	if (last == p)
		last = p->prev;
	if (first == p)
		first = p->next;
	if (LeftestTreeOnScreen == p)
		LeftestTreeOnScreen = p->prev == nullptr ? p->next : p->prev;
	if (p->prev != nullptr)
		p->prev->next = p->next;
	if (p->next != nullptr)
		p->next->prev = p->prev;
	clear(p->root);
	delete p;
	selectTree(last);
	if (last == nullptr) {
		leftestRootOnScreen = 0;
		LeftestTreeOnScreen = nullptr;
	}
	else
	{

	} {
		if (!isRight) {
			leftestRootOnScreen--;
		}
		else
		{
			if (countRight() < 7 && leftestRootOnScreen > 1) {
				LeftestTreeOnScreen = LeftestTreeOnScreen->prev;
				leftestRootOnScreen--;
			}
		}
	}
	isRight = true;
	numberOfTrees--;
	return;
}

// добавляет элемент дерева   
bool addElement (unsigned long long value, Tree*& node) {
	static bool shouldBalance = true;
	if (value > node->value) {
		if (node->Right != nullptr) {
			if (!addElement(value, node->Right)) return false;
			if (shouldBalance) {
				node->balance--;
				if (node->balance * node->balance == 4) {
					rotate(node);
				}
				if (node->balance == 0)
					shouldBalance = false;
			}
			return true;
		}
		else
		{
			node->Right = new Tree(value);
			node->balance--;
			shouldBalance = node->balance != 0;
			return true;
		}
	}
	else if (value < node->value) {
		if (node->Left != nullptr) { 
			if (!addElement(value, node->Left)) return false;
			if (shouldBalance) {
				node->balance++;
				if (node->balance * node->balance == 4) {
					rotate(node);
				}
				if (node->balance == 0)
					shouldBalance = false;
			}
			return true;
		}
		else
		{
			node->Left = new Tree(value);
			node->balance++;
			shouldBalance = node->balance != 0;
			return true;
		}
	}
	else
		return false;
} 

// удаляет элемент value 
bool shouldBalance = true;
std::pair<unsigned long long, bool> deleteElement(unsigned long long value, Tree*& node) {
	std::pair<unsigned long long, bool> a = { 0, false };
	bool recursionGoneToRight = false;
	static bool isleft;
	static bool rebalanceAfterRecursion = false;
	if (shouldBalance) {  ////////////////////////////?
		if (value > node->value) {
			if (node->Right != nullptr) {
				a = deleteElement(value, node->Right);
				recursionGoneToRight = true;
			}
			else
			{
				return a;
			}
		}
		else if (value < node->value) {
			if (node->Left != nullptr) {
				a = deleteElement(value, node->Left);
			}
			else
			{
				return a;
			}
		}
		else {
			rebalanceAfterRecursion = true;
			shouldBalance = false;
			isleft = node->balance == 1;
			if (node->Left == nullptr && node->Right == nullptr)
			{
				delete(node);
				node = nullptr;
				a.second = true;
				return a;
			}
			else
			{
				a = deleteElement(value, isleft ? node->Left : node->Right);
				node->value = a.first;
				recursionGoneToRight = !isleft;
			}
		}
	}
	else {
		if (isleft) {
			recursionGoneToRight = true;
			if (node->Right == nullptr) {
				Tree* q = node;
				a.first = node->value;
				a.second = true;
				node = q->Left;
				delete(q);
				return a;
			}
			else
				a = deleteElement(value, node->Right);
		}
		else {
			if (node->Left == nullptr) {
				Tree* q;
				q = node;
				a.first = node->value;
				a.second = true;
				node = node->Right;
				delete(q);
				return a;
			}
			else
				a = deleteElement(value, node->Left);
		}

	}
	if (a.second) {
		if (rebalanceAfterRecursion) {
			if (recursionGoneToRight) {
				node->balance++;
			}
			else
			{
				node->balance--;
			}
			if (node->balance * node->balance == 4)
				rotate(node);
			if (node->balance * node->balance == 1)
				rebalanceAfterRecursion = false;
		}
	}
	return a;
}

// ищет путь до искомого элемента, возвращает его как битовую последовательность
unsigned long long findway(int depth, long long numberFromLeft) {
	unsigned long long a = 0;
	unsigned long long b = 1;
	numberFromLeft >>= 3;
	numberFromLeft++;
	while (depth > 0) {
		if (numberFromLeft & 1) {
			numberFromLeft = (numberFromLeft + 1) >> 1;
		}
		else
		{
			a |= b;
			numberFromLeft >>= 1;
		}
		b <<= 1;
		--depth;
	}
	return a;
}

// проходит по пути и возвращает элемент дерева
Tree* getNode(int depth, long long way, Tree* Root) {
	if (depth < 0) return nullptr;
	while (depth > 0) {
		--depth;
		if ((way >> depth) & 1)
			if (Root->Right != nullptr)
				Root = Root->Right;
			else
				return nullptr;
		else
			if (Root->Left != nullptr)
				Root = Root->Left;
			else
				return nullptr;
	}
	return Root;
}

// проходит по пути и выбирает элемент
void go(int depth, long long way) {
	if (depth < 0) {
		while (depth < 0) {
			++depth;
			annihilator = fin;
			fin = fin->prev;
			nodeAtTopOfScreen = fin->node;
			if (nodeAtTopOfScreen->Right == annihilator->node) {
				selectedNumber += 2 << (selectedDepth + 2);
				heightstNodeOnScreenNumber += 2 << (heightstNodeOnScreenDepth + 2);
			}
			heightstNodeOnScreenDepth += 1;
			selectedDepth += 1;
			delete(annihilator);
		}
	}
	else {
		while (depth > 0) {
			--depth;
			if ((way >> depth) & 1) {
				fin = new Line(fin, fin->node->Right);
				selectedDepth -= 1;
				selectedNumber -= 2 << (selectedDepth + 2);
				heightstNodeOnScreenDepth -= 1;
				heightstNodeOnScreenNumber -= 2 << (heightstNodeOnScreenDepth + 2);
			}
			else {
				fin = new Line(fin, fin->node->Left);
				selectedDepth -= 1;
				heightstNodeOnScreenDepth -= 1;
			}
			nodeAtTopOfScreen = fin->node;
		}
	}
}



// находит, куда приозошло нажатие и меняет переменную в зависимости от этого
std::pair<int, int> locateClick(int x, int y, int& typeOfSelectedInput, short& selectedButton) {
	std::pair<int, int> a = { 0, 0 };
	typeOfSelectedInput = 0;
	selectedButton = 0;
	if (x < 512 && x > 0 && y < 592 && y > 80) {
		y -= 80;
		int C = (512 - y) >> 3;
		short i = 0;
		while (C > 0) {
			C = C >> 1;
			++i;
		}


		int Z = x / (2 << (i + 2));
		i = 6 - i;
		if ((x >= width[i] * (1 + (Z << 3))) &&
			(x <= width[i] * (1 + (Z << 3)) + width[i] * 6) &&
			(y + 80 >= height[i]) &&
			(y + 80 <= height[i] + (width[i] << 1))) {
			typeOfSelectedInput = 1;
			a = { i, Z };
		}
	}
	else if (x > 512 && x < 812 && y>80 && y < 592) {
		y -= 80;
		x -= 512;
		if (x >= 10 && x <= 145 && y > 10 && y < 50) { a.first = 0; typeOfSelectedInput = 2; }
		else if (x >= 155 && x <= 290 && y > 10 && y < 50) { a.first = 1; typeOfSelectedInput = 2; }
		else if (x >= 10 && x <= 145 && y > 60 && y < 100) { a.first = 2; typeOfSelectedInput = 2; }
		else if (x >= 155 && x <= 290 && y > 60 && y < 100) { a.first = 3; typeOfSelectedInput = 2; }
		else if (x >= 10 && x <= 290 && y > 110 && y < 150) { typeOfSelectedInput = 3; selectedButton = 4; }
		else if (x >= 10 && x <= 290 && y > 160 && y < 200) { typeOfSelectedInput = 3; selectedButton = 5; }
		else if (x >= 10 && x <= 290 && y > 210 && y < 250) { typeOfSelectedInput = 3; selectedButton = 6; }
		else if (x >= 10 && x <= 290 && y > 260 && y < 300) { a.first = 7; typeOfSelectedInput = 2; }
	}
	else if (x >= 10 && x <= 802 && y >= 10 && y <= 70) {
		if (x >= 756 && x <= 802) {
			typeOfSelectedInput = 4;
			a.first = 1;
		}
		else if (x >= 700 && x <= 846) {
			typeOfSelectedInput = 4;
			a.first = 2;
		}
		else if (x >= 10 && x <= 30) {
			typeOfSelectedInput = 4;
			a.first = 3;
		}
		else if (x >= 670 && x <= 690) {
			typeOfSelectedInput = 4;
			a.first = 4;
		}
		else if (x >= 40 && x <= 660) {
			if ((x - 40) % 90 <= 80)
				typeOfSelectedInput = 4;
			a.first = 5 + (x - 40) / 90;
		}

	}
	return a;
}


// рекурсивно рисует дерево
void drawtree(sf::RenderWindow& window, int depth, int numberFromLeft, Tree* p, bool shouldDrawUpBranch, const sf::Font& font) {
	std::string nodeValueToDraw;
	if (depth == 7)
		return;
	sf::RectangleShape sqr;
	sf::Text txt;
	if (shouldDrawUpBranch) {
		sf::RectangleShape line_with_thickness(sf::Vector2f(width[depth] >> 1, depth == 0 ? width[depth] : 3 * width[depth]));

		line_with_thickness.setFillColor(sf::Color(128, 128, 128));

		line_with_thickness.setPosition(width[depth] * (3.75f + numberFromLeft), height[depth] - (depth == 0 ? width[depth] : 3 * width[depth]));

		window.draw(line_with_thickness);

	}
	sqr.setSize(sf::Vector2f(width[depth] * 6, width[depth] << 1));
	sqr.setPosition(width[depth] * (1 + numberFromLeft), height[depth]);
	if (numberFromLeft == selectedNumber && depth == selectedDepth)
		sqr.setFillColor(sf::Color(255, 0, 0));
	else
		sqr.setFillColor(sf::Color(255, 255, 255));
	window.draw(sqr);
	if (depth < 4) {

		txt.setCharacterSize(width[depth]);
		txt.setFont(font);
		if (p->value >= 100000000) {
			unsigned long long F = 1;
			short i = -1;
			while (F <= p->value) {
				F *= 10;
				++i;
			}
			F /= 10;
			short j = i > 9 ? 3 : 4;
			nodeValueToDraw = std::to_string(p->value / F) + '.';
			for (; j > 0; --j) {
				F /= 10;
				nodeValueToDraw += std::to_string((p->value % (10 * F)) / F);
			}
			nodeValueToDraw += "E" + std::to_string(i);
		}
		else nodeValueToDraw = std::to_string(p->value);
		txt.setString(nodeValueToDraw);
		txt.setPosition(width[depth] * (1 + numberFromLeft) + (width[depth] >> 1), height[depth] + (width[depth] >> 1));
		txt.setFillColor(sf::Color::Black);
		window.draw(txt);
	}

	




	if (p->Left != nullptr) {
		drawtree(window, depth + 1, 2 * numberFromLeft, p->Left, true, font);
	}
	if (p->Right != nullptr) {
		drawtree(window, depth + 1, 2 * numberFromLeft + 8, p->Right, true, font);
	}
	return;
}

int main() {
	sf::Font font;
	system("dir");
	srand(time(0));

	int typeOfSelectedButton;
	short selectedButton = 0;
	unsigned long long AValue = 0, BValue = 0;
	int NValue = 0;
	char inputKey;
	std::string stringForInputA, stringForInputB, stringForInputN;

	bool mouseButtonIsPressed = true;

	std::pair<int, int> s;

	sf::RectangleShape Trees[7];
	sf::Text TreesNums[7];
	for (int i = 0; i < 7; ++i) {
		Trees[i].setSize(sf::Vector2f(80, 60));
		Trees[i].setFillColor(sf::Color::White);
		Trees[i].setPosition(40 + i * 90, 10);
		TreesNums[i].setPosition(45 + i * 90, 20);
		TreesNums[i].setCharacterSize(40);
		TreesNums[i].setFillColor(sf::Color::Black);
		TreesNums[i].setFont(font);
	}

	sf::RectangleShape LeftButton;
	sf::Text LeftButtonText;
	sf::RectangleShape RightButton;
	sf::Text RightButtonText;
	LeftButton.setSize(sf::Vector2f(20, 60));
	LeftButton.setPosition(10, 10);
	RightButton.setSize(sf::Vector2f(20, 60));
	RightButton.setPosition(670, 10);
	LeftButtonText.setString("<");
	LeftButtonText.setPosition(10, 20);
	LeftButtonText.setCharacterSize(32);
	LeftButtonText.setFillColor(sf::Color(0, 0, 0));
	LeftButtonText.setFont(font);
	RightButtonText.setString(">");
	RightButtonText.setPosition(670, 20);
	RightButtonText.setCharacterSize(32);
	RightButtonText.setFillColor(sf::Color(0, 0, 0));
	RightButtonText.setFont(font);

	sf::RectangleShape AddButton;
	sf::Text AddButtonText;
	sf::RectangleShape RemoveButton;
	sf::Text RemoveButtonText;
	AddButton.setSize(sf::Vector2f(46, 60));
	AddButton.setPosition(sf::Vector2f(756, 10));
	RemoveButton.setSize(sf::Vector2f(46, 60));
	RemoveButton.setPosition(sf::Vector2f(700, 10));
	AddButtonText.setString("+A");
	AddButtonText.setPosition(759, 20);
	AddButtonText.setCharacterSize(32);
	AddButtonText.setFillColor(sf::Color(0, 0, 0));
	AddButtonText.setFont(font);
	RemoveButtonText.setString("X");
	RemoveButtonText.setPosition(713, 20);
	RemoveButtonText.setCharacterSize(32);
	RemoveButtonText.setFillColor(sf::Color(0, 0, 0));
	RemoveButtonText.setFont(font);


	sf::RectangleShape UP;
	sf::RectangleShape RIGHT;
	UP.setFillColor(sf::Color(64, 64, 64));
	RIGHT.setFillColor(sf::Color(128, 128, 128));
	UP.setSize(sf::Vector2f(812, 80));
	RIGHT.setSize(sf::Vector2f(300, 512));
	UP.setPosition(0, 0);
	RIGHT.setPosition(512, 80);
	RIGHT.setPosition(512, 80);

	if (!font.loadFromFile("EtelkaMonospace.otf")) {
		std::cout << "Font not loaded\n";
	}
	sf::RectangleShape Button[8];
	sf::Text Txt[8];
	sf::Text Console;

	Console.setFont(font);
	Console.setCharacterSize(20);
	Console.setPosition(sf::Vector2f(527, 395));
	Console.setFillColor(sf::Color(0, 0, 0));

	for (int i = 0; i < 4; ++i) {
		Button[i].setSize(sf::Vector2f(135, 40));
		Button[i].setPosition(sf::Vector2f((i & 1) * 145 + 522, (i >> 1) * 50 + 90));
		Txt[i].setFont(font);
		Txt[i].setCharacterSize(20);
		Txt[i].setPosition(sf::Vector2f((i & 1) * 145 + 527, (i >> 1) * 50 + 95));
		Button[i].setFillColor(sf::Color(255, 255, 255));
		Txt[i].setFillColor(sf::Color(0, 0, 0));
	}
	for (int i = 4; i < 8; ++i) {
		Button[i].setSize(sf::Vector2f(280, 40));
		Button[i].setPosition(sf::Vector2f(522, (i - 2) * 50 + 90));
		Txt[i].setFont(font);
		Txt[i].setCharacterSize(20);
		Txt[i].setPosition(sf::Vector2f(527, (i - 2) * 50 + 95));
		Button[i].setFillColor(sf::Color(255, 255, 255));
		Txt[i].setFillColor(sf::Color(0, 0, 0));
	}
	Txt[0].setString("Deselect");
	Txt[1].setString("Goto");
	Txt[2].setString("Delete");
	Txt[3].setString("Add A");
	Txt[7].setString("Add N random in [A, B]");
	bool needUpdate = true;
	int windowSize[2] = { 812, 592 };
	sf::RenderWindow window(sf::VideoMode(windowSize[0], windowSize[1]), "Forest");





	while (window.isOpen()) {
		sf::Event event;

		// рисует экран
		if (needUpdate) {
			needUpdate = false;
			window.clear();
			window.draw(UP);
			window.draw(RIGHT);
			window.draw(Console);

			if (LeftestTreeOnScreen == nullptr || LeftestTreeOnScreen->prev == nullptr) {
				LeftButton.setFillColor(sf::Color(64, 64, 64));
			}
			else {
				LeftButton.setFillColor(sf::Color::White);
			}
			window.draw(LeftButton);
			window.draw(LeftButtonText);

			if (LeftestTreeOnScreen == nullptr || countRight() < 8){
				RightButton.setFillColor(sf::Color(64, 64, 64));
			}
			else {
				RightButton.setFillColor(sf::Color::White);
			}
			window.draw(RightButton);
			window.draw(RightButtonText);


			if (stringForInputA == "" || numberOfTrees >= 9999) {
				AddButton.setFillColor(sf::Color(64, 64, 64));
			} 
			else {
				AddButton.setFillColor(sf::Color::White);
			}
			window.draw(AddButton);
			window.draw(AddButtonText);
			if (selectedList == nullptr) {
				RemoveButton.setFillColor(sf::Color(64, 64, 64));
			}
			else {
				RemoveButton.setFillColor(sf::Color::White);
			}
			window.draw(RemoveButton);
			window.draw(RemoveButtonText);
			short k = 0;
			List* a = LeftestTreeOnScreen;
			while (a != nullptr && k < 7) {
				if (a == selectedList)
					Trees[k].setFillColor(sf::Color(255, 0, 0));
				else
					Trees[k].setFillColor(sf::Color::White);
				TreesNums[k].setString(std::to_string(leftestRootOnScreen + k));
				window.draw(Trees[k]);
				window.draw(TreesNums[k]);
				a = a->next;
				++k;
			}
			Txt[4].setString("A=" +stringForInputA + (selectedButton == 4 ? stringForInputA.length() == 19 ? "|" : "_" : ""));
			Txt[5].setString("B=" + stringForInputB + (selectedButton == 5 ? stringForInputB.length() == 19 ? "|" : "_" : ""));
			Txt[6].setString("N=" + stringForInputN + (selectedButton == 6 ? stringForInputN.length() == 5 ? "|" : "_" : ""));
			if (selectedList == nullptr || selectedDepth < heightstNodeOnScreenDepth)
			{
				Button[0].setFillColor(sf::Color(128, 128, 128));
				Button[1].setFillColor(sf::Color(128, 128, 128));
				Button[2].setFillColor(sf::Color(128, 128, 128));
			}
			else
			{
				Button[0].setFillColor(sf::Color(255, 255, 255));
				Button[1].setFillColor(sf::Color(255, 255, 255));
				Button[2].setFillColor(sf::Color(255, 255, 255));
			}
			if (selectedList == nullptr || stringForInputA.length() == 0)
				Button[3].setFillColor(sf::Color(128, 128, 128));
			else
				Button[3].setFillColor(sf::Color(255, 255, 255));
			if (selectedList == nullptr || stringForInputA.length() == 0 || stringForInputB.length() == 0 || NValue == 0 || BValue == AValue)
				Button[7].setFillColor(sf::Color(128, 128, 128));
			else
				Button[7].setFillColor(sf::Color(255, 255, 255));
			for (int i = 0; i < 8; ++i) {
				window.draw(Button[i]);
				window.draw(Txt[i]);
			}
			if (selectedList != nullptr)
				drawtree(window, 0, 0, nodeAtTopOfScreen, !(nodeAtTopOfScreen == selectedList->root), font);
				

			window.display();
		}

		while (window.pollEvent(event))
		{
			// обработка кликов
			if (event.type == sf::Event::MouseButtonReleased) {
				mouseButtonIsPressed = true;
			}
			else if (mouseButtonIsPressed && event.type == sf::Event::MouseButtonPressed) {

				s = locateClick(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y, typeOfSelectedButton, selectedButton);

				if (selectedList != nullptr && typeOfSelectedButton == 1 && getNode(s.first, s.second, nodeAtTopOfScreen) != nullptr) {
					selectedDepth = s.first;
					selectedNumber = s.second << 3;
				}
				else if (selectedList != nullptr && typeOfSelectedButton == 2) {

					if (s.first == 0) {
						selectedDepth = heightstNodeOnScreenDepth - 1;
						selectedNumber = heightstNodeOnScreenNumber;
					}
					else if (s.first == 1) {
						if (selectedDepth >= heightstNodeOnScreenDepth) {
							go(heightstNodeOnScreenDepth, 0);
							go(selectedDepth, findway(selectedDepth, selectedNumber));
						}
					}
					else if (s.first == 2) {
						if (selectedDepth >= heightstNodeOnScreenDepth) {
							bool F;
							go(heightstNodeOnScreenDepth, 0);
							if (selectedDepth == heightstNodeOnScreenDepth && selectedList->root->Right == nullptr && selectedList->root->Left == nullptr) {
								deleteTree(selectedList);
								F = true;
							}
							else {
								shouldBalance = true;
								F = deleteElement(getNode(selectedDepth, findway(selectedDepth, selectedNumber), nodeAtTopOfScreen)->value, selectedList->root).second; ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
								selectedDepth = -1;
								selectedNumber = 0;
								nodeAtTopOfScreen = selectedList->root;
								deleteLine();
								fin = new Line(nullptr, selectedList->root);
							}
							Console.setString(F ? "Element deleted\nsuccessfully" : "No element deleted");
						}
					}
					else if (s.first == 3 && stringForInputA.length() > 0) {
						go(heightstNodeOnScreenDepth, 0);
						selectedDepth = -1;
						selectedNumber = 0;
						if (addElement(AValue, selectedList->root))
							Console.setString("1 element added \nsuccessfully");
						else
							Console.setString("failed");
						std::cout << shouldBalance << std::endl;
						nodeAtTopOfScreen = selectedList->root;
						deleteLine();
						fin = new Line(nullptr, selectedList->root);
					}
					else if (s.first == 7 && AValue != BValue && stringForInputA.length() > 0 && stringForInputB.length() != 0 && stringForInputN.length() != 0) {
						long long AA = AValue > BValue ? AValue : BValue;
						long long BB = AValue - AA + BValue;
						AA = AA - BB + 1;
						int J = 5 * NValue;
						int K = NValue;
						go(heightstNodeOnScreenDepth, 0);
						selectedDepth = -1;
						selectedNumber = 0;
						while (J > 0 && K > 0) {
							K -= addElement((((((((unsigned long long)rand() << 15) + rand()
								<< 15) + rand()
								<< 15) + rand()
								<< 4) + (rand() & 15)) % AA) + BB, selectedList->root);
							--J;
						}

						nodeAtTopOfScreen = selectedList->root;
						deleteLine();
						fin = new Line(nullptr, selectedList->root);
						if (K == 0)
							Console.setString(std::to_string(NValue) + " element" + (NValue != 1 ? "s" : "") + " added \nsuccessfully");
						else
							Console.setString("failed, " + std::to_string(NValue - K) + " element" + (NValue - K != 1 ? "s" : "") + "\nadded");
					}
				}




				else if (typeOfSelectedButton == 4) {
					if (s.first == 1 && stringForInputA != "" && numberOfTrees < 9999) {
						Console.setString("Tree added");
						addTree(AValue);
					}
					else if (s.first == 1) {
						Console.setString("No tree added");
					}
					else if (s.first == 2 && selectedList != nullptr) {
						deleteTree(selectedList);
						Console.setString("Tree deleted");
					}
					else if (s.first == 3 && selectedList != nullptr) {
						if (LeftestTreeOnScreen->prev != nullptr) {
							LeftestTreeOnScreen = LeftestTreeOnScreen->prev;
							leftestRootOnScreen--;
							if (LeftestTreeOnScreen == selectedList) isRight = true;
						}
					}
					else if (s.first == 4 && selectedList != nullptr) {
						if (countRight() == 8) {
							if (LeftestTreeOnScreen == selectedList) isRight = false;
							LeftestTreeOnScreen = LeftestTreeOnScreen->next;
							leftestRootOnScreen++;
						}
					}
					else {
						List* treeToBeSelected = LeftestTreeOnScreen;
						int h = countRight();
						if (h + 4 >= s.first) {
							while (s.first > 5) {
								treeToBeSelected = treeToBeSelected->next;
								--s.first;
							}
							selectTree(treeToBeSelected);
							isRight = true;
						}
					}
				}
				needUpdate = true;
				mouseButtonIsPressed = false;
			}

			// обработка колёсика мыши
			else if (selectedList != nullptr && event.type == sf::Event::MouseWheelMoved) {
				if (sf::Mouse::getPosition(window).x >= 0 && sf::Mouse::getPosition(window).x <= 512 && sf::Mouse::getPosition(window).y > 80 && sf::Mouse::getPosition(window).y <= 592) {
					if (event.mouseWheel.delta > 0) {
						if (sf::Mouse::getPosition(window).x > 256 && fin->node->Right != nullptr) {
							fin = new Line(fin, fin->node->Right);
							selectedDepth -= 1;
							selectedNumber -= 2 << (selectedDepth + 2);
							heightstNodeOnScreenDepth -= 1;
							heightstNodeOnScreenNumber -= 2 << (heightstNodeOnScreenDepth + 2);
						}
						else if (fin->node->Left != nullptr) {
							fin = new Line(fin, fin->node->Left);
							selectedDepth -= 1;
							heightstNodeOnScreenDepth -= 1;
						}
						nodeAtTopOfScreen = fin->node;

					}
					else {
						if (fin->prev != nullptr) {
							annihilator = fin;
							fin = fin->prev;
							nodeAtTopOfScreen = fin->node;
							if (nodeAtTopOfScreen->Right == annihilator->node) {
								selectedNumber += 2 << (selectedDepth + 2);
								heightstNodeOnScreenNumber += 2 << (heightstNodeOnScreenDepth + 2);
							}
							heightstNodeOnScreenDepth += 1;
							selectedDepth += 1;
							delete(annihilator);
						}
					}
					needUpdate = true;
				}
			}

			// закрытие программы
			else if (event.type == sf::Event::Closed)
				window.close();
 
			// ввод символа
			else if (event.text.unicode < 128) {
				if (selectedButton == 4) {
					inputKey = event.text.unicode;
					if (inputKey <= '9' && inputKey >= '0' && stringForInputA.length() < 19) {
						stringForInputA += inputKey;
						AValue = AValue * 10 + (inputKey - '0');
						needUpdate = true;
					}
					else if (inputKey == '\b' && stringForInputA.length() > 0) {
						AValue /= 10;
						stringForInputA.erase(stringForInputA.length() - 1);
						needUpdate = true;
					}

				}
				else if (selectedButton == 5) {
					inputKey = event.text.unicode;
					if (inputKey <= '9' && inputKey >= '0' && stringForInputB.length() < 19) {
						stringForInputB += inputKey;
						BValue = BValue * 10 + (inputKey - '0');
						needUpdate = true;
					}
					else if (inputKey == '\b' && stringForInputB.length() > 0) {
						BValue /= 10;
						stringForInputB.erase(stringForInputB.length() - 1);
						needUpdate = true;
					}

				}
				else if (selectedButton == 6) {
					inputKey = event.text.unicode;
					if (inputKey <= '9' && inputKey >= '0' && stringForInputN.length() < 5) {
						stringForInputN += inputKey;
						NValue = NValue * 10 + (inputKey - '0');
						needUpdate = true;
					}
					else if (inputKey == '\b' && stringForInputN.length() > 0) {
						NValue /= 10;
						stringForInputN.erase(stringForInputN.length() - 1);
						needUpdate = true;
					}

				}
			}

		}

	}
	return 0;
}
   
