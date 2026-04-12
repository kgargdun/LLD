
#include<iostream>
#include<map>
#include<thread>
#include<chrono>
using namespace std;

struct Position {
    int x, y;

    bool operator< (const Position &other) const {
        if(this->x==other.x) return this->y < other.y;
        else return this->x < other.x;
    }

    bool operator== (const Position &other) const {
        return (this->x==other.x and this->y==other.y);
    }
};

enum class Direction {
    Up, Down, Left, Right
};

enum class Status {
    Progress, Done
};

class Snake;

class Food {
public:
    Food(Position position) : position(position) {

    }
    Position position;
};


class Board {
public:
    Board(int N) {
        this->N = N;
        food = nullptr;
    }

    bool isFood(Position position) {
        if(food and food->position==position) return true;
        return false;
    }

    bool hasFood() {
        return food != nullptr;
    }

    void consumeFood() {
        cout<<"I ate food"<<endl;
        food = nullptr;
    }

    void makeFood(Snake& snake);

    int getBound() {
        return N;
    }

    void show(Snake &snake);

private:
    int N;
    unique_ptr<Food>food;
};




class WaitStrategy {
public:
    virtual void wait() = 0;
    virtual ~WaitStrategy() = default;
};

class SlowStrategy : public WaitStrategy {
public:
    void wait() override {
        std::this_thread::sleep_for(std::chrono::seconds(waitTime));
    }
private:
    int waitTime = 3;
};

class Snake {
public:
    Snake(Position head, Direction currentDirection, unique_ptr<WaitStrategy>waitStrategy) {
        this->currentDirection = currentDirection;
        length = 1;
        this->head = head;
        this->waitStrategy = std::move(waitStrategy);
        maxSize = 20; // can set this
        inDq[head] = true;
        dq.push_back(head);
    }

    void changeDirection(Direction direction) {
        currentDirection = direction;
        cout<<"direction changed!!"<<endl;
    }

    bool inSnake(Position position) {
        return inDq[position];
    }

    Position next() {
        if(currentDirection==Direction::Left) {
            return {head.x,head.y-1};
        } else if(currentDirection == Direction::Down) {
            return {head.x+1, head.y};
        } else if(currentDirection == Direction::Right) {
            return {head.x, head.y+1};
        } else {
            return {head.x-1, head.y};
        }
    }

    bool outOfBound(Board &board, Position position) {
        int N = board.getBound();
        if(position.x<0 or position.y<0 or position.x>=N or position.y>=N) {
            cout<<"boundary collision"<<endl;
            return true;
        }
        return false;
    }

    bool collision(Position position) {
        if(inDq[position] and !(dq.front()==position)) {
            cout<<"self collsion"<<endl;
            return true;
        }
        return false;
    }

    Status forward(Board &board) {
        Position nextHead = next();
        if(outOfBound(board, nextHead)) return Status::Done;
        if(collision(nextHead)) return Status::Done;

        dq.push_back(nextHead);
        inDq[nextHead] = true;
        if(board.isFood(nextHead)) {
            board.consumeFood();
            length++;
        }
        else {
            inDq[dq.front()] = false;
            dq.pop_front();
        }
        head = nextHead;
        waitStrategy->wait();
        if(length==maxSize) return Status::Done;
        return Status::Progress;
    }
private:
    deque<Position>dq;
    map<Position,bool>inDq;
    int length;
    Direction currentDirection;
    Position head;
    unique_ptr<WaitStrategy>waitStrategy;
    int maxSize;
};


class PlayerStratgey {
public:
    virtual Direction getDirection(Board &board, Snake &snake) = 0;
    virtual ~PlayerStratgey() = default;
};

class RandomStrategy : public PlayerStratgey {
public:
    Direction getDirection(Board &board, Snake &snake) override {
        cout<<"Enter direction up(0), down(1), left(2), right(3) ......"<<endl;
        int newDirection = 0;
        cin>>newDirection;
        return static_cast<Direction>(newDirection);
    }
};

class Player {
public:

    Player(unique_ptr<PlayerStratgey> playerStrategy): playerStrategy(std::move(playerStrategy)) {

    }
    Direction getDirection(Board &board, Snake &snake) {
        return playerStrategy->getDirection(board, snake);
    }
private:
    unique_ptr<PlayerStratgey>playerStrategy;
};

void Board::makeFood(Snake &snake) {

    if(food) return;
    bool ok = false;
    while(!ok) {
        int x = rand()%N;
        int y = rand()%N;
        if(snake.inSnake(Position{x,y})) continue;
        food = make_unique<Food>(Position{x,y});
        ok = true;
    }
}

void Board::show(Snake &snake) {

    cout<<endl;
    for(int i=0;i<N;i++) {
        for(int j=0;j<N;j++) {
            if(isFood(Position{i,j})) {
                cout<<"F ";
                continue;
            } else if(snake.inSnake(Position{i,j})) {
                cout<<"S ";
                continue;
            } else cout<<"_ ";
        }
        cout<<endl;
    }
    cout<<endl;
}


class GamePlay {
public:
    GamePlay() {
        player = make_unique<Player>(make_unique<RandomStrategy>());
        board = make_unique<Board>(15);
        snake = make_unique<Snake>(Position{0,0}, Direction::Right, make_unique<SlowStrategy>());
        board->show(*snake);
    }

    void play() {
        
        thread t1(&GamePlay::listener, this);
        status = Status::Progress;
        while(status != Status::Done) {
            board->makeFood(*snake);
            status = snake->forward(*board);
            board->show(*snake);
        }
        t1.join();
        cout<<"Game Over"<<endl;
    }

    void listener() {
        while(status != Status::Done) {
            Direction newDirection = player->getDirection(*board, *snake);
            snake->changeDirection(newDirection);
            this_thread::sleep_for(chrono::seconds(1));
        }
        
    }
private:
    shared_ptr<Player>player;
    unique_ptr<Snake>snake;
    unique_ptr<Board>board;
    Status status;
};

int main() {

    GamePlay fun;
    fun.play();

}

