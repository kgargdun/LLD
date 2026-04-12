#include <iostream>
#include <vector>
#include <memory>

using namespace std;

class Observer {
public:
    virtual void update(int price) = 0;
    virtual ~Observer() = default;
};

class Subject {
public:
    virtual void addObserver(const shared_ptr<Observer>& o) = 0;
    virtual void removeObserver(const shared_ptr<Observer>& o) = 0;
    virtual void notifyAll() = 0;
    virtual ~Subject() = default;
};

class Stock : public Subject {
public:
    Stock(int p) : price(p) {}

    void changePrice(int newPrice) {
        price = newPrice;
        notifyAll();
    }

    void addObserver(const shared_ptr<Observer>& o) override {
        observers.push_back(o);   // weak_ptr from shared_ptr
    }

    void removeObserver(const shared_ptr<Observer>& o) override {
        for (auto it = observers.begin(); it != observers.end(); ) {
            auto sp = it->lock();
            if (!sp || sp == o) {   // expired OR same observer
                it = observers.erase(it);
            } else {
                ++it;
            }
        }
    }

    void notifyAll() override {
        for (auto it = observers.begin(); it != observers.end(); ) {
            if (auto sp = it->lock()) {
                sp->update(price);
                ++it;
            } else {
                it = observers.erase(it);  // cleanup expired
            }
        }
    }

private:
    int price;
    vector<weak_ptr<Observer>> observers;
};

class MobileDisplay : public Observer {
public:
    void update(int price) override {
        cout << "Mobile: " << price << "\n";
    }
};

class WebDisplay : public Observer {
public:
    void update(int price) override {
        cout << "Web: " << price << "\n";
    }
};

int main() {
    Stock stock(100);

    auto web = make_shared<WebDisplay>();
    auto mobile = make_shared<MobileDisplay>();

    stock.addObserver(web);
    stock.addObserver(mobile);

    stock.changePrice(110);

    stock.removeObserver(web);   // manual remove

    stock.changePrice(120);

    web.reset();                 // destroy observer

    stock.changePrice(130);      // only mobile
}


