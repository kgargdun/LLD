#include<iostream>
#include<map>
#include<set>
#include<vector>
#include<memory>
#include<chrono>
#include<thread>
using namespace std;

/*
-------------------------------------
*/

enum class VehicleCategory {
    TwoWheeler = 0, FourWheeler = 1, Heavy = 2
};

class Vehicle {
public:
    explicit Vehicle(VehicleCategory vehicleCategory, string id)
        : vehicleCategory(vehicleCategory), id(id) {}

    VehicleCategory getCategory() const {
        return vehicleCategory;
    }

private:
    VehicleCategory vehicleCategory;
    string id;
};

/*
--------------------------
*/

struct Event {
    using Clock = std::chrono::system_clock;
    using TimePoint = Clock::time_point;

    TimePoint time_point;

    static TimePoint getCurrent() {
        return Clock::now();
    }
};

class PricingStrategy {
public:
    virtual ~PricingStrategy() = default;
    virtual int price(std::chrono::seconds duration) = 0;
};

class RoutineStrategy : public PricingStrategy {
public:
    int price(std::chrono::seconds duration) override {
        return duration.count(); // simple
    }
};

class Slot;

class Ticket {
public:
    Ticket(unique_ptr<PricingStrategy> pricingStrategy, Slot* slot)
        : pricingStrategy(std::move(pricingStrategy)), slot(slot) {
        entry = Event{Event::getCurrent()};
        valid = true;
    }

    int getPrice() {
        auto now = Event::getCurrent();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - entry.time_point);
        valid = false;
        return pricingStrategy->price(duration);
    }

    Slot* getSlot() {
        return slot;
    }

private:
    unique_ptr<PricingStrategy> pricingStrategy;
    Event entry;
    bool valid;
    Slot* slot;
};

/*
------------------------
*/

enum class SlotCategory {
    Small = 0, Medium = 1, Big = 2
};

class Slot {
public:
    Slot(SlotCategory slotCategory, int id)
        : slotCategory(slotCategory), id(id) {}

    bool canPark(shared_ptr<Vehicle> vehicle) {
        return static_cast<int>(vehicle->getCategory()) <= static_cast<int>(slotCategory);
    }

    SlotCategory getCategory() {
        return slotCategory;
    }

private:
    SlotCategory slotCategory;
    int id;
};

/*
--------------------------
*/

class ParkingFLoor {
public:
    ParkingFLoor(int id, map<SlotCategory, int> layout) : id(id) {
        int gid = 0;
        for (const auto& entry : layout) {
            for (int i = 0; i < entry.second; i++) {
                empty[entry.first].push_back(make_unique<Slot>(entry.first, gid++));
            }
        }
    }

    shared_ptr<Ticket> park(SlotCategory slotCategory, shared_ptr<Vehicle> vehicle) {
        if (!hasVacancy(slotCategory, vehicle)) return nullptr;

        auto slot = std::move(empty[slotCategory].back());
        empty[slotCategory].pop_back();

        Slot* raw = slot.get();
        taken[slotCategory].push_back(std::move(slot));

        return make_shared<Ticket>(make_unique<RoutineStrategy>(), raw);
    }

    int withdraw(shared_ptr<Ticket> ticket) {
        Slot* slot = ticket->getSlot();
        SlotCategory cat = slot->getCategory();

        auto& vec = taken[cat];

        for (auto it = vec.begin(); it != vec.end(); ++it) {
            if (it->get() == slot) {
                empty[cat].push_back(std::move(*it));
                vec.erase(it);
                return ticket->getPrice();
            }
        }
        return -1;
    }

    bool hasVacancy(SlotCategory slotCategory, shared_ptr<Vehicle> vehicle) {
        return !empty[slotCategory].empty() &&
               empty[slotCategory].back()->canPark(vehicle);
    }

private:
    int id;
    map<SlotCategory, vector<unique_ptr<Slot>>> empty;
    map<SlotCategory, vector<unique_ptr<Slot>>> taken;
};

/*
--------------------------
*/

class AllocationStrategy {
public:
    virtual ~AllocationStrategy() = default;

    virtual shared_ptr<Ticket> park(
        shared_ptr<Vehicle> vehicle,
        int preferedFloor,
        vector<unique_ptr<ParkingFLoor>>& floors) = 0;
};

class SmallestFitStrategy : public AllocationStrategy {
public:
    shared_ptr<Ticket> park(
        shared_ptr<Vehicle> vehicle,
        int preferedFloor,
        vector<unique_ptr<ParkingFLoor>>& floors) override {

        const vector<SlotCategory> allSlots = {
            SlotCategory::Small, SlotCategory::Medium, SlotCategory::Big
        };

        for (const auto& slot : allSlots) {
            for (int i = 0; i < floors.size(); i++) {
                if (floors[i]->hasVacancy(slot, vehicle)) {
                    return floors[i]->park(slot, vehicle);
                }
            }
        }
        return nullptr;
    }
};

/*
--------------------------
*/

class ParkingLot {
public:
    ParkingLot(unique_ptr<AllocationStrategy> allocationStrategy) {
        this->allocationStrategy = std::move(allocationStrategy);

        map<SlotCategory, int> layout = {
            {SlotCategory::Small, 10},
            {SlotCategory::Medium, 5},
            {SlotCategory::Big, 5}
        };

        floors.push_back(make_unique<ParkingFLoor>(0, layout));
        floors.push_back(make_unique<ParkingFLoor>(1, layout));
    }

    shared_ptr<Ticket> park(shared_ptr<Vehicle> vehicle, int preferedFloor) {
        return allocationStrategy->park(vehicle, preferedFloor, floors);
    }

    int withdraw(shared_ptr<Ticket> ticket) {
        for (auto& floor : floors) {
            int res = floor->withdraw(ticket);
            if (res != -1) return res;
        }
        return -1;
    }

private:
    vector<unique_ptr<ParkingFLoor>> floors;
    unique_ptr<AllocationStrategy> allocationStrategy;
};

int main() {

    auto strategy = make_unique<SmallestFitStrategy>();
    ParkingLot lot(std::move(strategy));

    auto bike = make_shared<Vehicle>(VehicleCategory::TwoWheeler, "B1");
    auto car  = make_shared<Vehicle>(VehicleCategory::FourWheeler, "C1");

    auto t1 = lot.park(bike, 0);
    if (t1) cout << "Bike parked\n";

    auto t2 = lot.park(car, 0);
    if (t2) cout << "Car parked\n";

    // simulate time passing
    std::this_thread::sleep_for(std::chrono::seconds(5));

    if (t1) {
        int price = lot.withdraw(t1);
        cout << "Bike exit, price: " << price << endl;
    }

    if (t2) {
        int price = lot.withdraw(t2);
        cout << "Car exit, price: " << price << endl;
    }

    return 0;



}