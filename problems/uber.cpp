#include <algorithm>
#include <atomic>
#include <cmath>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Location {
    double lat{0.0};
    double lon{0.0};
};

static double distanceKm(const Location& a, const Location& b) {
    // Simple Euclidean approximation for demo purposes.
    double dx = a.lat - b.lat;
    double dy = a.lon - b.lon;
    return std::sqrt(dx * dx + dy * dy);
}

enum class RideStatus {
    REQUESTED,
    DRIVER_ASSIGNED,
    ARRIVED,
    STARTED,
    COMPLETED,
    CANCELLED
};

class Driver {
public:
    explicit Driver(int driverId) : id(driverId), available(true) {}

    int getId() const { return id; }

    bool tryReserve() {
        bool expected = true;
        return available.compare_exchange_strong(expected, false);
    }

    void release() {
        available.store(true);
    }

    bool isAvailable() const {
        return available.load();
    }

    void updateLocation(const Location& loc) {
        std::lock_guard<std::mutex> lock(locationMutex);
        location = loc;
    }

    Location getLocation() const {
        std::lock_guard<std::mutex> lock(locationMutex);
        return location;
    }

private:
    int id;
    std::atomic<bool> available;
    mutable std::mutex locationMutex;
    Location location;
};

class Ride {
public:
    Ride(int rideId, Location pickupLocation, Location dropLocation)
        : id(rideId),
          pickup(pickupLocation),
          drop(dropLocation),
          status(RideStatus::REQUESTED),
          driverId(-1) {}

    int getId() const { return id; }
    Location getPickup() const { return pickup; }
    Location getDrop() const { return drop; }

    RideStatus getStatus() const {
        return status.load();
    }

    int getDriverId() const {
        return driverId.load();
    }

    bool tryTransition(RideStatus expected, RideStatus next) {
        return status.compare_exchange_strong(expected, next);
    }

    void setDriverId(int did) {
        driverId.store(did);
    }

private:
    int id;
    const Location pickup;
    const Location drop;
    std::atomic<RideStatus> status;
    std::atomic<int> driverId;
};

class PricingService {
public:
    double estimateFare(const Location& pickup, const Location& drop) const {
        double dist = distanceKm(pickup, drop);
        return baseFare + dist * perKmRate;
    }

private:
    const double baseFare = 10.0;
    const double perKmRate = 2.0;
};

class IGeoIndex {
public:
    virtual ~IGeoIndex() = default;
    virtual void upsertDriver(int driverId, const Location& location) = 0;
    virtual std::vector<int> getNearbyDrivers(const Location& pickup) = 0;
};

class GridGeoIndex : public IGeoIndex {
public:
    explicit GridGeoIndex(double cellSize = 0.01) : cellSize(cellSize) {}

    void upsertDriver(int driverId, const Location& location) override {
        std::lock_guard<std::mutex> lock(mu);

        std::string newCell = cellKey(location);
        auto it = driverToCell.find(driverId);
        if (it != driverToCell.end()) {
            cellToDrivers[it->second].erase(driverId);
        }

        driverToCell[driverId] = newCell;
        cellToDrivers[newCell].insert(driverId);
    }

    std::vector<int> getNearbyDrivers(const Location& pickup) override {
        std::lock_guard<std::mutex> lock(mu);

        auto [cx, cy] = cellCoords(pickup);
        std::unordered_set<int> result;

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                std::string key = cellKey(cx + dx, cy + dy);
                auto it = cellToDrivers.find(key);
                if (it == cellToDrivers.end()) continue;

                for (int driverId : it->second) {
                    result.insert(driverId);
                }
            }
        }

        return std::vector<int>(result.begin(), result.end());
    }

private:
    std::pair<int, int> cellCoords(const Location& loc) const {
        int x = static_cast<int>(std::floor(loc.lat / cellSize));
        int y = static_cast<int>(std::floor(loc.lon / cellSize));
        return {x, y};
    }

    std::string cellKey(const Location& loc) const {
        auto [x, y] = cellCoords(loc);
        return cellKey(x, y);
    }

    std::string cellKey(int x, int y) const {
        return std::to_string(x) + "#" + std::to_string(y);
    }

    double cellSize;
    std::mutex mu;
    std::unordered_map<int, std::string> driverToCell;
    std::unordered_map<std::string, std::unordered_set<int>> cellToDrivers;
};

class RideService {
public:
    int createDriver() {
        std::lock_guard<std::mutex> lock(mu);
        int id = nextDriverId++;
        drivers[id] = std::make_shared<Driver>(id);
        return id;
    }

    int createRide(const Location& pickup, const Location& drop) {
        std::lock_guard<std::mutex> lock(mu);
        int id = nextRideId++;
        rides[id] = std::make_shared<Ride>(id, pickup, drop);
        return id;
    }

    std::shared_ptr<Driver> getDriver(int driverId) const {
        std::lock_guard<std::mutex> lock(mu);
        auto it = drivers.find(driverId);
        if (it == drivers.end()) return nullptr;
        return it->second;
    }

    std::shared_ptr<Ride> getRide(int rideId) const {
        std::lock_guard<std::mutex> lock(mu);
        auto it = rides.find(rideId);
        if (it == rides.end()) return nullptr;
        return it->second;
    }

    bool tryAssignRide(int rideId, int driverId) {
        auto ride = getRide(rideId);
        auto driver = getDriver(driverId);

        if (!ride || !driver) return false;

        if (!driver->tryReserve()) {
            return false;
        }

        RideStatus expected = RideStatus::REQUESTED;
        if (!ride->tryTransition(expected, RideStatus::DRIVER_ASSIGNED)) {
            driver->release();
            return false;
        }

        ride->setDriverId(driverId);
        return true;
    }

    bool cancelRide(int rideId) {
        auto ride = getRide(rideId);
        if (!ride) return false;

        RideStatus expected = RideStatus::REQUESTED;
        return ride->tryTransition(expected, RideStatus::CANCELLED);
    }

    bool markArrived(int rideId, int driverId) {
        auto ride = getRide(rideId);
        if (!ride || ride->getDriverId() != driverId) return false;

        RideStatus expected = RideStatus::DRIVER_ASSIGNED;
        return ride->tryTransition(expected, RideStatus::ARRIVED);
    }

    bool startRide(int rideId, int driverId) {
        auto ride = getRide(rideId);
        if (!ride || ride->getDriverId() != driverId) return false;

        RideStatus expected = RideStatus::ARRIVED;
        return ride->tryTransition(expected, RideStatus::STARTED);
    }

    bool completeRide(int rideId, int driverId) {
        auto ride = getRide(rideId);
        if (!ride || ride->getDriverId() != driverId) return false;

        RideStatus expected = RideStatus::STARTED;
        if (!ride->tryTransition(expected, RideStatus::COMPLETED)) {
            return false;
        }

        auto driver = getDriver(driverId);
        if (driver) {
            driver->release();
        }
        return true;
    }

private:
    mutable std::mutex mu;
    int nextDriverId{1};
    int nextRideId{1};
    std::unordered_map<int, std::shared_ptr<Driver>> drivers;
    std::unordered_map<int, std::shared_ptr<Ride>> rides;
};

class LocationService {
public:
    LocationService(RideService& rideService, IGeoIndex& geoIndex)
        : rideService(rideService), geoIndex(geoIndex) {}

    bool updateDriverLocation(int driverId, const Location& location) {
        auto driver = rideService.getDriver(driverId);
        if (!driver) return false;

        driver->updateLocation(location);
        geoIndex.upsertDriver(driverId, location);
        return true;
    }

private:
    RideService& rideService;
    IGeoIndex& geoIndex;
};

class MatchingService {
public:
    MatchingService(RideService& rideService, IGeoIndex& geoIndex)
        : rideService(rideService), geoIndex(geoIndex) {}

    bool assignNearestDriver(int rideId) {
        auto ride = rideService.getRide(rideId);
        if (!ride) return false;

        auto candidates = geoIndex.getNearbyDrivers(ride->getPickup());
        for (int driverId : candidates) {
            if (rideService.tryAssignRide(rideId, driverId)) {
                return true;
            }
        }
        return false;
    }

private:
    RideService& rideService;
    IGeoIndex& geoIndex;
};

int main() {
    RideService rideService;
    GridGeoIndex geoIndex;
    LocationService locationService(rideService, geoIndex);
    MatchingService matchingService(rideService, geoIndex);
    PricingService pricingService;

    int driver1 = rideService.createDriver();
    int driver2 = rideService.createDriver();

    locationService.updateDriverLocation(driver1, {12.9716, 77.5946});
    locationService.updateDriverLocation(driver2, {12.9720, 77.5950});

    int rideId = rideService.createRide({12.9718, 77.5948}, {12.9800, 77.6100});

    auto ride = rideService.getRide(rideId);
    if (ride) {
        double fare = pricingService.estimateFare(ride->getPickup(), ride->getDrop());
        std::cout << "Estimated fare: " << fare << "\n";
    }

    if (matchingService.assignNearestDriver(rideId)) {
        auto assignedRide = rideService.getRide(rideId);
        int assignedDriverId = assignedRide ? assignedRide->getDriverId() : -1;
        std::cout << "Ride assigned to driver: " << assignedDriverId << "\n";

        rideService.markArrived(rideId, assignedDriverId);
        rideService.startRide(rideId, assignedDriverId);
        rideService.completeRide(rideId, assignedDriverId);

        std::cout << "Ride completed\n";
    } else {
        std::cout << "No driver assigned\n";
    }

    return 0;
}