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
#include <source_location>

using namespace std;

enum class LogLevel {
    VERBOSE = 0, DEBUG, WARN, ERROR
};

class LogEvent {
public:
    string message;
    LogLevel level;
    std::source_location location;
    string loggerName;
    chrono::system_clock::time_point timestamp;
};

class LogWriter {
public:
    virtual void write(const LogEvent& event) = 0;
    virtual ~LogWriter() = default;
};

class ConsoleLogWriter : public LogWriter {
public:
    void write(const LogEvent& event) override {
        cout << "[" << event.loggerName << "] "
             << event.message << endl;
    }
};

class FileLogWriter : public LogWriter {
public:
    FileLogWriter(const string& path) : path(path) {}

    void write(const LogEvent& event) override {
        ofstream out(path, ios::app);
        out << "[" << event.loggerName << "] "
            << event.message << endl;
    }

private:
    string path;
};

class QueueWorker {
public:
    QueueWorker(shared_ptr<LogWriter> writer)
        : writer(writer), stop(false) {
        workerThread = thread([this]() { run(); });
    }

    ~QueueWorker() {
        {
            unique_lock<mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        if (workerThread.joinable()) workerThread.join();
    }

    void enqueue(const LogEvent& event) {
        {
            lock_guard<mutex> lock(mtx);
            q.push(event);
        }
        cv.notify_one();
    }

private:
    void run() {
        while (true) {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [&]() { return stop || !q.empty(); });

            if (stop && q.empty()) return;

            auto event = q.front();
            q.pop();
            lock.unlock();

            writer->write(event);
        }
    }

private:
    queue<LogEvent> q;
    mutex mtx;
    condition_variable cv;
    shared_ptr<LogWriter> writer;
    thread workerThread;
    bool stop;
};

class Dispatcher {
public:
    void dispatch(const LogEvent& event) {
        if (!mapping.count(event.loggerName)) return;

        for (auto& worker : mapping[event.loggerName]) {
            worker->enqueue(event);
        }
    }

    void addFileWriter(const string& tag, const string& path) {
        auto writer = make_shared<FileLogWriter>(path);
        mapping[tag].push_back(make_shared<QueueWorker>(writer));
    }

    void addConsoleWriter(const string& tag) {
        auto writer = make_shared<ConsoleLogWriter>();
        mapping[tag].push_back(make_shared<QueueWorker>(writer));
    }

private:
    unordered_map<string, vector<shared_ptr<QueueWorker>>> mapping;
};

class Logger {
public:
    Logger(const string& tag, shared_ptr<Dispatcher> dispatcher)
        : tag(tag), dispatcher(dispatcher) {}

    void log(const string& msg, LogLevel level,
             std::source_location loc = std::source_location::current()) {

        LogEvent event;
        event.message = msg;
        event.level = level;
        event.location = loc;
        event.loggerName = tag;
        event.timestamp = chrono::system_clock::now();

        dispatcher->dispatch(event);
    }

private:
    string tag;
    shared_ptr<Dispatcher> dispatcher;
};

class LoggerManager {
public:
    static LoggerManager& getInstance() {
        static LoggerManager instance;
        return instance;
    }

    shared_ptr<Logger> getLogger(const string& tag) {
        if (loggers.count(tag)) return loggers[tag];

        auto logger = make_shared<Logger>(tag, dispatcher);
        loggers[tag] = logger;
        return logger;
    }

    void addFileWriter(const string& tag, const string& path) {
        dispatcher->addFileWriter(tag, path);
    }

    void addConsoleWriter(const string& tag) {
        dispatcher->addConsoleWriter(tag);
    }

private:
    LoggerManager() {
        dispatcher = make_shared<Dispatcher>();
    }

    unordered_map<string, shared_ptr<Logger>> loggers;
    shared_ptr<Dispatcher> dispatcher;
};



int main() {
    auto& mgr = LoggerManager::getInstance();

    // configure
    mgr.addConsoleWriter("auth");
    mgr.addFileWriter("auth", "auth.log");

    mgr.addConsoleWriter("db");
    mgr.addFileWriter("db", "db.log");

    // get loggers
    auto authLogger = mgr.getLogger("auth");
    auto dbLogger = mgr.getLogger("db");

    // single-thread logs
    authLogger->log("user login", LogLevel::INFO);
    dbLogger->log("db connected", LogLevel::DEBUG);

    // multi-thread test
    thread t1([&]() {
        for (int i = 0; i < 5; i++) {
            authLogger->log("auth thread log " + to_string(i), LogLevel::DEBUG);
        }
    });

    thread t2([&]() {
        for (int i = 0; i < 5; i++) {
            dbLogger->log("db thread log " + to_string(i), LogLevel::DEBUG);
        }
    });

    t1.join();
    t2.join();

    // give some time for async workers to flush
    this_thread::sleep_for(chrono::seconds(1));

    return 0;
}