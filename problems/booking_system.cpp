#include <bits/stdc++.h>
using namespace std;

enum class SeatType {
    Affordable = 0, Regular = 1, Premium = 2, VIP = 3	
};

enum class ScreenType {
    _2D = 0, _3D = 1, _4D = 2
};

enum class SeatStatus {
    Available = 0, Reserved = 1, Booked = 2 
};

enum class BookingStatus {
    Failed = 0, Hold = 1, Success = 2
};

class Seat {
public:
    Seat(int id, SeatType type) : id(id), type(type) {}
    int getId() const { return id; }

private:
    int id;
    SeatType type;
};

class Screen {
public:
    Screen(int id, ScreenType type) : id(id), type(type) {
        for (int i = 1; i <= 5; i++) {
            seats.emplace_back(make_unique<Seat>(i, SeatType::Regular));
        }
    }

    vector<int> getSeatIds() const {
        vector<int> ids;
        for (auto &s : seats) ids.push_back(s->getId());
        return ids;
    }

private:
    int id;
    ScreenType type;
    vector<unique_ptr<Seat>> seats;
};

class ShowSeat {
public:
    ShowSeat(int show_id, int seat_id)
        : seat_id(seat_id), show_id(show_id),
          status(SeatStatus::Available), bookingId(-1) {}

    timed_mutex& getMutex() { return showSeatMutex; }

    SeatStatus getStatus() { return status; }
    int getBookingId() { return bookingId; }

    void reserve(int bId) {
        status = SeatStatus::Reserved;
        bookingId = bId;
    }

    void book(int bId) {
        status = SeatStatus::Booked;
        bookingId = bId;
    }

    void makeAvailable() {
        status = SeatStatus::Available;
        bookingId = -1;
    }

private:
    int seat_id;
    int show_id;
    SeatStatus status;
    int bookingId;
    timed_mutex showSeatMutex;
};

class Show {
public:
    Show(int id, shared_ptr<Screen> screen) : id(id), screen(screen) {
        for (auto sid : screen->getSeatIds()) {
            showSeats[sid] = make_shared<ShowSeat>(id, sid);
        }
    }

    shared_ptr<ShowSeat> getSeat(int seat_id) {
        auto it = showSeats.find(seat_id);
        if (it == showSeats.end()) return nullptr;
        return it->second;
    }

private:
    map<int, shared_ptr<ShowSeat>> showSeats;
    shared_ptr<Screen> screen;
    int id;
};

class User {
public:
    User(int id, string name) : id(id), name(name) {}
private:
    int id;
    string name;
};

class Booking {
public:
    Booking(int id, User* user, Show* show, vector<int> seatIds)
        : bookingId(id), user(user), show(show), seatIds(seatIds),
          bookingStatus(BookingStatus::Hold) {
        expiryTime = chrono::steady_clock::now() + chrono::seconds(30);
    }

    int getId() { return bookingId; }
    vector<int> getSeats() { return seatIds; }
    Show* getShow() { return show; }
    BookingStatus getStatus() { return bookingStatus; }

    bool isExpired() {
        return chrono::steady_clock::now() > expiryTime;
    }

    void markSuccess() { bookingStatus = BookingStatus::Success; }
    void markFailed() { bookingStatus = BookingStatus::Failed; }

private:
    int bookingId;
    BookingStatus bookingStatus;
    vector<int> seatIds;
    User* user;
    Show* show;
    chrono::steady_clock::time_point expiryTime;
};

class PaymentStrategy {
public:
    virtual bool pay(int amount) = 0;
};

class DummyPayment : public PaymentStrategy {
public:
    bool pay(int amount) override { return true; }
};

class PaymentProcessor {
public:
    bool pay(int amount) {
        return strategy->pay(amount);
    }

    void setStrategy(unique_ptr<PaymentStrategy> s) {
        strategy = move(s);
    }

private:
    unique_ptr<PaymentStrategy> strategy;
};

class BookingManager {
public:
    BookingManager() {
        startCleanupThread();
    }

    Booking* reserve(User *user, Show* show, vector<int> seats) {
        sort(seats.begin(), seats.end());

        if (adjacent_find(seats.begin(), seats.end()) != seats.end()) {
            return nullptr;
        }

        int bookingId = idGen++;

        vector<shared_ptr<ShowSeat>> showSeats;
        for(auto seat : seats) {
            auto s = show->getSeat(seat);
            if (!s) return nullptr;
            showSeats.emplace_back(s);
        }

        vector<unique_lock<timed_mutex>> locks;

        for (auto &seat : showSeats) {
            unique_lock<timed_mutex> lk(seat->getMutex(), defer_lock);
            if (!lk.try_lock_for(chrono::milliseconds(200))) {
                return nullptr;
            }
            locks.push_back(move(lk));
        }

        for (auto &seat : showSeats) {
            if (seat->getStatus() != SeatStatus::Available) {
                return nullptr;
            }
        }

        for (auto &seat : showSeats) {
            seat->reserve(bookingId);
        }

        auto booking = make_shared<Booking>(bookingId, user, show, seats);

        {
            lock_guard<mutex> lk(bookingMutex);
            bookings.push_back(booking);
        }

        return booking.get();
    }

    bool book(Booking *booking) {
        if (!booking || booking->getStatus() != BookingStatus::Hold || booking->isExpired())
            return false;

        auto show = booking->getShow();
        auto seats = booking->getSeats();

        sort(seats.begin(), seats.end());

        vector<shared_ptr<ShowSeat>> showSeats;
        for(auto seat : seats) showSeats.emplace_back(show->getSeat(seat));

        vector<unique_lock<timed_mutex>> locks;

        for (auto &seat : showSeats) {
            unique_lock<timed_mutex> lk(seat->getMutex(), defer_lock);
            if (!lk.try_lock_for(chrono::milliseconds(200))) {
                return false;
            }
            locks.push_back(move(lk));
        }

        for (auto &seat : showSeats) {
            if (seat->getStatus() != SeatStatus::Reserved ||
                seat->getBookingId() != booking->getId()) {
                return false;
            }
        }

        for (auto &seat : showSeats) {
            seat->book(booking->getId());
        }

        booking->markSuccess();
        return true;
    }

    bool fail(Booking *booking) {
        if (!booking || booking->getStatus() != BookingStatus::Hold)
            return false;

        auto show = booking->getShow();
        auto seats = booking->getSeats();

        sort(seats.begin(), seats.end());

        vector<shared_ptr<ShowSeat>> showSeats;
        for(auto seat : seats) showSeats.emplace_back(show->getSeat(seat));

        vector<unique_lock<timed_mutex>> locks;

        for (auto &seat : showSeats) {
            unique_lock<timed_mutex> lk(seat->getMutex(), defer_lock);
            if (!lk.try_lock_for(chrono::milliseconds(200))) {
                return false;
            }
            locks.push_back(move(lk));
        }

        for (auto &seat : showSeats) {
            if (seat->getStatus() == SeatStatus::Reserved &&
                seat->getBookingId() == booking->getId()) {
                seat->makeAvailable();
            }
        }

        booking->markFailed();
        return true;
    }

private:
    void cleanupExpired() {
        vector<shared_ptr<Booking>> copy;
        {
            lock_guard<mutex> lk(bookingMutex);
            copy = bookings;
        }

        for (auto &b : copy) {
            if (b->getStatus() == BookingStatus::Hold && b->isExpired()) {
                fail(b.get());
            }
        }
    }

    void startCleanupThread() {
        thread([this]() {
            while (true) {
                this_thread::sleep_for(chrono::seconds(5));
                cleanupExpired();
            }
        }).detach();
    }

private:
    atomic<int> idGen{1};
    vector<shared_ptr<Booking>> bookings;
    mutex bookingMutex;
};

class BookingSystem {
public:
    BookingSystem() {
        auto screen = make_shared<Screen>(1, ScreenType::_2D);
        auto show = make_shared<Show>(1, screen);

        screens.push_back(screen);
        shows.push_back(show);
    }

    shared_ptr<Show> getShow() {
        return shows[0];
    }

private:
    vector<shared_ptr<Show>> shows;
    vector<shared_ptr<Screen>> screens;
};