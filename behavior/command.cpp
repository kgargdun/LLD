
#include<iostream>
using namespace std;

class TV {
public:
    void turnOn() {
        cout<<"turning on tv"<<endl;
    }
};

class Car {
public:
    void start() {
        cout<<"starting car"<<endl;
    }
};

class Command {
public:
    virtual void execute() = 0;
    virtual ~Command() = default;
};

class TVTurnOnCommand  : public Command {
public:
    TVTurnOnCommand(shared_ptr<TV> tv) {
        this->tv = tv;
    }
    void execute() override {
        auto sp = this->tv.lock();
        if(sp) sp->turnOn();
    }
private:
    weak_ptr<TV>tv;
};

class CarStartComand : public Command {
public:
    CarStartComand(shared_ptr<Car> car) {
        this->car = car;
    }
    void execute() override {
        auto sp = this->car.lock();
        if(sp) sp->start();
    }
private:
    weak_ptr<Car>car;
};

class Remote {
public:
    void setOnCommand(shared_ptr<Command>command) {
        this->onCommand = command;
    }

    void setOffCommand(shared_ptr<Command>command) {
        this->OffCommand = command;
    }

    void pressOn() {
        this->onCommand->execute();
    }

    void pressOff() {
        this->OffCommand->execute();
    }
private:
    shared_ptr<Command>onCommand, OffCommand;
};


int main() {
    shared_ptr<TV> tv = make_shared<TV>();
    shared_ptr<Car> car = make_shared<Car>();
    Remote remote;
    shared_ptr<TVTurnOnCommand> tvTurnOnCommand  = make_shared<TVTurnOnCommand>(tv);
    shared_ptr<CarStartComand> carStartComand  = make_shared<CarStartComand>(car);
    remote.setOnCommand(tvTurnOnCommand);
    remote.pressOn();

}



