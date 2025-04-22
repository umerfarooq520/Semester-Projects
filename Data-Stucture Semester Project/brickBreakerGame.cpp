#include <iostream>
#include <vector>
#include <conio.h>     // For _kbhit() and _getch()
#include <windows.h>   // For Sleep(), SetConsoleCursorPosition(), GetStdHandle(), COORD
#include <ctime>       // For seeding random number generator

using namespace std;

// --- Game Settings ---
const int width = 20;
const int height = 20;
const int paddleWidth = 5; // Make paddle width adjustable
const int initialBrickRows = 5; // How many rows of bricks to start with
int gameSpeed = 110; // fps

// Game Variables
bool gameOver;
int score;
int paddleX;            // X position of the left side of the paddle
COORD ballPos;          // Ball's current position {X, Y}
COORD ballDir;          // Ball's direction {-1, 0, 1} for X and Y
vector<vector<bool>> bricks; // true if brick exists, false if broken
int bricksRemaining;

// --- Console Handle ---
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

// --- Function Declarations ---
void Setup();
void Draw();
void Input();
void Logic();
void HideCursor();
void SetCursorPosition(int x, int y); // Helper for setting cursor position

int main() {
    HideCursor();
    Setup();

    while (!gameOver) {
        Draw();
        Input();
        Logic();
        Sleep(gameSpeed);
    }

    // --- Game Over Screen ---
    SetCursorPosition(0, height + 2); // Position cursor below game area
    cout << "=====================" << endl;
    if (bricksRemaining == 0) {
        cout << "      YOU WIN!       " << endl;
    } else {
        cout << "     GAME OVER!      " << endl;
    }
    cout << "   Final Score: " << score << endl;
    cout << "=====================" << endl << endl;

    return 0;
}

// --- Function Definitions ---

void HideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void SetCursorPosition(int x, int y) {
    COORD pos = {(short)x, (short)y};
    SetConsoleCursorPosition(hConsole, pos);
}

void Setup() {
    gameOver = false;
    score = 0;
    srand(time(0));

    // Center the paddle initially
    paddleX = width / 2 - paddleWidth / 2;

    // Start ball above the paddle, moving diagonally up
    ballPos = {(short)(paddleX + paddleWidth / 2), (short)(height - 2)};
    ballDir = {(short)((rand() % 2 == 0) ? 1 : -1), -1}; // Random initial X dir, Y is up

    // Initialize bricks
    bricks.assign(height, vector<bool>(width, false)); // Start all false
    bricksRemaining = 0;
    for (int i = 1; i < initialBrickRows + 1; ++i) { // Start bricks from row 1
        for (int j = 1; j < width - 1; ++j) { // Leave side borders empty
            bricks[i][j] = true;
            bricksRemaining++;
        }
    }
}

void Draw() {
    SetCursorPosition(0, 0); // Move cursor to top-left for redrawing

    // --- Draw Top Border (Optional but nice) ---
    cout << "+";
    for (int i = 0; i < width; ++i) cout << "-";
    cout << "+" << endl;

    // --- Draw Game Area ---
    for (int i = 0; i < height; ++i) {
        cout << "|"; // Left border
        for (int j = 0; j < width; ++j) {
            // Draw Ball
            if (i == ballPos.Y && j == ballPos.X) {
                cout << "*";
            }
            // Draw Paddle
            else if (i == height - 1 && j >= paddleX && j < paddleX + paddleWidth) {
                cout << "="; // Using '=' for paddle looks a bit better
            }
            // Draw Bricks
            else if (bricks[i][j]) {
                cout << "#";
            }
            // Draw Empty Space
            else {
                cout << " ";
            }
        }
        cout << "|" << endl; // Right border
    }

    // --- Draw Bottom Border (Paddle row acts as bottom border) ---
    // We can skip explicit bottom border draw as the loop covers the paddle row

    // --- Draw Score ---aaa
    SetCursorPosition(0, height + 1); // Position cursor below game area
    cout << "Score: " << score << "   Bricks Left: " << bricksRemaining << "   "; // Padding to overwrite previous
}

void Input() {
    if (_kbhit()) { // Check if a key is pressed
        char key = _getch(); // Get the pressed key

        switch (tolower(key)) { // Use tolower for case-insensitivity
            case 'a': // Move Left
                if (paddleX > 0) {
                    paddleX--;
                }
                break;
            case 'd': // Move Right
                if (paddleX < width - paddleWidth) {
                    paddleX++;
                }
                break;
            case 'x': // Exit Game
                gameOver = true;
                break;
        }
    }
}

void Logic() {
    // Calculate potential next position
    COORD nextBallPos = { (short)(ballPos.X + ballDir.X), (short)(ballPos.Y + ballDir.Y) };

    // --- Collision Detection ---

    // 1. Wall Collision (Left/Right)
    if (nextBallPos.X < 0 || nextBallPos.X >= width) {
        ballDir.X *= -1; // Reverse horizontal direction
        nextBallPos.X = ballPos.X + ballDir.X; // Recalculate next X based on new direction
    }

    // 2. Wall Collision (Top)
    if (nextBallPos.Y < 0) {
        ballDir.Y *= -1; // Reverse vertical direction
        nextBallPos.Y = ballPos.Y + ballDir.Y; // Recalculate next Y based on new direction
    }

    // 3. Paddle Collision
    if (nextBallPos.Y == height - 1 && // Is ball at paddle level?
        nextBallPos.X >= paddleX &&    // Is ball horizontally within paddle start?
        nextBallPos.X < paddleX + paddleWidth) // Is ball horizontally within paddle end?
    {
        ballDir.Y = -1; // Always bounce up from paddle
        // Optional: Change ballDir.X based on where it hit the paddle
        // if (nextBallPos.X < paddleX + paddleWidth / 2) ballDir.X = -1; // Hit left side
        // else ballDir.X = 1; // Hit right side
        nextBallPos.Y = ballPos.Y + ballDir.Y; // Recalculate next Y
    }

    // 4. Brick Collision
    // Check if the next position is within the grid bounds where bricks might exist
    if (nextBallPos.Y >= 0 && nextBallPos.Y < height && nextBallPos.X >= 0 && nextBallPos.X < width)
    {
        if (bricks[nextBallPos.Y][nextBallPos.X])
        {
            bricks[nextBallPos.Y][nextBallPos.X] = false; // Break the brick
            bricksRemaining--;
            score += 10;      // Increase score
            ballDir.Y *= -1; // Reverse vertical direction (simplest bounce logic)

            // Check for win condition
            if (bricksRemaining <= 0) {
                gameOver = true;
                return; // Exit logic early on win
            }

             // Recalculate next position based on new direction after bounce
             nextBallPos.Y = ballPos.Y + ballDir.Y;
        }
    }


    // 5. Ball Below Paddle (Game Over)
    if (nextBallPos.Y >= height) {
        gameOver = true;
        return; // Exit logic early on game over
    }

    // Update Ball Position
    ballPos = nextBallPos;
}
