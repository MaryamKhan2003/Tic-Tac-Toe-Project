#include <iostream>
using namespace std;

#define SIDE 3
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
        if (!head) {
            cout << "No moves to undo!\n";
            return;
        }
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
    // @brief for rows
    for (int i = 0; i < SIDE; i++)
        if (board[i][0] == player && board[i][1] == player && board[i][2] == player)
            return true;
    // @brief for columns
    for (int j = 0; j < SIDE; j++)
        if (board[0][j] == player && board[1][j] == player && board[2][j] == player)
            return true;
    //@brief for diagonals
    if (board[0][0] == player && board[1][1] == player && board[2][2] == player)
        return true;
    if (board[0][2] == player && board[1][1] == player && board[2][0] == player)
        return true;

    return false;
}

void printBoard(char board[SIDE][SIDE]) {
    cout << "\n";
    for (int i = 0; i < SIDE; i++) {
        for (int j = 0; j < SIDE; j++) {
            cout << " " << board[i][j];
            if (j < SIDE - 1) cout << " |";
        }
        cout << "\n";
        if (i < SIDE - 1) cout << "---|---|---\n";
    }
    cout << "\n";
}


void buildGameTree(GameTreeNode* node, char currentPlayer) {
    if (hasWon(node->board, COMPUTER) || hasWon(node->board, HUMAN) || isBoardFull(node->board))
        return;
    for (int pos = 0; pos < SIDE * SIDE; pos++) {
        int x = pos / SIDE;
        int y = pos % SIDE;
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


int minimax(char board[SIDE][SIDE], int depth, bool isMaximizingPlayer) {
    
    if (hasWon(board, COMPUTER))
        return 10 - depth;  
    if (hasWon(board, HUMAN))
        return depth - 10;  
    if (isBoardFull(board))
        return 0;

    if (isMaximizingPlayer) {//@brief:Computer maximizing player
        int bestVal = -1000;
        for (int i = 0; i < SIDE; ++i) {
            for (int j = 0; j < SIDE; ++j) {
                if (board[i][j] == EMPTY) {
                    board[i][j] = COMPUTER;  
                    int value = minimax(board, depth + 1, false);
                    board[i][j] = EMPTY;     
                    bestVal = max(bestVal, value);
                }
            }
        }
        return bestVal;
    }
    else {
        int bestVal = 1000; // @brief:Human minimizing player
        for (int i = 0; i < SIDE; ++i) {
            for (int j = 0; j < SIDE; ++j) {
                if (board[i][j] == EMPTY) {
                    board[i][j] = HUMAN;  
                    int value = minimax(board, depth + 1, true);
                    board[i][j] = EMPTY;   
                    bestVal = min(bestVal, value);
                }
            }
        }
        return bestVal;
    }
}


GameTreeNode* bestMove(GameTreeNode* root) {
    int bestVal = -1000;
    GameTreeNode* bestChild = nullptr;

    for (int i = 0; i < root->childCount; i++) {
        
        int moveVal = minimax(root->children[i]->board, 0, false);  // @brief:False means Next player is HUMAN after COMPUTER's move

        if (moveVal > bestVal) {
            bestVal = moveVal;
            bestChild = root->children[i];
        }
    }

    return bestChild;
}


// ----- Delete GameTree to avoid memory leak -----
void deleteGameTree(GameTreeNode* node) {
    for (int i = 0; i < node->childCount; i++)
        deleteGameTree(node->children[i]);
    delete node;
}


int main() {
    cout << "***************************************************" << endl;
    cout << "         WELCOME TO Tic-Tac-Toe Game             " << endl;
    cout << "***************************************************" << endl;
    char board[SIDE][SIDE];
    for (int i = 0; i < SIDE; i++)
        for (int j = 0; j < SIDE; j++)
            board[i][j] = EMPTY;

    MoveHistory history;
    bool humanTurn = true;

    while (true) {
        printBoard(board);

        if (hasWon(board, HUMAN)) {
            cout << "You won!\n";
            break;
        }
        if (hasWon(board, COMPUTER)) {
            cout << "Computer won!\n";
            break;
        }
        if (isBoardFull(board)) {
            cout << "It's a draw!\n";
            break;
        }

        if (humanTurn) {
            int move;
            cout << "Enter your move (1-9) or 0 to undo last move: ";
            cin >> move;
            if (move == 0) {
                history.undoMove(board);
                history.undoMove(board);
                humanTurn = true;
                continue;
            }
            move--;
            if (move < 0 || move >= SIDE * SIDE) {
                cout << "Invalid move!\n";
                continue;
            }
            int x = move / SIDE;
            int y = move % SIDE;
            if (board[x][y] != EMPTY) {
                cout << "Cell occupied!\n";
                continue;
            }
            board[x][y] = HUMAN;
            history.addMove(move, HUMAN);
            humanTurn = false;
        }
        else {

            GameTreeNode* root = new GameTreeNode();
            for (int i = 0; i < SIDE; i++)
                for (int j = 0; j < SIDE; j++)
                    root->board[i][j] = board[i][j];
            buildGameTree(root, COMPUTER);

            GameTreeNode* best = bestMove(root);
            if (!best) {
                cout << "No moves left!\n";
                deleteGameTree(root);
                break;
            }
            int pos = best->movePos;
            int x = pos / SIDE;
            int y = pos % SIDE;
            board[x][y] = COMPUTER;
            history.addMove(pos, COMPUTER);
            cout << "Computer placed at position " << pos + 1 << "\n";

            deleteGameTree(root);
            humanTurn = true;
        }
    }
    return 0;
}