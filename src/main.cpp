#include <iostream>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <deque>

// --- HERE BE CONSTANTS ---

// Window dimension in pixels
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

// Arena size in unit of tiles
const int ARENA_WIDTH = 60;
const int ARENA_HEIGHT = 30;

// Tile size in unit of pixels
const float TILE_SIZE = 10;

// Max frames per second
const int MAX_FPS = 18;
const int MICROSECS_PER_FRAME = 1000000 / MAX_FPS;

// Growth per food tile
const int GROWTH_PER_FOOD = 3;

// --- TYPE DEFINITIONS ---

typedef sf::Vector2i Coord;
typedef std::deque<Coord> SnakeBody;
typedef std::array<std::array<sf::Drawable*, ARENA_WIDTH>, ARENA_HEIGHT> Arena;

enum Direction {
    Up, Right, Down, Left
};

enum GameState {
    Running, Over, Pause
};

// Data structure to represent keyboard input states
typedef struct {
    void reset();
    bool upPressed = false;
    bool leftPressed = false;
    bool downPressed = false;
    bool rightPressed = false;
    bool spaceIsPressed = false;
} InputState;

void InputState::reset() {
    this->upPressed = false;
    this->leftPressed = false;
    this->downPressed = false;
    this->rightPressed = false;
    this->spaceIsPressed = false;
}

// --- STATIC VARS ---

// Game state
GameState gameState = GameState::Running;
int score = 0;

// Arena buffer
Arena arenaBuffer;

// Tile models
sf::RectangleShape emptyTileModel;
sf::RectangleShape snakeBodyTileModel;
sf::RectangleShape foodTileModel;

// Snake body
SnakeBody snake;

InputState inputState;
Direction currentDirection = Direction::Down;
Coord foodPosition;

void clearBuffer(Arena& buffer) {
    for (auto& row: buffer) {
        row.fill(&emptyTileModel);
    }
}

void resetFood(Coord& foodCord) {
    int amountLeft = (ARENA_WIDTH * ARENA_HEIGHT) - snake.size();
    int randomOffset = std::rand() % amountLeft;

    int row, col, count = 0;
    
    for (row = 0; row < ARENA_HEIGHT; ++row) {
        for (col = 0; col < ARENA_WIDTH; ++col) {

            if (arenaBuffer[row][col] != &emptyTileModel) {
                continue;
            }

            if (count < randomOffset) {
                count++;
            }
            else {
                goto food_pos_found;
            }

        }
    }

food_pos_found:
    foodCord.x = col;
    foodCord.y = row;
}

void resetGame() {
    score = 0;
    gameState = GameState::Pause;
    clearBuffer(arenaBuffer);

    snake.clear();
    snake.push_back(Coord(std::rand() % ARENA_WIDTH, std::rand() % ARENA_HEIGHT));
    
    resetFood(foodPosition);
}

int main()
{
    resetGame();
   
    // Initializations....
    emptyTileModel.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    emptyTileModel.setFillColor(sf::Color::White);
    emptyTileModel.setOutlineColor(sf::Color::Black);
    emptyTileModel.setOutlineThickness(1);

    snakeBodyTileModel.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    snakeBodyTileModel.setFillColor(sf::Color::Red);
    snakeBodyTileModel.setOutlineColor(sf::Color::Black);
    snakeBodyTileModel.setOutlineThickness(1);

    foodTileModel.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    foodTileModel.setFillColor(sf::Color::Green);
    foodTileModel.setOutlineColor(sf::Color::Black);
    foodTileModel.setOutlineThickness(1);

    sf::Font font;
    font.loadFromFile("DejaVuSans.ttf");
    sf::Text infoText;
    infoText.setFont(font);
    infoText.setFillColor(sf::Color::Black);
    infoText.setCharacterSize(20);

    sf::RenderWindow window(
        sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
        "Snake Game",
        sf::Style::Titlebar
    );

    // Transformation to be applied to every drawn entity
    sf::Transform baseTransform;

    // Shift to create a 'margin'
    baseTransform.translate(10, 40);

    sf::Clock clock;
    int elapsed = 0;
    int growthCounter = 0;
    int displayScore = score;

    while (window.isOpen()) {

        elapsed += clock.getElapsedTime().asMicroseconds();
        clock.restart();

        // Drawing phase
        window.clear(sf::Color::White);

        // Clear buffer
        clearBuffer(arenaBuffer);
        
        // Place snake on buffer
        for (auto const& tile: snake) {
            arenaBuffer[tile.y][tile.x] = &snakeBodyTileModel;
        }

        // Draw food
        arenaBuffer[foodPosition.y][foodPosition.x] = &foodTileModel;

        // Draw according to the buffer
        for (int row = 0; row < ARENA_HEIGHT; row++) {
            for (int col = 0; col < ARENA_WIDTH; col++) {
                
                // Place tile at its supposed place
                sf::Transform transform;
                transform.combine(baseTransform);
                transform.translate(
                    col * TILE_SIZE,
                    row * TILE_SIZE
                );

                window.draw(*arenaBuffer[row][col], transform);
            }
        }

        // Tween score
        if (displayScore < score) {
            displayScore = displayScore + 10;
        }
        else if (displayScore > score) {
            displayScore = displayScore - 10;
        }

        // --- Draw Info Text
        infoText.setStyle(sf::Text::Style::Bold);
        infoText.setString(
            std::string("SCORE: ") + std::to_string(displayScore)
        );
        sf::Transform textShift;
        textShift.translate(10, 10);
        window.draw(infoText, textShift);

        // --- Game State Updates ---

        switch (gameState) {
            case GameState::Running:

                if (inputState.spaceIsPressed) {
                    gameState = GameState::Pause;
                    break;
                }

                // Handle input states
                if (inputState.upPressed) {
                    if (currentDirection != Direction::Down)
                        currentDirection = Direction::Up;
                }
                else if (inputState.rightPressed) {
                    if (currentDirection != Direction::Left)
                        currentDirection = Direction::Right;
                }
                else if (inputState.downPressed) {
                    if (currentDirection != Direction::Up)
                        currentDirection = Direction::Down;
                }
                else if (inputState.leftPressed) {
                    if (currentDirection != Direction::Right)
                        currentDirection = Direction::Left;
                }

                while (elapsed > MICROSECS_PER_FRAME) {
                    // Update snake position
                    int snakeXUpdate = 0;
                    int snakeYUpdate = 0;
                    switch (currentDirection) {
                        case Up:
                            snakeYUpdate = -1;
                            break;
                        case Right:
                            snakeXUpdate = 1;
                            break;
                        case Down:
                            snakeYUpdate = 1;
                            break;
                        case Left:
                            snakeXUpdate = -1;
                            break;
                    }

                    // Copy the current head
                    auto newHead = *( --snake.end() );
                    
                    // Update the position, make sure the coord doesn't overflow
                    newHead.x = (newHead.x + snakeXUpdate + ARENA_WIDTH) % ARENA_WIDTH;
                    newHead.y = (newHead.y + snakeYUpdate + ARENA_HEIGHT) % ARENA_HEIGHT;

                    // Check if any collision occurs
                    if (arenaBuffer[newHead.y][newHead.x] == &snakeBodyTileModel) {
                        resetGame();
                        break;
                    }

                    // Update body
                    snake.push_back(newHead);
                    
                    if (newHead.x == foodPosition.x && newHead.y == foodPosition.y) {
                        growthCounter = GROWTH_PER_FOOD;
                        score += 100;
                        resetFood(foodPosition);
                    }
                    else {
                        if (growthCounter > 0) {
                            growthCounter--;
                        }
                        else {
                            snake.pop_front();
                        }
                    }

                    elapsed -= MICROSECS_PER_FRAME;
                }

                break;
            case GameState::Over:
                if (inputState.spaceIsPressed) {
                    resetGame();
                }
                break;
            case GameState::Pause:
                if (inputState.spaceIsPressed) {
                    elapsed = 0;
                    gameState = GameState::Running;
                }
                break;
            default:
                break;
        }
        
        // Blit to window
        window.display();

        // Reset input states
        inputState.reset();

        // Handle events
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                // If the user closes the window
                case sf::Event::Closed:
                    window.close();
                    break;
                // Keyboard events
                case sf::Event::KeyPressed:
                    
                    switch (event.key.code) {
                        case sf::Keyboard::Key::Up:
                            inputState.upPressed = true;
                            break;
                        case sf::Keyboard::Key::Right:
                            inputState.rightPressed = true;
                            break;
                        case sf::Keyboard::Key::Down:
                            inputState.downPressed = true;
                            break;
                        case sf::Keyboard::Key::Left:
                            inputState.leftPressed = true;
                            break;
                        case sf::Keyboard::Key::Space:
                            inputState.spaceIsPressed = true;
                        default:
                            break;
                    }

                    break;
                default:
                    break;
            }
        }

    }

    return 0;
}