#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <termios.h>
#include <sys/select.h>
#include <unistd.h>

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

const int SPACE_DEFAULT_HEIGHT = 10;
const int SPACE_DEFAULT_WIDTH = 20;
const char SPACE_DEFAULT_BG = '.';

const Cell SNAKE_DEFAULT_HEAD_POSITION = {0, 4};
const Direction SNAKE_DEFAULT_DIRECTION = Direction::East;
const int SNAKE_DEFAULT_LENGTH = 4;
const char SNAKE_DEFAULT_HEAD_SKIN = '$';
const char SNAKE_DEFAULT_BODY_SKIN = '#';

const Cell APPLE_DEFAULT_POSITION = {SPACE_DEFAULT_HEIGHT - 1, SPACE_DEFAULT_WIDTH - 1};
const char APPLE_DEFAULT_TEXTURE = '@';

class Space {
private:
    int height;
    int width;
    char background;
    std::vector<std::vector<char>> grid;

public:
    Space() : height(SPACE_DEFAULT_HEIGHT), width(SPACE_DEFAULT_WIDTH), background(SPACE_DEFAULT_BG), 
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
        head = SNAKE_DEFAULT_HEAD_POSITION;
        length = SNAKE_DEFAULT_LENGTH;
        headSkin = SNAKE_DEFAULT_HEAD_SKIN;
        bodySkin = SNAKE_DEFAULT_BODY_SKIN;
        currentDirection = SNAKE_DEFAULT_DIRECTION;

        body.reserve(length - 1);
        Cell prevPosition = head;
        for(int i = 0; i < length - 1; i++) {
            body.push_back({
                prevPosition.x + dx[(static_cast<int>(currentDirection) + 2) % 4],
                prevPosition.y + dy[(static_cast<int>(currentDirection) + 2) % 4],
            });
            prevPosition = body[i];
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
        Cell prevPosition = head;
        for(int i = 0; i < length - 1; i++) {
            body.push_back({
                prevPosition.x + dx[(static_cast<int>(direction) + 2) % 4],
                prevPosition.y + dy[(static_cast<int>(direction) + 2) % 4],
            });
            prevPosition = body[i];
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

    void changeDirection(Direction newDirection) {
        currentDirection = newDirection;
    }

    void move(int spaceHeight, int spaceWidth) {
        for(int i = length - 2; i > 0; i--) {
            body[i] = body[i - 1];
        }

        body[0] = head;

        Cell newHead = {
            (head.x + dx[static_cast<int>(currentDirection)]) % spaceHeight,
            (head.y + dy[static_cast<int>(currentDirection)]) % spaceWidth,
        };

        if(newHead.x < 0) newHead.x += spaceHeight;

        if(newHead.y < 0) newHead.y += spaceWidth;

        head = newHead;
    }
};

class Apple {
private:
    Cell position;
    char texture;

public:
    Apple() : position(APPLE_DEFAULT_POSITION), texture(APPLE_DEFAULT_TEXTURE) {}

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

    bool kbhit() {
        struct timeval tv = {0, 0};
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
    }

    void setNonBlockingMode(bool enable) {
        static struct termios oldt, newt;
        if (enable) {
            tcgetattr(STDIN_FILENO, &oldt);
            newt = oldt;
            // Matikan ICANON (line buffering) dan ECHO (tampilan karakter)
            newt.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        } else {
            // Kembalikan ke pengaturan semula
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        }
    }

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
        while(status == GameStatus::Running) {
            if(kbhit()) {
                char c;
                read(STDIN_FILENO, &c, 1);

                if(c == 'q') status = GameStatus::Stopped;
                else if(c == 'w') snake.changeDirection(Direction::North);
                else if(c == 'd') snake.changeDirection(Direction::East);
                else if(c == 's') snake.changeDirection(Direction::South);
                else if(c == 'a') snake.changeDirection(Direction::West);
            }

            render();
            print();
            snake.move(gameSpace.getHeight(), gameSpace.getWidth());
            std::this_thread::sleep_for(std::chrono::milliseconds(frameTime));
        }
    }

public:
    Game() : status(GameStatus::Stopped), targetFps(10), frameTime(1000 / targetFps) {}

    void setTargetFps(int targetFps) {
        this->targetFps = targetFps;
        frameTime = 1000 / targetFps;
    }

    void run() {
        setNonBlockingMode(true);

        status = GameStatus::Running;

        gameLoop();

        setNonBlockingMode(false);
    }
};

int main() {
    Game game;

    game.run();

    return 0;
}
