#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std;
using namespace sf;

#define SIDE 3
#define CELL_SIZE 150
#define WINDOW_SIZE (SIDE * CELL_SIZE)

#define COMPUTER 'O'
#define HUMAN 'X'
#define EMPTY '*'

struct MoveNode {
    int position;
    char player;
    MoveNode* next;
    MoveNode(int pos, char p) : position(pos), player(p), next(nullptr) {}
};

class MoveHistory {
    MoveNode* head;
public:
    MoveHistory() : head(nullptr) {}
    void addMove(int pos, char player) {
        MoveNode* node = new MoveNode(pos, player);
        node->next = head;
        head = node;
    }
    void undoMove(char board[SIDE][SIDE]) {
        if (!head) return;
        int x = head->position / SIDE;
        int y = head->position % SIDE;
        board[x][y] = EMPTY;
        MoveNode* temp = head;
        head = head->next;
        delete temp;
    }
    ~MoveHistory() {
        while (head) {
            MoveNode* temp = head;
            head = head->next;
            delete temp;
        }
    }
};

struct GameTreeNode {
    char board[SIDE][SIDE];
    GameTreeNode* children[9];
    int childCount;
    int movePos;
    char player;

    GameTreeNode() : childCount(0), movePos(-1), player(EMPTY) {
        for (int i = 0; i < SIDE; i++)
            for (int j = 0; j < SIDE; j++)
                board[i][j] = EMPTY;
        for (int i = 0; i < 9; i++)
            children[i] = nullptr;
    }
};

bool isBoardFull(char board[SIDE][SIDE]) {
    for (int i = 0; i < SIDE; i++)
        for (int j = 0; j < SIDE; j++)
            if (board[i][j] == EMPTY) return false;
    return true;
}

bool hasWon(char board[SIDE][SIDE], char player) {
    for (int i = 0; i < SIDE; i++)
        if (board[i][0] == player && board[i][1] == player && board[i][2] == player) return true;
    for (int j = 0; j < SIDE; j++)
        if (board[0][j] == player && board[1][j] == player && board[2][j] == player) return true;
    if (board[0][0] == player && board[1][1] == player && board[2][2] == player) return true;
    if (board[0][2] == player && board[1][1] == player && board[2][0] == player) return true;
    return false;
}

void buildGameTree(GameTreeNode* node, char currentPlayer) {
    if (hasWon(node->board, COMPUTER) || hasWon(node->board, HUMAN) || isBoardFull(node->board)) return;
    for (int pos = 0; pos < 9; pos++) {
        int x = pos / SIDE, y = pos % SIDE;
        if (node->board[x][y] == EMPTY) {
            GameTreeNode* child = new GameTreeNode();
            for (int i = 0; i < SIDE; i++)
                for (int j = 0; j < SIDE; j++)
                    child->board[i][j] = node->board[i][j];
            child->board[x][y] = currentPlayer;
            child->movePos = pos;
            child->player = currentPlayer;
            node->children[node->childCount++] = child;
            buildGameTree(child, (currentPlayer == COMPUTER) ? HUMAN : COMPUTER);
        }
    }
}

int minimax(char board[SIDE][SIDE], int depth, bool isMax) {
    if (hasWon(board, COMPUTER)) return 10 - depth;
    if (hasWon(board, HUMAN)) return depth - 10;
    if (isBoardFull(board)) return 0;

    int bestVal = isMax ? -1000 : 1000;
    for (int i = 0; i < SIDE; ++i)
        for (int j = 0; j < SIDE; ++j)
            if (board[i][j] == EMPTY) {
                board[i][j] = isMax ? COMPUTER : HUMAN;
                int value = minimax(board, depth + 1, !isMax);
                board[i][j] = EMPTY;
                bestVal = isMax ? max(bestVal, value) : min(bestVal, value);
            }
    return bestVal;
}

GameTreeNode* bestMove(GameTreeNode* root) {
    int bestVal = -1000;
    GameTreeNode* bestChild = nullptr;
    for (int i = 0; i < root->childCount; i++) {
        int val = minimax(root->children[i]->board, 0, false);
        if (val > bestVal) {
            bestVal = val;
            bestChild = root->children[i];
        }
    }
    return bestChild;
}

void deleteGameTree(GameTreeNode* node) {
    for (int i = 0; i < node->childCount; i++)
        deleteGameTree(node->children[i]);
    delete node;
}

void drawGrid(RenderWindow& window) {
    RectangleShape line;
    for (int i = 1; i < SIDE; i++) {
        line.setSize(Vector2f(WINDOW_SIZE, 5));
        line.setFillColor(Color::White);
        line.setPosition(0, i * CELL_SIZE);
        window.draw(line);
    }
    for (int i = 1; i < SIDE; i++) {
        line.setSize(Vector2f(WINDOW_SIZE, 5));
        line.setFillColor(Color::White);
        line.setRotation(90);
        line.setPosition(i * CELL_SIZE, 0);
        window.draw(line);
        line.setRotation(0);
    }
}

void drawMarks(RenderWindow& window, char board[SIDE][SIDE], Font& font) {
    for (int i = 0; i < SIDE; i++) {
        for (int j = 0; j < SIDE; j++) {
            if (board[i][j] != EMPTY) {
                Text text;
                text.setFont(font);
                text.setString(board[i][j]);
                text.setCharacterSize(100);
                text.setFillColor(Color::Red);
                text.setPosition(j * CELL_SIZE + 40, i * CELL_SIZE + 10);
                window.draw(text);
            }
        }
    }
}

int main() {
    char board[SIDE][SIDE];
    for (int i = 0; i < SIDE; i++)
        for (int j = 0; j < SIDE; j++)
            board[i][j] = EMPTY;

    Font font;
    if (!font.loadFromFile("C:/Users/DELL/Documents/Tictactoe/x64/Debug/arial/ARIAL.ttf")) {
        cout << "Font loading failed\n";
        return -1;
    }

    RenderWindow window(VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Tic Tac Toe Minimax");
    MoveHistory history;
    bool humanTurn = true;
    bool gameOver = false;

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();

            if (!gameOver && humanTurn && event.type == Event::MouseButtonPressed) {
                int row = event.mouseButton.y / CELL_SIZE;
                int col = event.mouseButton.x / CELL_SIZE;
                int move = row * SIDE + col;
                if (board[row][col] == EMPTY) {
                    board[row][col] = HUMAN;
                    history.addMove(move, HUMAN);
                    if (hasWon(board, HUMAN) || isBoardFull(board))
                        gameOver = true;
                    else
                        humanTurn = false;
                }
            }

            if (!gameOver && event.type == Event::KeyPressed && event.key.code == Keyboard::U) {
                history.undoMove(board);
                history.undoMove(board);
                humanTurn = true;
                gameOver = false;
            }
        }

        if (!humanTurn && !gameOver) {
            GameTreeNode* root = new GameTreeNode();
            for (int i = 0; i < SIDE; i++)
                for (int j = 0; j < SIDE; j++)
                    root->board[i][j] = board[i][j];
            buildGameTree(root, COMPUTER);

            GameTreeNode* best = bestMove(root);
            if (best) {
                int pos = best->movePos;
                int x = pos / SIDE, y = pos % SIDE;
                board[x][y] = COMPUTER;
                history.addMove(pos, COMPUTER);
                if (hasWon(board, COMPUTER) || isBoardFull(board))
                    gameOver = true;
            }
            deleteGameTree(root);
            humanTurn = true;
        }

        window.clear(Color::Black);
        drawGrid(window);
        drawMarks(window, board, font);

        if (gameOver) {
            Text message;
            message.setFont(font);
            message.setCharacterSize(50);
            message.setFillColor(Color::Green);
            message.setPosition(20, WINDOW_SIZE / 2 - 50);
            if (hasWon(board, HUMAN))
                message.setString("Human Wins!");
            else if (hasWon(board, COMPUTER))
                message.setString("Computer Wins!");
            else
                message.setString("It's a Draw!");
            window.draw(message);
        }

        window.display();
    }
    return 0;
}