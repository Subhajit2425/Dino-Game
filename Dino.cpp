#include <iostream>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <ctime>
#include <fstream>

using namespace std;

// Constants
const int WIDTH = 60;
const int HEIGHT = 20;
const char DINO_CHAR = 'O';
const char OBSTACLE_CHAR = '|';
const char GROUND_CHAR = '=';
const char CLOUD_CHAR = '*';
const char EMPTY_CHAR = ' ';

// Game variables
bool isPaused = false;
char playAgain = 'R';
int dinoX = 5;
int dinoY = HEIGHT - 2;
bool isJumping = false;
int jumpHeight = 6;
int jumpCount = 0;
int score = 0, highScore = 0;
int speed = 100;
int gameNo = 0;
int obstacleRate = 20;
string difficulty = "Easy";

vector<int> obstacles;
vector<pair<int, int>> clouds;

// Hide cursor for smoother output
void hideCursor() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

// Reset cursor to top-left to reduce flicker
void resetCursor() {
    COORD cursorPosition = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
}

void loadHighScore() {
    ifstream fin("_HighScore_Dino.txt");
    if (fin >> highScore) {
        // Successfully loaded
    } else {
        highScore = 0;
    }
    fin.close();
}

void saveHighScore() {
    ofstream fout("_HighScore_Dino.txt");
    fout << highScore;
    fout.close();
}

void deleteLine(int lineNumber) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD startPos = {0, static_cast<SHORT>(lineNumber)};
    DWORD charsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(hConsole, &csbi);
    int consoleWidth = csbi.dwSize.X;

    // Fill the line with spaces
    FillConsoleOutputCharacter(hConsole, ' ', consoleWidth, startPos, &charsWritten);
    // Reset character attributes
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, consoleWidth, startPos, &charsWritten);
    // Move cursor back to start of that line
    SetConsoleCursorPosition(hConsole, startPos);
}

// Draw the game frame
void draw() {
    resetCursor();

    string buffer;
    buffer += " []=[]=[]=[]=[]=[]=[]=[  [ DINO ]  ]=[]=[]=[]=[]=[]=[]=[] \n\n";
    buffer += "  | [Game: " + to_string(gameNo) + "] | [Difficulty: " + difficulty + "]\n";
    for (int i=0; i < WIDTH ; i++) {
        buffer += ':';
    } 
    cout << endl ;

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            bool drawn = false;
            if (x == 1 || x == WIDTH - 2) {
                buffer += ':' ;
                drawn = true ;
            }   

            // Dino
            if (x == dinoX && y == dinoY) {
                buffer += DINO_CHAR;
                drawn = true;
            }

            // Obstacles
            if (!drawn) {
                for (int obsX : obstacles) {
                    if (x == obsX && y == HEIGHT - 2) {
                        buffer += OBSTACLE_CHAR;
                        drawn = true;
                        break;
                    }
                }
            }

            // Clouds
            if (!drawn) {
                for (auto cloud : clouds) {
                    if (x == cloud.first && y == cloud.second) {
                        buffer += CLOUD_CHAR;
                        drawn = true;
                        break;
                    }
                }
            }

            // Ground or empty
            if (!drawn) {
                if (y == HEIGHT - 1) buffer += GROUND_CHAR;
                else buffer += EMPTY_CHAR;
            }
        }
        buffer += '\n';
    }
    buffer += " | Score: " + to_string(score) + " | High Score: " + to_string(highScore) + "\n\n";
    buffer += " | Controls: [Space] -> 'Jump' \n";
    buffer += "           | [ESC] -> 'Quit' \n";
    buffer += "           | [P] -> 'Pause' \n";

    cout << buffer;
}

// Input handler
void input(bool& quit) {
    if (_kbhit()) {
        char ch = _getch();
        if ((ch == ' ' || ch == 'w') && !isJumping && dinoY == HEIGHT - 2 && !isPaused) {
            isJumping = true;
            jumpCount = jumpHeight;
        }
        if (ch == 27) quit = true; // ESC to quit
        if (ch == 'p' || ch == 'P') {
            isPaused = !isPaused;
        }
    }
}

// Game logic
bool logic() {
    // Jumping mechanics
    if (isJumping) {
        dinoY--;
        jumpCount--;
        if (jumpCount <= 0)
            isJumping = false;
    } else if (dinoY < HEIGHT - 2) {
        dinoY++;
    }

    // Move obstacles
    for (int& obs : obstacles) obs--;
    if (!obstacles.empty() && obstacles.front() < 0) {
        obstacles.erase(obstacles.begin());
        score++;
    }
    if (rand() % obstacleRate == 0)
        obstacles.push_back(WIDTH - 2);

    // Move clouds
    for (auto& cloud : clouds) cloud.first--;
    if (rand() % 25 == 0)
        clouds.push_back({WIDTH - 1, rand() % (HEIGHT / 2) + 1});
    if (!clouds.empty() && clouds.front().first < 0)
        clouds.erase(clouds.begin());

    // Collision detection
    for (int obsX : obstacles) {
        if (obsX == dinoX && dinoY == HEIGHT - 2)
            return true; // collision
    }

    return false;
}

void showGameOverScreen() {
    cout << "\n =====================[ GAME  OVER ]===================== \n\n";
    if (score > highScore) {
        cout << " * Congratulations !! New High Score !! \n" ;
        cout << " * Score: " << score << " [High Score] \n" ;
        highScore = score;
        saveHighScore();
    } else {
        cout << " * Score: " << score << endl ;
        cout << " * High Score: " << highScore << "\n" ; 
    }

    cout << "\n ====================[ RESTART GAME ]==================== \n\n";
    cout << " * Press [R] -> 'Restart' \n";
    cout << " *     | [Q] -> 'Quit' \n\n";
    cout << " * Enter Your Choice: " ;

    cin >> playAgain;

    if (playAgain != 'R' && playAgain != 'r') {
        cout << "\n ======================================================== \n\n";
        cout << " * Thank You For Visiting !! \n" ;
        cout << " * Hope You Come Again !! \n\n" ;
        cout << " ======================================================== \n\n";
        cout << " * Press Any Key To Exit: " ;
        _getch();
        exit(0);
    }
}

// Menu and difficulty selection
void showMenu() {
    system("cls");
    cout << " []=[]=[]=[]=[]=[]=[]=[  [ DINO ]  ]=[]=[]=[]=[]=[]=[]=[] \n\n";
    cout << " ====================[ INSTRUCTIONS ]==================== \n\n";
    cout << "   1. Press [Space] -> 'Jump' \n";
    cout << "          | [ESC] -> 'Quit' \n";
    cout << "          | [P] -> 'Pause' \n\n";
    cout << "   2. Avoid Obstacles '|' \n\n";
    
    cout << " =================[ DIFFFICULTY  LEVEL ]================= \n\n";
    cout << " * Press [E] -> Easy \n";
    cout << "       | [M] -> Medium \n";
    cout << "       | [H] -> Hard \n\n";
    cout << " * Enter Your Choice [H or M or E]: ";
    char ch = _getch();

    while (ch != 'e' && ch != 'E' &&
            ch != 'm' && ch != 'M' &&
            ch != 'h' && ch != 'H') {
        cout << "\n * Invalid Level !! Try Again : ";
        ch = _getch();
    }

    cout << "\n\n =====================[ START GAME ]===================== \n\n";
    cout << " * Press Any Key To Start The Game: " ;
    _getch();

    if (ch == 'e' || ch == 'E') {
        speed = 100;
        obstacleRate = 20;
        difficulty = "Easy";
    } else if (ch == 'm' || ch == 'M') {
        speed = 70;
        obstacleRate = 15;
        difficulty = "Medium";
    } else {
        speed = 40;
        obstacleRate = 10;
        difficulty = "Hard";
    }
}

// Main
int main() {
    srand((unsigned)time(0));
    hideCursor();
    loadHighScore();
    showMenu();

    while (playAgain == 'R' || playAgain == 'r') {
        gameNo++;
        score = 0;
        dinoY = HEIGHT - 2;
        isJumping = false;
        obstacles.clear();
        clouds.clear();

        system("cls");
        bool quit = false;

        while (!quit) {
            draw();
            input(quit);
            if (!isPaused) {
                if (logic()) {
                    showGameOverScreen();
                    break;
                }
            } else {
                cout << "\n ========================[ PAUSED ]======================== ";
                cout << "\n\n * Enter Any Key To Resume: " ;
                _getch();
                isPaused = false ;
                deleteLine(30); //  This is to delete the current line
                deleteLine(32);
            }
            Sleep(speed);
        }
    }

    return 0;
}
