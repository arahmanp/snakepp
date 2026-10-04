#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

enum class Direction {
    North,
    East,
    South,
    West
};

enum class GameStatus {
    Running,
    Stopped,
};

int dx[] = {-1, 0, 1, 0};
int dy[] = {0, 1, 0, -1};

struct Cell {
    int x;
    int y;
};

class Space {
private:
    int height;
    int width;
    char background;
    std::vector<std::vector<char>> grid;

public:
    Space() : height(10), width(20), background('.'), 
        grid(height, std::vector<char>(width, background)) {}

    void setSpace(int height, int width, char background) {
        this->height = height;
        this->width = width;
        this->background = background;
        grid.assign(height, std::vector<char>(width, background));
    }

    void setPixelAtCell(Cell position, char c) {
        auto [x, y] = position;
        if(x >= 0 && x < height && y >= 0 && y < width) {
            grid[x][y] = c;
        }
    }

    int getHeight() {
        return height;
    }

    int getWidth() {
        return width;
    }

    void print() {
        for(int i = 0; i < height; i++) {
            for(int j = 0; j < width; j++) {
                std::cout << grid[i][j];
            }
            std::cout << '\n';
        }
    }

    void clear() {
        for(int i = 0; i < height; i++) {
            for(int j = 0; j < width; j++) {
                grid[i][j] = background;
            }
        }
    }
};

class Snake {
private:
    Cell head;
    std::vector<Cell> body;
    Direction currentDirection;
    int length;
    char headSkin;
    char bodySkin;

public:
    Snake() {
        head = {0, 4};
        length = 4;
        headSkin = '$';
        bodySkin = '#';
        currentDirection = Direction::East;

        body.reserve(length - 1);
        for(int i = 0; i < length - 1; i++) {
            body.push_back({
                head.x + dx[(static_cast<int>(currentDirection) + 2) % 4],
                head.y + dy[(static_cast<int>(currentDirection) + 2) % 4],
            });
        }
    }

    void setSnake(Cell head, int length, char headSkin, char bodySkin, Direction direction) {
        this->head = head;
        this->length = length;
        this->headSkin = headSkin;
        this->bodySkin = bodySkin;

        currentDirection = direction;

        body.clear();
        body.reserve(length - 1);
        for(int i = 0; i < length - 1; i++) {
            body.push_back({
                head.x + dx[(static_cast<int>(direction) + 2) % 4],
                head.y + dy[(static_cast<int>(direction) + 2) % 4],
            });
        }
    }

    Cell getHeadPosition() {
        return head;
    }

    std::vector<Cell> getBodyPosition() {
        return body;
    }

    char getHeadSkin() {
        return headSkin;
    }

    char getBodySkin() {
        return bodySkin;
    }
};

class Apple {
private:
    Cell position;
    char texture;

public:
    Apple() : position({9, 19}), texture('@') {}

    void setSnake(Cell position, char texture) {
        this->position = position;
        this->texture = texture;
    }

    Cell getPosition() {
        return position;
    }

    char getTexture() {
        return texture;
    }
};

class Game {
private:
    Space gameSpace;
    Snake snake;
    Apple apple;
    GameStatus status;
    int targetFps;
    int frameTime; // in millisecond

    void clearScreen() {
        std::cout << "\033[2J\033[1;1H";
    }

    void render() {
        gameSpace.setPixelAtCell(snake.getHeadPosition(), snake.getHeadSkin());
        
        for(auto cell : snake.getBodyPosition()) {
            gameSpace.setPixelAtCell(cell, snake.getBodySkin());
        }

        gameSpace.setPixelAtCell(apple.getPosition(), apple.getTexture());
    }

    void print() {
        clearScreen();
        gameSpace.print();
        gameSpace.clear();
    }

    void gameLoop() {
        while(1) {
            render();
            print();
            std::this_thread::sleep_for(std::chrono::milliseconds(frameTime));
        }
    }

public:
    Game() : status(GameStatus::Running), targetFps(10), frameTime(1000 / targetFps) {}

    void setTargetFps(int targetFps) {
        this->targetFps = targetFps;
        frameTime = 1000 / targetFps;
    }

    void run() {
        gameLoop();
    }
};

int main() {
    Game game;

    game.run();

    return 0;
}