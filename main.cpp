// Lucas Di Pietro
// SFML - Tic-Tac-Toe
// A Tic-Tac-Toe AI that's impossible to beat, given the first move.
// Will NEVER allow the player to win when given second move.
// i.e. by playing perfectly against the bot, you will ALWAYS draw.

// Milestones:
// Main game design completed night of June 20 - AM June 21.
// Base game logic completed after class ~3:30PM June 21.
// Full logic completed June 21, 2021, 11:32.

// Cleanup in Progress
// Todo:
//     Rewrite more functions to take parameters rather than globals
//     Move functions to an hpp/cpp
//     Some aesthetic changes such as breathing black > grey > black background
//     Functionality such as restart button, etc.

// includes
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <algorithm>
#include <map>

// flags
bool playerTurn = true;
bool musicMode = true;

// early declarations
std::vector<sf::RectangleShape> crossGroup;
std::vector<sf::CircleShape> circleGroup;
std::vector<std::vector<int>> boardState(3);

sf::Color currentColor;

sf::Color black = sf::Color(0, 0, 0, 255);

// functions
void restartGame() {
    boardState = { {0, 0, 0}, {0, 0, 0}, {0, 0, 0} };
    circleGroup = {};
    crossGroup = {};
    currentColor = black;
}

bool isSquareEmpty(int const& xGrid, int const& yGrid) {
    return (boardState[xGrid][yGrid] == 0);
}

std::vector<int> unpackBoardState(std::vector<std::vector<int>> const& a) {
    return { a[0][0], a[1][0], a[2][0], a[0][1], a[1][1], a[2][1], a[0][2], a[1][2], a[2][2] };
}

std::vector<int> checkException(std::vector<std::vector<int>> const& boardState) {
    std::vector<int> bs = unpackBoardState(boardState); // just for convenience
    std::vector<int> case1 = { 2, 0, 0, 0, 1, 0, 0, 0, 1 };
    std::vector<int> case2 = { 1, 0, 0, 0, 1, 0, 0, 0, 2 };
    std::vector<int> case3 = { 0, 0, 2, 0, 1, 0, 1, 0, 0 };
    std::vector<int> case4 = { 0, 0, 1, 0, 1, 0, 2, 0, 0 };
    std::vector<int> case5 = { 0, 0, 1, 0, 2, 0, 1, 0, 0 };
    std::vector<int> case6 = { 1, 0, 0, 0, 2, 0, 0, 0, 1,};
    // cant switchcase :(
    int salt;
    if (bs == case1 || bs == case2) {
        salt = rand() % 2;
        switch (salt) {
            case 0:
                return { 2, 0 };
                break;
            case 1:
                return { 0, 2 };
                break;
            default:
                std::cout << "RandError " << salt << ", " << "1/2" << std::endl;
                break;
        }
    }
    else if (bs == case3 || bs == case4) {
        salt = rand() % 2;
        switch (salt) {
            case 0:
                return { 0, 0 };
                break;
            case 1:
                return { 2, 2 };
                break;
            default:
                std::cout << "RandError " << salt << ", " << "3/4" << std::endl;
                break;
        }
    }
    else if (bs == case5 || bs == case6) {
        salt = rand() % 4;
        switch (salt) {
            case 0:
                return { 2, 1 };
                break;
            case 1:
                return { 1, 2 };
                break;
            case 2:
                return { 0, 1 };
                break;
            case 3:
                return { 1, 0 };
                break;
            default:
                std::cout << "RandError " << salt << ", " << "5/6" << std::endl;
                break;
        }
    }
    return { -1, -1 };
}

std::vector<std::vector<int>> getAllLines(std::vector<int> const& b) {
    return {
        { b[0], b[1], b[2] },
        { b[3], b[4], b[5] },
        { b[6], b[7], b[8] },
        { b[0], b[3], b[6] },
        { b[1], b[4], b[7] },
        { b[2], b[5], b[8] },
        { b[0], b[4], b[8] },
        { b[2], b[4], b[6] },
    };
}

std::vector<std::vector<std::vector<int>>> getMoveDict(std::vector<std::vector<int>> const& c) {
    return {
        {c[0], c[1], c[2]},
        {c[3], c[4], c[5]},
        {c[6], c[7], c[8]},
        {c[0], c[3], c[6]},
        {c[1], c[4], c[7]},
        {c[2], c[5], c[8]},
        {c[0], c[4], c[8]},
        {c[2], c[4], c[6]},
    };
}

std::vector<std::vector<int>> getHypoBoardState(int const& x, int const& y, int const& target) {
    std::vector<std::vector<int>> hb = ::boardState; // hypothetical board
    if (isSquareEmpty(x, y)) {
        hb[x][y] = target;
        return hb;
    }
    return { {-1} };
}

void placeCircle(int const& xGrid, int const& yGrid) {
    sf::CircleShape currentCircle(75, 100);
    currentCircle.setOrigin(currentCircle.getRadius(), currentCircle.getRadius());
    currentCircle.setPosition(100 + (xGrid * 200), 100 + (yGrid * 200));
    currentCircle.setRotation(45);
    ::circleGroup.push_back(currentCircle);
}

int isLineClose(std::vector<int> const& line, int const& target) {
    // -1 = no win possible,
    // 0 = first box,
    // 1 = second box,
    // 2 = third box
    int a = line[0], b = line[1], c = line[2];
    if (a == target && b == target && c == 0) { // theres probably better logic for this but whatever
        return 2;
    }
    else if (a == target && c == target && b == 0) {
        return 1;
    }
    else if (b == target && c == target && a == 0) {
        return 0;
    }
    return -1;
}

std::vector<int> findBestMove(std::vector<std::vector<int>> const& boardState) {
    // -1, -1 if no win
    // coords of win if win available
    std::vector<std::vector<int>> const c = { {0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1}, {2, 1}, {0, 2}, {1, 2}, {2, 2} }; // all coords ordered
    std::vector<int> const b = unpackBoardState(boardState); // unpack state into b
    std::vector<std::vector<int>> const allLines = getAllLines(b); // every possible line on a board, represented as a vector
    std::vector<std::vector<std::vector<int>>> const moveDict = getMoveDict(c); // allLines but pointing to vector c, yes this is a bad practice
    int i = 0;
    int j = 0;
    int winningIdx = -1;
    std::vector<int> winningMove = { -1, -1 };
    // first, find direct wins
    while (i < allLines.size()) {
        std::cout << ".";
        winningIdx = isLineClose(allLines[i], 2);
        if (winningIdx != -1) {
            winningMove = { moveDict[i][winningIdx] };
            return winningMove;
        }
        i++;
    }
    // second, find if the player can be blocked from a win
    while (j < allLines.size()) {
        winningIdx = isLineClose(allLines[j], 1);
        if (winningIdx != -1) {
            winningMove = { moveDict[j][winningIdx] };
            return winningMove;
        }
        j++;
    }
    // third, if the centre is empty, play the centre
    if (isSquareEmpty(1, 1)) {
        return { 1, 1 };
    }

    // finally, return {-1, -1} if no higher prio move was found, ie. select a random move
    return winningMove;
}

std::vector<int> getSecondDepthBestMove() {
    std::vector<std::vector<int>> const c = { {0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1}, {2, 1}, {0, 2}, {1, 2}, {2, 2} };
    std::vector<std::vector<int>> currentBestMoves = {};
    for (int k = 0; k < 9; k++) {
        std::vector<std::vector<int>> hb = (getHypoBoardState(c[k][0], c[k][1], 2)); // = current square
        if (hb[0][0] != -1) {
            std::vector<int> currentMove = findBestMove(hb);
            if (currentMove[0] != -1) {
                currentBestMoves.push_back(currentMove);
            }
        }
    }
    for (int k2 = 0; k2 < 9; k2++) {
        std::vector<std::vector<int>> hb = (getHypoBoardState(c[k2][0], c[k2][1], 1));
        if (hb[0][0] != -1) {
            std::vector<int> currentMove = findBestMove(hb);
            if (currentMove[0] != -1) {
                currentBestMoves.push_back(currentMove);
                currentBestMoves.push_back(currentMove);
            }
        }
    }
    if (currentBestMoves.size() > 0) {
        if (currentBestMoves.size() == 1) {
            return currentBestMoves[0];
        }
        else {
            std::map<std::vector<int>, int> moveMap;
            for (int i = 0; i < currentBestMoves.size(); i++) {
                std::vector<int> currentKey = currentBestMoves[i];
                int count = moveMap.count(currentKey);
                if (count == 0) {
                    moveMap.insert(make_pair(currentKey, 1));
                }
                else {
                    moveMap.erase(currentKey);
                    moveMap.insert(make_pair(currentKey, count + 1));
                }
            }
            std::vector<int> mags = {};
            for (auto const& imap : moveMap) {
                mags.push_back(imap.second);
            }
            std::sort(mags.begin(), mags.end());
            std::reverse(mags.begin(), mags.end());
            int target = mags[0];
            std::cout << "Occurences of best move: " << target << std::endl;
            for (auto const& imap : moveMap) {
                if (imap.second == target) {
                    return imap.first;
                }
            }
        }
    }
    return { -1, -1 };
}

std::vector<int> getAiMove() {
    // find best 1-depth move
    std::cout << "First depth search...";
    std::vector<int> winningMove = findBestMove(boardState);
    if (winningMove[0] != -1) {
        return winningMove;
    }
    // else check if an exception-move is on the board
    winningMove = checkException(boardState);
    if (winningMove[0] != -1) {
        std::cout << "\n*Exception move found. Sorry Noah :)" << std::endl;
        return winningMove;
    }
    // else find best 2-depth move
    std::cout << "\nSecond depth search...";
    winningMove = getSecondDepthBestMove();
    if (winningMove[0] != -1) {
        std::cout << "Found second depth move." << std::endl;
        return winningMove;
    }
    // else find random move
    std::cout << "\nSelecting random move." << std::endl;
    bool moveFound = false;
    while (!moveFound) {
        int x = rand() % 3;
        int y = rand() % 3;
        if (isSquareEmpty(x, y)) {
            return { x, y };
        }
    }
}

int isLineWon(std::vector<int> const& line) {
    int a = line[0], b = line[1], c = line[2];
    if (a == b && b == c) {
        return a;
    }
    return 0;
}

int isGameWon() {
    std::vector<std::vector<int>> const &a = ::boardState; // alias due to repitition in next line
    std::vector<int> const b = { a[0][0], a[1][0], a[2][0], a[0][1], a[1][1], a[2][1], a[0][2], a[1][2], a[2][2] }; // unpack state into b
    std::vector<std::vector<int>> const allLines = {
        {b[0], b[1], b[2]},
        {b[3], b[4], b[5]},
        {b[6], b[7], b[8]},
        {b[0], b[3], b[6]},
        {b[1], b[4], b[7]},
        {b[2], b[5], b[8]},
        {b[0], b[4], b[8]},
        {b[2], b[4], b[6]},
    }; // every possible line on a board, represented as a vector
    int i = 0;
    int winner = 0;
    while (i < allLines.size()) {
        //std::cout << allLines[i][0] << allLines[i][1] << allLines[i][2] << std::endl;
        winner = isLineWon(allLines[i]);
        if (winner != 0) {
            return winner;
        }
        i++;
    }
    return 0;
}

bool isGameDrawn() {
    std::vector<std::vector<int>> const& a = ::boardState; // alias due to repitition in next line
    if (isGameWon() == 0) {
        // std::find is stupid, lol
        if (a[0][0] * a[1][0] * a[2][0] * a[0][1] * a[1][1] * a[2][1] * a[0][2] * a[1][2] * a[2][2] != 0) { //no 0s
            return true;
        }
    }
    return false;
}


void pushBoardState(int const& xGrid, int const& yGrid, int const& player) {
    // 0 = empty
    // 1 = x
    // 2 = o
    boardState[xGrid][yGrid] = player;
}

std::vector<int> getClickLocation(int const& mouseX, int const& mouseY) {
    if (mouseX <= 25 || (mouseX >= 195 && mouseX <= 205) || (mouseX >= 395 && mouseX <= 405) || mouseX >= 575) {
        // clicked on line / oob
        return { -1, -1 };
    }
    else if (mouseY <= 25 || (mouseY >= 195 && mouseY <= 205) || (mouseY >= 395 && mouseY <= 405) || mouseY >= 575) {
        // clicked on line / oob
        return { -1, -1 };
    }
    else {
        // returns grid (not pixel) coordinate of click
        return { mouseX / 200, mouseY / 200 };
    }
}

void placeCross(int const& xGrid, int const& yGrid) {
    sf::RectangleShape currentRect1({ 12, 140 });
    sf::RectangleShape currentRect2({ 12, 140 });
    currentRect1.setOrigin(6, 70);
    currentRect2.setOrigin(6, 70);
    currentRect1.setPosition(100 + (xGrid * 200), 100 + (yGrid * 200));
    currentRect2.setPosition(100 + (xGrid * 200), 100 + (yGrid * 200));
    currentRect1.setRotation(45);
    currentRect2.setRotation(135);
    ::crossGroup.push_back(currentRect1);
    ::crossGroup.push_back(currentRect2);
}

void printBoardState() {
    std::cout << boardState[0][0] << " " << boardState[1][0] << " " << boardState[2][0] << std::endl;
    std::cout << boardState[0][1] << " " << boardState[1][1] << " " << boardState[2][1] << std::endl;
    std::cout << boardState[0][2] << " " << boardState[1][2] << " " << boardState[2][2] << std::endl;
}

// main
int main()
{
    srand(time(NULL));

    boardState = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };

    sf::RenderWindow window(sf::VideoMode(600, 600), "tic-tac-toe!", (sf::Style::Titlebar, sf::Style::Close));

    sf::Color barColor = sf::Color(200, 200, 200, 255);

    sf::Color red = sf::Color(194, 105, 97, 255);
    sf::Color yellow = sf::Color(212, 199, 133, 255);
    sf::Color green = sf::Color(148, 204, 141, 255);
    sf::Color black = sf::Color(0, 0, 0, 255);

    //playerTurn = (rand() % 2);
    bool gameOver = false;

    float barHeight = 580;
    float barWidth = 10;

    sf::RectangleShape leftBar({ barWidth, barHeight });
    leftBar.setOrigin(barWidth / 2, barHeight / 2);
    leftBar.setPosition(200, 300);
    leftBar.setFillColor(barColor);

    sf::RectangleShape rightBar({ barWidth, barHeight });
    rightBar.setOrigin(barWidth / 2, barHeight / 2);
    rightBar.setPosition(400, 300);
    rightBar.setFillColor(barColor);

    sf::RectangleShape topBar({ barHeight, barWidth });
    topBar.setOrigin(barHeight / 2, barWidth / 2);
    topBar.setPosition(300, 200);
    topBar.setFillColor(barColor);

    sf::RectangleShape bottomBar({ barHeight, barWidth });
    bottomBar.setOrigin(barHeight / 2, barWidth / 2);
    bottomBar.setPosition(300, 400);
    bottomBar.setFillColor(barColor);

    if (musicMode) {
        sf::Music squidTheme;
        if (!squidTheme.openFromFile("Assets/squidTheme.ogg")) {
            return -1; // error
        }
        squidTheme.setVolume(10);
        squidTheme.setLoop(true);
        squidTheme.play();
    }

    currentColor = black;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (playerTurn && !gameOver) {
                        std::vector<int> clickBox = getClickLocation(event.mouseButton.x, event.mouseButton.y);
                        if (clickBox[0] != -1) {
                            if (isSquareEmpty(clickBox[0], clickBox[1])) {
                                std::cout << "Player move: " << clickBox[0] << ", " << clickBox[1] << std::endl;
                                pushBoardState(clickBox[0], clickBox[1], 1);
                                placeCross(clickBox[0], clickBox[1]);
                                printBoardState();
                                if (isGameWon() == 1) {
                                    currentColor = green;
                                    gameOver = true;
                                }
                                std::cout << std::endl;
                                playerTurn = false;
                            }
                            else {
                                std::cout << "Square occupied" << std::endl;
                            }
                        }
                        else {
                            std::cout << "Clicked non-square region" << std::endl;
                        }
                    }
                }
            }
        }

        if (isGameDrawn()) {
            currentColor = yellow;
            gameOver = true;
        }

        if (!playerTurn && !gameOver) {
            std::vector<int> aiMove = getAiMove();
            std::cout << "\nAI move: " << aiMove[0] << ", " << aiMove[1] << std::endl;
            pushBoardState(aiMove[0], aiMove[1], 2);
            placeCircle(aiMove[0], aiMove[1]);
            printBoardState();
            if (isGameWon() == 2) {
                currentColor = red;
                gameOver = true;
            }
            std::cout << std::endl;
            playerTurn = true;
        }

        window.clear(currentColor);

        // if gameover do whatever?

        window.draw(leftBar);
        window.draw(rightBar);
        window.draw(topBar);
        window.draw(bottomBar);

        for (int i = 0; i < crossGroup.size(); i++) {
            window.draw(crossGroup[i]);
        }
        for (int i = 0; i < circleGroup.size(); i++) {
            window.draw(circleGroup[i]);
        }

        window.display();
    }

    return 0;

}