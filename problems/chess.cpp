#include<iostream>
using namespace std;

/*
DATA TYPES
---------------------------
*/
#define BOARD_SIZE 8


enum class Color {
    White,
    Black
};

enum class PieceType {
    Pawn,
    Bishop,
    Knight,
    Rook,
    King,
    Queen
};

struct Position {
    int x;
    int y;
};

struct Move {
    Position initial;
    Position next;
    PieceType toPromote;
};

/*
------------------------------
*/


class Board;

/*
Class Player
------------------------------
 */
class PlayerStratgey {
public:
    virtual Move getMove(Board&) = 0;
    virtual ~PlayerStratgey() = default;
};

class HumanStrategy : public PlayerStratgey {
public:
    Move getMove(Board&);
};

class Player {
public:
    Player(unique_ptr<PlayerStratgey>playerStrategy, Color color) {

    }

    Move getMove(Board&) {

    }

    Color getColor() {
        return color;
    }

private:
    unique_ptr<PlayerStratgey> playerStrategy;
    Color color;
};
/*
------------------------------
*/


/*
Class Piece
------------------------------
*/

class MoveStrategy {
public:
    virtual bool canMove(Move &move, Board &board) = 0;
    virtual ~MoveStrategy() = default;
};

class PawnMoveStrategy : public MoveStrategy {
public:
    bool canMove(Move &move, Board &board) override;
};
class BishopMoveStrategy : public MoveStrategy {
public:
    bool canMove(Move &move, Board &board) override {

    }
};
class KnightMoveStrategy : public MoveStrategy {
public:
    bool canMove(Move &move, Board &board) override;
};
class RookMoveStrategy : public MoveStrategy {
public:
    bool canMove(Move &move, Board &board) override {

    }
};
class QueenMoveStrategy : public MoveStrategy {
public:
    bool canMove(Move &move, Board &board) override {

    }
};

class KingMoveStrategy : public MoveStrategy {
public:
    bool canMove(Move &move, Board &board) override {

    }
};

class Piece {
public:

    Piece(PieceType pieceType, Color color) {
        this->pieceType = pieceType;
        isAlive = true;
        this->color = color;
        setStrategy();
    }

    bool canMove(Move &move, Board &board) {
        return moveStrategy->canMove(move, board);
    }

    Color getColor() {
        return color;
    }

    void setType(PieceType pieceType) {
        this->pieceType = pieceType;
        setStrategy();
    }

    void setStrategy() {
        switch(pieceType) {
            case PieceType::Pawn :
                moveStrategy = make_unique<PawnMoveStrategy>();
                break;
            case PieceType::Knight :
                moveStrategy = make_unique<KnightMoveStrategy>();
                break;
            case PieceType::Queen :
                moveStrategy = make_unique<QueenMoveStrategy>();
                break;
            default:
                break;
        }
    }

    PieceType getPieceType() {
        return pieceType;
    }

private:
    bool isAlive;
    unique_ptr<MoveStrategy> moveStrategy;
    PieceType pieceType;
    Color color;
};
/*
------------------------------
*/


/*
Class Board
------------------------------
*/

enum class Status {
    CHECKMATE, STALEMATE, INVALID, NORESULT
};

class Board {
public:
    Board() {
        buffer.assign(BOARD_SIZE, vector<shared_ptr<Piece>>(BOARD_SIZE, nullptr));
        setup();
    }

    void setup() {
        for(int j=0;j<7;j++) {
            buffer[1][j] = make_shared<Piece>(PieceType::Pawn, Color::White);
        }
    }

    shared_ptr<Piece> getPiece(Position p) {
        return buffer[p.x][p.y];
    }

    bool isEmpty(Position p) {
        return buffer[p.x][p.y] == nullptr;
    }

    struct MoveUndo {
        Position from;
        Position to;
        shared_ptr<Piece> moved;
        shared_ptr<Piece> captured;
        Position whiteKingPos;
        Position blackKingPos;
    };

    MoveUndo applyMove(Move &move) {
        Position from = move.initial;
        Position to = move.next;

        auto piece = buffer[from.x][from.y];
        auto captured = buffer[to.x][to.y];

        MoveUndo undo;
        undo.from = from;
        undo.to = to;
        undo.moved = piece;
        undo.captured = captured;
        undo.whiteKingPos = whiteKingPos;
        undo.blackKingPos = blackKingPos;

        if(piece->getPieceType()==PieceType::King) {
            if(piece->getColor()==Color::White) whiteKingPos = to;
            else blackKingPos = to;
        }

        buffer[from.x][from.y] = nullptr;
        buffer[to.x][to.y] = piece;

        applyPromotion(move, piece);

        return undo;
    }

    void undoMove(const MoveUndo &undo) {
        buffer[undo.from.x][undo.from.y] = undo.moved;
        buffer[undo.to.x][undo.to.y] = undo.captured;
        whiteKingPos = undo.whiteKingPos;
        blackKingPos = undo.blackKingPos;
    }

    Status makeMove(Move move, Color color) {
        if(!basicChecks(move, color)) return Status::INVALID;

        MoveUndo undo = applyMove(move);

        if(isSelfKingCheck(color)) {
            undoMove(undo);
            return Status::INVALID;
        }

        if(isCheckmate()) return Status::CHECKMATE;
        if(isStalemate()) return Status::STALEMATE;
        return Status::NORESULT;
    }

    bool basicChecks(Move move, Color color) {
        Position initial = move.initial;
        Position next = move.next;

        auto initialPiece = buffer[initial.x][initial.y];
        auto finalPiece = buffer[next.x][next.y];

        if(!initialPiece) return false;
        if(initialPiece->getColor() != color) return false;
        if(finalPiece && finalPiece->getColor()==color) return false;
        if(!initialPiece->canMove(move, *this)) return false;

        return true;
    }

    void applyPromotion(const Move& move, shared_ptr<Piece> piece) {
        Position next = move.next;
        if(piece->getPieceType()==PieceType::Pawn && (next.x==0 || next.x==7)) {
            piece->setType(move.toPromote);
        }
    }

    bool isSelfKingCheck(Color color) {
        Position kingPos = (color==Color::White) ? whiteKingPos : blackKingPos;

        for(int i=0;i<BOARD_SIZE;i++) {
            for(int j=0;j<BOARD_SIZE;j++) {
                auto p = buffer[i][j];
                if(!p) continue;
                if(p->getColor()==color) continue;

                Move m;
                m.initial = Position{i,j};
                m.next = kingPos;

                if(p->canMove(m, *this)) return true;
            }
        }
        return false;
    }

    bool isCheckmate() {

    }

    bool isStalemate() {

    }

private:
    vector<vector<shared_ptr<Piece>>>buffer;
    Position blackKingPos;
    Position whiteKingPos;
};
/*
------------------------------
*/


class ChessGame {
public:
    ChessGame() {

    }

    void play() {

        while(true) {
            Move move = curPlayer->getMove(*board);
            Status status = board->makeMove(move, curPlayer->getColor());
            if(status == Status::INVALID) {
                continue;
            }

            switchPlayer();
        }
    }

    void switchPlayer() {

    }

private:
    shared_ptr<Player>player1;
    shared_ptr<Player>player2;
    shared_ptr<Player>curPlayer;
    unique_ptr<Board>board;
};



int main() {

}


/*
Strategy implementations
*/

bool PawnMoveStrategy::canMove(Move &move, Board &board) {
    Position from = move.initial;
    Position to = move.next;

    auto piece = board.getPiece(from);
    if(!piece) return false;

    Color color = piece->getColor();

    int dir = (color == Color::White) ? -1 : 1;
    int startRow = (color == Color::White) ? 6 : 1;

    if(to.x == from.x + dir && to.y == from.y && board.isEmpty(to))
        return true;

    if(from.x == startRow &&
       to.x == from.x + 2*dir &&
       to.y == from.y &&
       board.isEmpty(to) &&
       board.isEmpty(Position{from.x + dir, from.y}))
        return true;

    if(to.x == from.x + dir &&
       abs(to.y - from.y) == 1) {
        auto target = board.getPiece(to);
        if(target && target->getColor() != color)
            return true;
    }

    return false;
}

bool KnightMoveStrategy::canMove(Move &move, Board &board) {
    Position from = move.initial;
    Position to = move.next;

    int dx = abs(to.x - from.x);
    int dy = abs(to.y - from.y);

    if(!((dx==2 && dy==1) || (dx==1 && dy==2)))
        return false;

    auto piece = board.getPiece(from);
    auto target = board.getPiece(to);

    if(!piece) return false;
    if(target && target->getColor() == piece->getColor())
        return false;

    return true;
}