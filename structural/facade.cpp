#include <iostream>
#include <string>
using namespace std;

// ===== Subsystems =====

class Amplifier {
public:
    void on()  { cout << "Amplifier on\n"; }
    void off() { cout << "Amplifier off\n"; }
};

class DVDPlayer {
public:
    void on() { cout << "DVD on\n"; }
    void play(const string& movie) { cout << "Playing \"" << movie << "\"\n"; }
    void stop() { cout << "DVD stopped\n"; }
    void off() { cout << "DVD off\n"; }
};

class Projector {
public:
    void on()  { cout << "Projector on\n"; }
    void off() { cout << "Projector off\n"; }
};

class Lights {
public:
    void dim(int level) { cout << "Lights dimmed to " << level << "%\n"; }
    void on() { cout << "Lights on\n"; }
};

// ===== Facade =====

class HomeTheaterFacade {
    Amplifier& amp;
    DVDPlayer& dvd;
    Projector& projector;
    Lights& lights;

public:
    HomeTheaterFacade(Amplifier& a, DVDPlayer& d, Projector& p, Lights& l)
        : amp(a), dvd(d), projector(p), lights(l) {}

    void watchMovie(const string& movie) {
        lights.dim(10);
        projector.on();
        amp.on();
        dvd.on();
        dvd.play(movie);
    }

    void endMovie() {
        dvd.stop();
        dvd.off();
        amp.off();
        projector.off();
        lights.on();
    }
};

// ===== Demo =====

int main() {
    Amplifier amp;
    DVDPlayer dvd;
    Projector projector;
    Lights lights;

    HomeTheaterFacade theater(amp, dvd, projector, lights);

    theater.watchMovie("Inception");
    theater.endMovie();
}
