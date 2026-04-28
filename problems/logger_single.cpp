#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

using namespace std;

enum class LogLevel { DEBUG, INFO, WARN, ERROR };

struct LogEvent {
    string msg;
    string tag;
};

class LogWriter {
public:
    virtual void write(const LogEvent& e) = 0;
    virtual ~LogWriter() = default;
};

class ConsoleWriter : public LogWriter {
public:
    void write(const LogEvent& e) override {
        cout << "[" << e.tag << "] " << e.msg << endl;
    }
};

class FileWriter : public LogWriter {
    string path;
public:
    FileWriter(string p) : path(p) {}
    void write(const LogEvent& e) override {
        ofstream out(path, ios::app);
        out << "[" << e.tag << "] " << e.msg << endl;
    }
};

class Dispatcher {
public:
    Dispatcher() {
        worker = thread([this]() { run(); });
    }

    ~Dispatcher() {
        {
            unique_lock<mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        worker.join();
    }

    void dispatch(const LogEvent& e) {
        {
            lock_guard<mutex> lock(mtx);
            q.push(e);
        }
        cv.notify_one();
    }

    void addConsole(string tag) {
        writers[tag].push_back(make_shared<ConsoleWriter>());
    }

    void addFile(string tag, string path) {
        writers[tag].push_back(make_shared<FileWriter>(path));
    }

private:
    void run() {
        while (true) {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [&]() { return stop || !q.empty(); });

            if (stop && q.empty()) return;

            auto e = q.front(); q.pop();
            lock.unlock();

            if (writers.count(e.tag)) {
                for (auto& w : writers[e.tag]) {
                    w->write(e);
                }
            }
        }
    }

private:
    queue<LogEvent> q;
    mutex mtx;
    condition_variable cv;
    thread worker;
    bool stop = false;

    unordered_map<string, vector<shared_ptr<LogWriter>>> writers;
};

class Logger {
    string tag;
    shared_ptr<Dispatcher> d;
public:
    Logger(string t, shared_ptr<Dispatcher> d) : tag(t), d(d) {}

    void log(string msg) {
        d->dispatch({msg, tag});
    }
};

class LoggerManager {
public:
    static LoggerManager& get() {
        static LoggerManager inst;
        return inst;
    }

    shared_ptr<Logger> getLogger(string tag) {
        if (!mp.count(tag))
            mp[tag] = make_shared<Logger>(tag, dispatcher);
        return mp[tag];
    }

    void addConsole(string tag) { dispatcher->addConsole(tag); }
    void addFile(string tag, string path) { dispatcher->addFile(tag, path); }

private:
    LoggerManager() {
        dispatcher = make_shared<Dispatcher>();
    }

    unordered_map<string, shared_ptr<Logger>> mp;
    shared_ptr<Dispatcher> dispatcher;
};


int main() {

    // configure your serivce
    auto& mgr = LoggerManager::get();
    mgr.addConsole("auth");
    mgr.addFile("auth", "auth.log");

    // log your events
    auto logger = mgr.getLogger("auth");
    logger->log("hello");
    logger->log("world");

    this_thread::sleep_for(chrono::seconds(1));
}




