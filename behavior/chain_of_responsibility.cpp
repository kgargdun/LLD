#include<iostream>
using namespace std;

class Approver {
public:
    void setNext(Approver *approver)  {
        this->approver = approver;
    }
    virtual bool handle(int amount) = 0;
    virtual ~Approver() = default; 
protected:
    Approver *approver = nullptr;
};

class Rejector : public Approver {
public:
    bool handle(int amount) override {
        return false;
    }

    static Approver& getInstance() {
        static Rejector r;
        return r;
    }
};



class Lead : public Approver {
public:
    Lead() {
        approver = &Rejector::getInstance();
    }
    bool handle(int amount) override {
        if(amount <=100) return true;
        return approver->handle(amount);
    }
};

class Director : public Approver {
public:
    Director() {
        approver = &Rejector::getInstance();
    }
    bool handle(int amount) override {
         if(amount <=1000) return true;
        return approver->handle(amount);
    }
};

int main() {
    Lead lead;
    Director director;
    lead.setNext(&director);
    cout<<lead.handle(100)<<endl;
    cout<<lead.handle(100000)<<endl;

}






