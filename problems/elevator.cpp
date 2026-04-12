#include<iostream>
#include<set>
#include<map>
#include<queue>
#include<thread>




// --------------------------------------------------
enum class Direction {
    Up, Down, Idle
};

// --------------------------------------------------



// --------------------------------------------------
class Monitor {
public:
    static void update(int curTime) {
        std::cout<<"Cur time is "<<curTime<<std::endl;
    }
    static void onStop(int elevator, int floor) {
        std::cout<<"Elevator "<<elevator<<" stopped at floor "<<floor<<std::endl;
        std::cout<<std::endl;
    }

    static void status(int elevator, int floor, Direction direction) {
      //  std::cout<<"Elevator "<<elevator<<" currenty at floor "<<floor<<" direction is "<<(int)direction<<std::endl;
    }

    static void space() {
        std::cout<<std::endl;
    }
};
// --------------------------------------------------





// --------------------------------------------------
struct Request {
    int source;
    Direction direction;

};

struct RequestIncreasing {
    bool operator() (const Request &r1, const Request &r2) const {
        return r1.source > r2.source;
    }
};

struct RequestDecreasing  {
    bool operator() (const Request &r1, const Request &r2) const {
        return r1.source < r2.source;
    }
};


struct RequestQueue{
    void enqueRequest(Request request) {
        if(request.direction==Direction::Up) {
            upRequests.push(request);
        } else {
            downRequests.push(request);
        }
    }
    bool isPending() {
        return !upRequests.empty() or !downRequests.empty();
    }
    std::priority_queue<Request, std::vector<Request>, RequestIncreasing> upRequests;
    std::priority_queue<Request, std::vector<Request>, RequestDecreasing> downRequests;
};
// --------------------------------------------------



class ElevatorSystem;
class Elevator;
class ElevatorPanel {
public:

    ElevatorPanel(ElevatorSystem *elevatorSystem, Elevator *elevator) : elevatorSystem(elevatorSystem), elevator(elevator)  {

    }

    void press(int floorId);
private:
    ElevatorSystem *elevatorSystem;
    Elevator *elevator;

};

class Elevator {
public:

    Elevator(ElevatorSystem *elevatorSystem, int id) : id(id), elevatorSystem(elevatorSystem) {
        curDirection = Direction::Idle;
        curFloor = 0;
        elevatorPanel = std::make_unique<ElevatorPanel>(elevatorSystem, this);
    }

    void step() {
        if(curDirection==Direction::Up) {
            curFloor++;
        } else if(curDirection==Direction::Down) {
            curFloor--;
        }
        if(stopAt.find(curFloor)!= stopAt.end()) {
            stop();
            stopAt.erase(curFloor);
            adjustDirection();
        }
        Monitor::status(id, curFloor, curDirection);

    }

    void stop() {
        Monitor::onStop(id, curFloor);
    }

    Direction getDirection() {
        return curDirection;
    }

    void addStop(int floorId) {
        if(curFloor == floorId) {
            stop();
            return;
        }
        stopAt.insert(floorId);
        adjustDirection();
    }

    void adjustDirection() {
        if(stopAt.empty()) setDirection(Direction::Idle);
        else if((curDirection==Direction::Up  or curDirection==Direction::Idle) and *stopAt.rbegin()<curFloor) {
            setDirection(Direction::Down);
        } else if((curDirection==Direction::Down or curDirection==Direction::Idle) and  *stopAt.begin()>curFloor) {
            setDirection(Direction::Up);
        }
    }

    void setDirection(Direction direction) {
        curDirection = direction;
    }

    int getCurFloor() {
        return curFloor;
    }

    ElevatorPanel *getPanel() {
        return elevatorPanel.get();
    }


private:
    Direction curDirection;
    std::set<int>stopAt;
    int id;
    int curFloor;
    std::unique_ptr<ElevatorPanel>elevatorPanel;
    ElevatorSystem *elevatorSystem;
};
// --------------------------------------------------





// --------------------------------------------------

class Floor;
class FloorPanel {
public:

    FloorPanel(ElevatorSystem *elevatorSystem, Floor *floor) : elevatorSystem(elevatorSystem), floor(floor) {

    }
    void pressUp();
    void pressDown();
private:
    ElevatorSystem *elevatorSystem;
    Floor *floor;

};

class Floor {
public:

    Floor(ElevatorSystem *elevatorSystem,
        int id)
        : elevatorSystem(elevatorSystem), id(id) {
            this->elevatorSystem = elevatorSystem;
            floorPanel = std::make_unique<FloorPanel>(elevatorSystem, this);
    }
    FloorPanel *getPanel() {
        return floorPanel.get();
    }

    int getId() {
        return id;
    }


private:
    int id;
    std::unique_ptr<FloorPanel>floorPanel;
    ElevatorSystem *elevatorSystem;

};
// --------------------------------------------------



// --------------------------------------------------
class AllotmentStrategy {
public:
    virtual ~AllotmentStrategy() = default;

    virtual void assign(std::vector<std::unique_ptr<Elevator>>&elevators, RequestQueue *requestQueue) = 0;
    
};

class ScanStrategy : public AllotmentStrategy {
public:
    void assign(std::vector<std::unique_ptr<Elevator>>&elevators, RequestQueue *requestQueue) {

        auto& up = requestQueue->upRequests;
        for(const auto &elevator:elevators) {
            while(!up.empty() and 
                (elevator->getCurFloor()<up.top().source and elevator->getDirection()==Direction::Up) 
                or elevator->getDirection()==Direction::Idle) {
                elevator->addStop(up.top().source);
                up.pop();
            }
            if(up.empty()) break;
        }

        auto& down = requestQueue->downRequests;
        for(const auto &elevator:elevators) {
            while(!down.empty() and 
                (elevator->getCurFloor()>down.top().source and elevator->getDirection()==Direction::Down)
                or elevator->getDirection()==Direction::Idle) {
                elevator->addStop(down.top().source);
                down.pop();
            }
            if(down.empty()) break;
        }        
    }

};
// --------------------------------------------------



// --------------------------------------------------
class ElevatorSystem {
public:
    ElevatorSystem(int elevatorCount, int floorCount, std::unique_ptr<AllotmentStrategy>allotmentStrategy) {
        for(int i=0;i<elevatorCount;i++) {
            elevators.push_back(std::make_unique<Elevator>(this, i));
        }
        for(int i=0;i<floorCount;i++) {
            floors.push_back(std::make_unique<Floor>(this, i));
        }

        requestQueue = std::make_unique<RequestQueue>();

        this->allotmentStrategy = std::move(allotmentStrategy);
    }

    void start() {
        worker = std::thread(&ElevatorSystem::runAll, this, 100);
    }

    void end() {
        worker.join();
    }

    void runAll(int time) {

        int curTime = 0;
        while(curTime<time) {
            Monitor::space();
            Monitor::update(curTime);
            if(requestQueue->isPending()) {
                allotmentStrategy->assign(elevators, requestQueue.get());
            } 
            for(const auto &elevator : elevators) {
                elevator->step();  
            }
            curTime++;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    }

    Floor *getFloor(int num) {
        return floors[num].get();
    }

    Elevator *getElevator(int num) {
        return elevators[num].get();
    }

    void floorPanelRequest(Request request) {
        requestQueue->enqueRequest(request);
    }

    void elevatorPanelRequest(Elevator*elevator, int destination) {
        elevator->addStop(destination);
    }



private:

    std::vector<std::unique_ptr<Elevator>> elevators;
    std::vector<std::unique_ptr<Floor>> floors;
    std::unique_ptr<RequestQueue> requestQueue;
    std::unique_ptr<AllotmentStrategy>allotmentStrategy;
    std::thread worker;

};
// --------------------------------------------------


// --------------------------------------------------
void FloorPanel::pressUp() {
    elevatorSystem->floorPanelRequest(Request{floor->getId(), Direction::Up});
}

void FloorPanel::pressDown() {
    elevatorSystem->floorPanelRequest(Request{floor->getId(), Direction::Down});
}

void ElevatorPanel::press(int floorId) {
    elevatorSystem->elevatorPanelRequest(elevator, floorId);
}


// --------------------------------------------------






// --------------------------------------------------
void stimulate() {

    ElevatorSystem elevatorSystem(3, 10, std::make_unique<ScanStrategy>());
    elevatorSystem.start();
    // std::this_thread::sleep_for(std::chrono::seconds(3));
    // elevatorSystem.getFloor(2)->getPanel()->pressUp();


    std::this_thread::sleep_for(std::chrono::seconds(1));
    elevatorSystem.getFloor(2)->getPanel()->pressUp();   // pickup at 2

    std::this_thread::sleep_for(std::chrono::seconds(2));
    elevatorSystem.getElevator(0)->getPanel()->press(6); // inside elevator → go to 6

    std::this_thread::sleep_for(std::chrono::seconds(2));
    elevatorSystem.getFloor(7)->getPanel()->pressDown(); // new request above

    std::this_thread::sleep_for(std::chrono::seconds(2));
    elevatorSystem.getElevator(0)->getPanel()->press(1); // go down

    std::this_thread::sleep_for(std::chrono::seconds(2));
    elevatorSystem.getFloor(3)->getPanel()->pressUp();   // mid-way request
        
    elevatorSystem.end();

}

int main() {
    stimulate();


}


