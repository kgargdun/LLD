#include<iostream>
#include<vector>
#include<memory>
using namespace std;

struct Position {
    int x;
    int y;
};

enum class Symbol {
    O , X , Empty
};

char symbolToString(Symbol s) {
    if(s==Symbol::O) return 'O';
    else if(s==Symbol::X) return 'X';
    else return '_';
}

enum class Status {
    INVALID, DRAW, WIN , NORESULT
};

class Board; 

class WinStrategy {
public:
    virtual Status hasWon(Position pos, Symbol s, Board &board) = 0;
    virtual ~WinStrategy() = default;
};

class TicToeStrategy : public WinStrategy {
public:
    Status hasWon(Position pos, Symbol s, Board &board) override;
};

class Board {
public:
    Board(int n) : n(n) {
        buffer.assign(n,vector<Symbol>(n,Symbol::Empty));
        rem = n*n;
        winStrategy = make_unique<TicToeStrategy>();
    }

    Status makeMove(Position pos, Symbol symbol) {
        if(!canMove(pos)) return Status::INVALID;
        buffer[pos.x][pos.y] = symbol;
        rem--;
        show();
        if(!rem) return Status::DRAW;
        return winStrategy->hasWon(pos,symbol,*this);
    }

    bool canMove(Position pos) {
        if(pos.x<0 or pos.y<0 or pos.x>=n or pos.y>=n) return false;
        return buffer[pos.x][pos.y] == Symbol::Empty;
    }

    Symbol getPosition(Position pos) {
        return buffer[pos.x][pos.y]; 
    }

    int size() const { return n; }

    void show() {
        cout<<endl;
        for(int i=0;i<n;i++) {
            for(int j=0;j<n;j++) {
                cout<<symbolToString(buffer[i][j])<<" ";
            }
            cout<<endl;
        }
        cout<<endl;
    }

private:
    int n;
    int rem;
    vector<vector<Symbol>>buffer;
    unique_ptr<WinStrategy> winStrategy;
};

Status TicToeStrategy::hasWon(Position pos, Symbol s, Board &board) {
    int n = board.size();

    bool check = true;
    for(int i=0;i<n;i++)
        if(board.getPosition({pos.x,i})!=s) { check=false; break; }
    if(check) return Status::WIN;

    check = true;
    for(int i=0;i<n;i++)
        if(board.getPosition({i,pos.y})!=s) { check=false; break; }
    if(check) return Status::WIN;

    if(pos.x==pos.y) {
        check = true;
        for(int i=0;i<n;i++)
            if(board.getPosition({i,i})!=s) { check=false; break; }
        if(check) return Status::WIN;
    }

    if(pos.x+pos.y==n-1) {
        check = true;
        for(int i=0;i<n;i++)
            if(board.getPosition({i,n-i-1})!=s) { check=false; break; }
        if(check) return Status::WIN;
    }

    return Status::NORESULT;
}

class PlayerStratgey {
public:
    virtual Position getMove(Board &board) = 0;
    virtual ~PlayerStratgey() = default;
};

class HumanStrategy : public PlayerStratgey {
public:
    Position getMove(Board &board) override {
        Position pos;
        cout<<"cur board is " <<endl;
        board.show();
        cin>>pos.x>>pos.y;
        return pos;
    }
};

class Player {
public:
    Player(unique_ptr<PlayerStratgey> strategy, Symbol symbol) : 
    strategy(move(strategy)), symbol(symbol) {}

    void setStrategy(unique_ptr<PlayerStratgey> strategy) {
        this->strategy = move(strategy);
    }

    Position getMove(Board &board) {
        cout<<"getting player move for player "<<symbolToString(symbol)<<endl;
        return strategy->getMove(board);
    }

    Symbol getSymbol() {
        return symbol;
    }

private:    
    unique_ptr<PlayerStratgey> strategy;
    Symbol symbol;
};

class TicTacToeGame {
public:
    TicTacToeGame(int dim, shared_ptr<Player>player1, shared_ptr<Player> player2) {
        board = make_shared<Board>(dim);
        this->player1 = player1;
        this->player2 = player2;
        curPlayer = player1;
    }

    void switchPlayer() {
        curPlayer = (curPlayer==player1) ? player2 : player1;
    }

    void play() {
        while(true) {
            auto move = curPlayer->getMove(*board);
            Status status = board->makeMove(move, curPlayer->getSymbol());

            if(status==Status::INVALID) {
                cout<<"invalid move"<<endl;
                continue;
            }
            if(status==Status::DRAW) {
                cout<<"game draw"<<endl;
                board->show();
                break;
            }
            if(status==Status::WIN) {
                cout<<"winner is "<<symbolToString(curPlayer->getSymbol())<<endl;
                board->show();
                break;
            }
            switchPlayer();
        }
    }

private:
    shared_ptr<Board> board;
    shared_ptr<Player> player1, player2;
    shared_ptr<Player> curPlayer;
};

int main() {
    auto player1 = make_shared<Player>(make_unique<HumanStrategy>(), Symbol::O);
    auto player2 = make_shared<Player>(make_unique<HumanStrategy>(), Symbol::X);
    TicTacToeGame game(3, player1, player2);
    game.play();
}
