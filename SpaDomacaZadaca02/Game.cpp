#include "Game.h"
#include <algorithm>

Game::Game(unsigned int width, unsigned int height, unsigned int cSize) : cellSize(cSize), cols(width / cSize), rows(height / cSize)
{
	this->initWindow();
	font.loadFromFile("fonts/conway.ttf");
	this->initGUI();
	this->initView(width, height);
	this->initGrid();
	this->loadGosper();
	
	std::cout << "Space - start simulation" << std::endl;
	std::cout << "Right/Left arrow key - speedup/slowdown simulation" << std::endl;
}

void Game::pollEvent()
{
	
	while (this->window->pollEvent(this->event))
	{
		switch (this->event.type)
		{
		case sf::Event::Closed:
		{
			this->window->close();
			break;
		}
		case sf::Event::KeyPressed:
		{
			if (this->event.key.code == sf::Keyboard::Right)
			{
				newGen();
				updateInterval = std::max(0.01f, updateInterval - 0.01f); // speed up
				updateIntervalLabel();
			}
			else if (event.key.code == sf::Keyboard::Left)
			{
				updateInterval = std::min(1.0f, updateInterval + 0.01f); // slow down
				updateIntervalLabel();
			}
			else if (this->event.key.code == sf::Keyboard::Space)
			{
				this->isRunning = !this->isRunning;
				clock.restart();
			}
			else if (this->event.key.code == sf::Keyboard::Num1)
			{
				currentTool = Tool::SingleCell;
				toolName = "Single";
			}
			else if (this->event.key.code == sf::Keyboard::Num2)
			{
				currentTool = Tool::Eraser;
				toolName = "Eraser";
			}
			else if (this->event.key.code == sf::Keyboard::Num3)
			{
				currentTool = Tool::Spray;
				toolName = "Spray";
			}
			else if (this->event.key.code == sf::Keyboard::Num4)
			{
				currentTool = Tool::Stamp;
				toolName = "Stamp";
			}
			break;
		}
		case sf::Event::MouseButtonPressed:
		{
			if (!this->isRunning && this->event.mouseButton.button == sf::Mouse::Left)
			{
				sf::Vector2i pixelPos{ event.mouseButton.x, event.mouseButton.y };
				sf::Vector2f worldPos = window->mapPixelToCoords(pixelPos, view);
				sf::Vector2f uiPos = window->mapPixelToCoords(pixelPos, uiView);
				if (clearButton.getGlobalBounds().contains(uiPos)) {
					clearGrid();
				}
				else
				{
					drawing = true;
					int Wx = static_cast<int>(worldPos.x) / cellSize;
					int Wy = static_cast<int>(worldPos.y) / cellSize;
					useTool(currentTool, Wx, Wy);
				}
			}
			else if (this->event.mouseButton.button == sf::Mouse::Middle) {
				panning = true;
				lastMouse = { event.mouseButton.x, event.mouseButton.y };
			}
			break;
		}
		case sf::Event::MouseButtonReleased:
		{
			if (event.mouseButton.button == sf::Mouse::Middle) panning = false;
			else if (event.mouseButton.button == sf::Mouse::Left) drawing = false;
			break;
		}
		case sf::Event::MouseMoved:
		{
			if (panning) {
				sf::Vector2i mouse(event.mouseMove.x, event.mouseMove.y);
				panningEvent(mouse);
			}
			else if (drawing) {
				sf::Vector2f worldPos = window->mapPixelToCoords({ event.mouseMove.x, event.mouseMove.y });
				int Wx = static_cast<int>(worldPos.x) / cellSize;
				int Wy = static_cast<int>(worldPos.y) / cellSize;
				useTool(currentTool, Wx, Wy);
			}
			break;
		}
		case sf::Event::MouseWheelScrolled:
		{
			const float zoomAmount = event.mouseWheelScroll.delta > 0 ? 0.9f : 1.1f;
			zoomEvent(zoomAmount);
			break;
		}
		case sf::Event::Resized:
		{
			updateViewSize(event.size.width, event.size.height);
			break;
		}
		default:
			break;
		}
	}
}

void Game::Update()
{
	int alive = countAliveCells();
	aliveLabel.setString("Alive: " + std::to_string(alive));
	toolLabel.setString("Tool: " + toolName);

	this->pollEvent();
	if (this->isRunning && this->clock.getElapsedTime().asSeconds() >= this->updateInterval)
	{
		this->newGen();
		this->clock.restart();
	}
	
}

void Game::Render()
{
	this->window->setView(view);
	this->window->clear(sf::Color(0, 0, 0, 255));
	//draw
	aliveCellsVA.clear();
	for (unsigned int y = 0; y < rows; ++y)
	{
		for (unsigned int x = 0; x < cols; ++x)
		{
			if (!grid[y][x].alive) continue;
			float px = static_cast<float>(x) * cellSize;
			float py = static_cast<float>(y) * cellSize;
			sf::Color color = grid[y][x].color;
			// Append quad
			aliveCellsVA.append({ {px,          py},          color });
			aliveCellsVA.append({ {px + float(cellSize), py},          color });
			aliveCellsVA.append({ {px + float(cellSize), py + float(cellSize)}, color });
			aliveCellsVA.append({ {px,          py + float(cellSize)}, color });
		}
	}
	this->window->draw(gridLines);
	this->window->draw(aliveCellsVA);

	
	window->setView(uiView);
	this->window->draw(clearButton);
	this->window->draw(clearButtonText);

	this->window->draw(intervalLabel);
	this->window->draw(aliveLabel);
	this->window->draw(toolLabel);

	window->setView(view);
	this->window->display();
}

void Game::initWindow()
{
	this->window = new sf::RenderWindow(sf::VideoMode(800, 600), "Conway", sf::Style::Titlebar | sf::Style::Close);
	this->window->setFramerateLimit(60);
}

void Game::initGUI()
{
	// --- Clear Grid Button ---
	clearButton.setSize({ 140.f, 30.f });
	clearButton.setPosition({ 10.f, 10.f });
	clearButton.setFillColor(sf::Color(100, 100, 100)); // dark gray

	clearButtonText.setFont(font);
	clearButtonText.setString("Clear Grid");
	clearButtonText.setCharacterSize(16);
	clearButtonText.setFillColor(sf::Color::White);
	clearButtonText.setPosition(clearButton.getPosition().x + 10, clearButton.getPosition().y + 5);

	// --- Time Interval Label --
	intervalLabel.setFont(font);
	intervalLabel.setCharacterSize(20);
	intervalLabel.setFillColor(sf::Color::White);
	intervalLabel.setPosition(10.f, 60.f);
	
	updateIntervalLabel(); // Update text initially


	// --- Alive Cells Label ---
	aliveLabel.setFont(font);
	aliveLabel.setCharacterSize(20);
	aliveLabel.setFillColor(sf::Color::White);
	aliveLabel.setPosition(10, 80);

	// --- Current Tool Label ---
	toolLabel.setFont(font);
	toolLabel.setCharacterSize(20);
	toolLabel.setFillColor(sf::Color::White);
	toolLabel.setPosition(10, 100);
}


void Game::initGrid()
{
	this->grid.assign(this->rows, std::vector<Cell>(cols));
	this->bufferGrid = this->grid;
	aliveCellsVA.setPrimitiveType(sf::Quads);
	gridLines.setPrimitiveType(sf::Lines);

	view.setSize(window->getDefaultView().getSize());
	view.setCenter(static_cast<float>(cols * cellSize) / 2.f, static_cast<float>(rows * cellSize) / 2.f);
	window->setView(view);
	for (unsigned int x = 0; x <= cols; ++x)
	{
		float px = x * cellSize;
		gridLines.append({ {px, 0.f},                             sf::Color(100,100,100, 64) });
		gridLines.append({ {px, float(rows * cellSize)},         sf::Color(100,100,100, 64) });
	}
	for (unsigned int y = 0; y <= rows; ++y)
	{
		float py = y * cellSize;
		gridLines.append({ {0.f, py},                             sf::Color(100,100,100, 64) });
		gridLines.append({ {float(cols * cellSize), py},         sf::Color(100,100,100, 64) });
	}
}

void Game::clearGrid()
{
	for (auto& row : grid)
	{
		for (auto& cell : row)
		{
			cell.alive = false;
			cell.age = 0;
			cell.color = sf::Color(255, 255, 255, 255);
		}
	}
	bufferGrid = grid;
	counter = 0;
	isRunning = false;
}

void Game::initView(unsigned int width, unsigned int height)
{
	sf::View uiView(sf::FloatRect(0, 0, window->getSize().x, window->getSize().y));
	uiView = window->getDefaultView();
	view.setSize(static_cast<float>(width), static_cast<float>(height));
	view.setCenter(static_cast<float>(width) / 2.f, static_cast<float>(height) / 2.f);
	window->setView(view);
}

void Game::newGen()
{
	for (unsigned int y = 0; y < this->rows; ++y)
	{
		for (unsigned int x = 0; x < this->cols; ++x)
		{
			Cell& c = bufferGrid[y][x];
			int n = this->countNeighbours(y, x);
			bool nextStateCheck = this->grid[y][x].alive ? (n == 2 || n == 3) : (n == 3);
			c.alive = nextStateCheck;
			
			c.age = nextStateCheck ? (grid[y][x].age + 1) : 0;
			
			
			if (c.age < 253) {
				c.color = sf::Color
				(
					sf::Uint8(255 - c.age),
					sf::Uint8(255),
					sf::Uint8(255 - c.age)
				);
			}
			
			
			/*	store in bufferGrid until loop is done. grid[y][x] is current cell.
			* if current cell is bool true, set boolean values of (n=2 or 3) to bufferGrid(when alive, chance to alive, or dead)
			* else if current cell is false, set boolean valuse of n=3 (when dead, chance to revive)
			*/
		}
	}
	grid.swap(bufferGrid);
	counter++;
	window->setTitle("Conway Iteration #: " + std::to_string(counter)); //iteration counter
}

int Game::countNeighbours(int y, int x) const
{
	int count = 0;
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx) //checks 3i*3j field
		{
			if (dy == 0 && dx == 0) continue; //dont count self
			int ny = y + dy; //shifts look to right(when i=1) or left(when i=-1) on y coord
			int nx = x + dx; //shifts look to right or left on x coord
			if (ny >= 0 && ny < static_cast<int>(this->rows) &&
				nx >= 0 && nx < static_cast<int>(this->cols)) //checks if in bounds of window(in corners of screen)
			{
				count += this->grid[ny][nx].alive; //looks at grid coordinates, bool implicitly convert to int 1 or int 0, then adds
			}
		}
	}
	return count;
}

void Game::useTool(Tool tool, int x, int y)
{
	switch (tool)
	{
	case Tool::SingleCell:
		toggleCell(x, y, true);
		break;
	case Tool::Eraser:
		toggleCell(x, y, false);
		break;
	case Tool::Spray:
		
		for (int dy = -3; dy <= 3; ++dy)
			for (int dx = -3; dx <= 3; ++dx)
				if (rand() % 100 < 40) // 30% density
					toggleCell(x + dx, y + dy, true);
		break;

	case Tool::Rectangle:
		dragStart = { x, y };
		break;

	case Tool::Stamp:
		
		for (auto& p : stampPattern) {
			int gx = x + p.x;
			int gy = y + p.y;
			if (gx >= 0 && gx < static_cast<int>(cols) && gy >= 0 && gy < static_cast<int>(rows))
				grid[gy][gx].alive = true;
		}

		break;
	}
}

void Game::toggleCell(int x, int y, bool state)
{
	
	if (x >= 0 && x < static_cast<int>(cols) && y >= 0 && y < static_cast<int>(rows))
	{
		Cell& cell = grid[y][x]; // & accesses the real cell instead of copy 
		cell.alive = state;
	}
}

void Game::panningEvent(sf::Vector2i mousePos)
{
	sf::Vector2f delta = window->mapPixelToCoords(lastMouse) - window->mapPixelToCoords(mousePos);
	view.move(delta);
	window->setView(view);
	lastMouse = mousePos;
}

void Game::zoomEvent(float zoom)
{
	float newZoom = currentZoom * zoom;
	if (newZoom >= minZoom && newZoom <= maxZoom) {
		view.zoom(zoom);
		currentZoom = newZoom;
		window->setView(view);
	}
}

void Game::updateViewSize(unsigned int width, unsigned int height)
{
	view.setSize(static_cast<float>(width), static_cast<float>(height));
	window->setView(view);
}

const bool Game::getWindowIsOpen() const
{
	return this->window->isOpen();
}

int Game::countAliveCells() const {
	int sum = 0;
	for (auto const& row : grid)
		for (auto const& c : row)
			sum += c.alive;
	return sum;
}

void Game::updateIntervalLabel()
{
	intervalLabel.setString("Interval: " + std::to_string(updateInterval) + " ms");
}

Game::~Game()
{
	delete this->window;
}

void Game::loadGosper()
{
	stampPattern.clear();
	// coordinates taken from a standard Gosper glider gun definition
	int pattern[][2] = {
		{0,4},{0,5},{1,4},{1,5},
		{10,4},{10,5},{10,6},{11,3},{11,7},{12,2},{12,8},{13,2},{13,8},{14,5},
		{15,3},{15,7},{16,4},{16,5},{16,6},{17,5},
		{20,2},{20,3},{20,4},{21,2},{21,3},{21,4},{22,1},{22,5},{24,0},{24,1},
		{24,5},{24,6},
		{34,2},{34,3},{35,2},{35,3}
	};
	for (auto& p : pattern)
		stampPattern.emplace_back(p[0], p[1]);
}