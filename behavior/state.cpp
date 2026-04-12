#include <iostream>
using namespace std;

/* Forward */
class MediaPlayer;

/* State interface */
class State {
public:
    virtual void play(MediaPlayer&) = 0;
    virtual void pause(MediaPlayer&) = 0;
    virtual void stop(MediaPlayer&) = 0;
    virtual ~State() = default;
};

/* Context */
class MediaPlayer {
public:
    MediaPlayer(State* s) : state(s) {}

    void setState(State* s) { state = s; }

    void play()  { state->play(*this); }
    void pause() { state->pause(*this); }
    void stop()  { state->stop(*this); }

private:
    State* state;
};

/* Concrete states */
class Stopped;
class Playing;
class Paused;


/* Stopped */
class Stopped : public State {
public:
    void play(MediaPlayer& mp) override;
    void pause(MediaPlayer&) override {
        cout << "already stopped\n";
    }
    void stop(MediaPlayer&) override {
        cout << "already stopped\n";
    }
};

/* Playing */
class Playing : public State {
public:
    void play(MediaPlayer&) override {
        cout << "already playing\n";
    }
    void pause(MediaPlayer& mp) override;
    void stop(MediaPlayer& mp) override;
};

/* Paused */
class Paused : public State {
public:
    void play(MediaPlayer& mp) override;
    void pause(MediaPlayer&) override {
        cout << "already paused\n";
    }
    void stop(MediaPlayer& mp) override;
};

/* ===== Shared instances ===== */
Stopped stoppedState;
Playing playingState;
Paused pausedState;

/* ===== Transitions ===== */
void Stopped::play(MediaPlayer& mp) {
    cout << "start playing\n";
    mp.setState(&playingState);
}

void Playing::pause(MediaPlayer& mp) {
    cout << "paused\n";
    mp.setState(&pausedState);
}

void Playing::stop(MediaPlayer& mp) {
    cout << "stopped\n";
    mp.setState(&stoppedState);
}

void Paused::play(MediaPlayer& mp) {
    cout << "resume playing\n";
    mp.setState(&playingState);
}

void Paused::stop(MediaPlayer& mp) {
    cout << "stopped\n";
    mp.setState(&stoppedState);
}

/* ===== Usage ===== */
int main() {
    MediaPlayer p(&stoppedState);

    p.play();   // start playing
    p.pause();  // paused
    p.play();   // resume
    p.stop();   // stopped
}
