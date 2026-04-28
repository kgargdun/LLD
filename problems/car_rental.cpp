#include <string>
#include <vector>
#include <map>
#include <memory>
#include<iostream>

using namespace std;

enum class VehicleState {
    Available, Reserved, Rented, Maintenance
};

enum class VehicleType {
    Hatchback, Sedan, SUV, Convertible, Other
};

enum class ReservationStatus {
    Active, Completed, Cancelled
};

enum class PaymentStatus {
    Pending, Completed, Failed
};

struct Location {
    string name;
    int id;
};

class Vehicle {
public:
    Vehicle(int id, int cap, string brand, VehicleType type, Location loc)
        : id(id), capacity(cap), brand(brand), type(type), location(loc), state(VehicleState::Available) {}

    VehicleState getState() const { return state; }
    VehicleType getType() const { return type; }
    int getCapacity() const { return capacity; }

    void markReserved() { state = VehicleState::Reserved; }
    void markAvailable() { state = VehicleState::Available; }
    void markRented() { state = VehicleState::Rented; }

private:
    int id;
    int capacity;
    string brand;
    VehicleState state;
    VehicleType type;
    Location location;
};

class Inventory {
public:
    Inventory(Location loc) : location(loc) {}

    void addVehicle(Vehicle* v) {
        vehicles.push_back(v);
    }
  
    vector<Vehicle*> getAvailableVehicles() {
        vector<Vehicle*> result;
        for (auto v : vehicles) {
            if (v->getState() == VehicleState::Available) {
                result.push_back(v);
            }
        }
        return result;
    }
  
private:
    Location location;
    vector<Vehicle*> vehicles;
};

class SearchStrategy {
public:
    virtual void filter(const vector<Vehicle*>& input,
                        vector<Vehicle*>& output) = 0;
    virtual ~SearchStrategy() = default;
};

class DefaultSearch : public SearchStrategy {
public:
    void filter(const vector<Vehicle*>& input,
                vector<Vehicle*>& output) override {
        for (auto v : input) {
            // simple default: SUV + capacity >= 5
            if (v->getType() == VehicleType::SUV && v->getCapacity() >= 5) {
                output.push_back(v);
            }
        }
    }
};

class PricingStrategy {
public:
    virtual double calculatePrice(int hours) = 0;
    virtual ~PricingStrategy() = default;
};

class HourlyPricing : public PricingStrategy {
public:
    double calculatePrice(int hours) override {
        return hours * 10.0; // simple logic
    }
};

class Payment {
public:
    Payment(double amount) : amount(amount), status(PaymentStatus::Pending) {}

    void complete() { status = PaymentStatus::Completed; }

private:
    double amount;
    PaymentStatus status;
};

class User {
public:
    User(int id) : id(id) {}

private:
    int id;
};

class Reservation {
public:
    Reservation(int id, User* user, Vehicle* vehicle, int hours)
        : id(id), user(user), vehicle(vehicle), hours(hours), status(ReservationStatus::Active) {}

    Vehicle* getVehicle() { return vehicle; }
    int getHours() const { return hours; }

    void complete() { status = ReservationStatus::Completed; }

private:
    int id;
    User* user;
    Vehicle* vehicle;
    int hours;
    ReservationStatus status;
};

class CarRentalSystem {
public:
    CarRentalSystem() {
        searchStrategy = make_shared<DefaultSearch>();
        pricingStrategy = make_shared<HourlyPricing>();
    }

    void addInventory(int locationId, shared_ptr<Inventory> inv) {
        inventories[locationId] = inv;
    }

    void addUser(User* user) {
        users.push_back(user);
    }

    vector<Vehicle*> search(int locationId) {
        auto inventory = inventories[locationId];
        auto available = inventory->getAvailableVehicles();

        vector<Vehicle*> result;
        searchStrategy->filter(available, result);
        return result;
    }

    Reservation* reserve(User* user, Vehicle* vehicle, int hours) {
        if (vehicle->getState() != VehicleState::Available) return nullptr;

        vehicle->markReserved();

        auto res = new Reservation(++reservationId, user, vehicle, hours);
        reservations.push_back(res);
        return res;
    }

    Payment* makePayment(Reservation* res) {
        double amount = pricingStrategy->calculatePrice(res->getHours());
        auto payment = new Payment(amount);
        payment->complete();
        return payment;
    }

private:
    map<int, shared_ptr<Inventory>> inventories;
    vector<User*> users;
    vector<Reservation*> reservations;

    shared_ptr<SearchStrategy> searchStrategy;
    shared_ptr<PricingStrategy> pricingStrategy;

    int reservationId = 0;
};

int main() {

    // Create system
    CarRentalSystem system;

    // Create location + inventory
    Location loc{ "Bangalore", 1 };
    auto inventory = make_shared<Inventory>(loc);

    // Add vehicles
    Vehicle* v1 = new Vehicle(1, 4, "Toyota", VehicleType::Sedan, loc);
    Vehicle* v2 = new Vehicle(2, 6, "Mahindra", VehicleType::SUV, loc);

    inventory->addVehicle(v1);
    inventory->addVehicle(v2);

    system.addInventory(loc.id, inventory);

    // Add user
    User* user = new User(101);
    system.addUser(user);

    // Search vehicles
    auto vehicles = system.search(loc.id);

    if (vehicles.empty()) {
        std::cout << "No vehicles found\n";
        return 0;
    }

    // Select first vehicle
    Vehicle* selected = vehicles[0];

    // Reserve
    auto reservation = system.reserve(user, selected, 5);
    if (!reservation) {
        std::cout << "Reservation failed\n";
        return 0;
    }

    // Payment
    auto payment = system.makePayment(reservation);

    std::cout << "Booking successful\n";

    return 0;
}