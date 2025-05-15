#pragma once
#include <iostream>
#include <algorithm>
#include <sstream>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>

enum class Tool
{
	SingleCell,
	Eraser,
	Spray,
	Rectangle,
	Stamp
};
struct Cell
{
	bool alive = false;
	Cell() = default;
	float age = 0;
	sf::Color color = sf::Color(255, 255, 255, 255);

	//initialize 
	//Cell(float size, float x, float y) : alive(false) {
		//shape.setSize({ size - 1.f, size - 1.f }); //size -1.f, for clear grid pattern
		//shape.setPosition(x + 0.5f, y + 0.5f);
		//shape.setFillColor(sf::Color::Black);
		//shape.setOutlineThickness(0.5f);
		//shape.setOutlineColor(sf::Color(255, 255, 0, 64));
	//}
};

class Game
{
public:
	Game(unsigned int width = 3200, unsigned int height = 2400, unsigned int cellSize = 10);
	virtual ~Game();
	const bool getWindowIsOpen() const;

	//Functions
	void Update();
	void Render();
private:
	//grid	
	const unsigned int cols;
	const unsigned int rows;
	const unsigned int cellSize;
	std::vector<std::vector<Cell>> grid;
	std::vector<std::vector<Cell>> bufferGrid;
	void clearGrid();

	//window
	sf::RenderWindow* window; //dynamic alloc because of destructor
	sf::VideoMode videoMode;
	sf::Event event;

	//grid view == zoom/pan
	sf::View view;
	float currentZoom = 1.f;
	const float minZoom = 0.2f; // 180% zoom in
	const float maxZoom = 4.0f; // 250% zoom out
	sf::VertexArray	aliveCellsVA; //batch draw alive cells
	sf::VertexArray gridLines;
	
	//panning
	sf::Vector2i lastMouse;
	bool panning = false;
	bool drawing = false;

	//painting tools
	Tool currentTool = Tool::SingleCell;
	sf::Vector2i dragStart;     // for Rectangle tool
	std::vector<sf::Vector2i> stampPattern;

	//patterns
	void loadGosper();

	//core
	void initWindow();
	void initGrid();
	void initView(unsigned int width, unsigned int height);
	void pollEvent();

	//clock
	sf::Clock clock;	
	float updateInterval = 0.1f;

	//core game functions
	bool isRunning = false; //if game logic is being applied
	void newGen();
	int countNeighbours(int y, int x) const;
	
	//game functions
	void toggleCell(int x, int y, bool state);
	void useTool(Tool tool, int x, int y);
	std::string toolName = "Single";
	void panningEvent(sf::Vector2i mousePos);
	void zoomEvent(float zoomAmount);
	void updateViewSize(unsigned int width, unsigned int height);

	//GUI
	sf::View uiView;
	sf::Font font;
	
	void initGUI();

	int  countAliveCells() const;
	void updateIntervalLabel();
	sf::Text intervalLabel;
	sf::Text aliveLabel;
	sf::Text toolLabel;

	sf::RectangleShape clearButton;
	sf::Text clearButtonText;

	//misc
	unsigned int counter = 0; //iteration counter
};

/*
kod veceg broja celija stvorenih koristen je VertexArray kako
bi se crtalo vise celija od jednom i premjestio load na GPU
*/