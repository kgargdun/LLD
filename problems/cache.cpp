#include <iostream>
#include <unordered_map>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>

using namespace std;

// -------------------- Node --------------------

struct Node {
    string value;
    chrono::steady_clock::time_point expiry;

    Node() {}
    Node(string v, chrono::steady_clock::time_point exp)
        : value(v), expiry(exp) {}
};

// -------------------- Write Policy --------------------

class WritePolicy {
public:
    virtual void write(const string& key, const string& value) = 0;
    virtual ~WritePolicy() = default;
};

// Dummy DB + write-through

class DummyDB {
public:
    void write(const string& key, const string& value) {
        this_thread::sleep_for(chrono::milliseconds(5));
    }
};

class WriteThroughPolicy : public WritePolicy {
    DummyDB& db;

public:
    WriteThroughPolicy(DummyDB& db) : db(db) {}

    void write(const string& key, const string& value) override {
        db.write(key, value);
    }
};

// -------------------- Eviction Policy --------------------

class EvictionPolicy {
public:
    virtual void onAccess(const string& key) = 0;
    virtual void onDelete(const string& key) = 0;
    virtual string evict() = 0;

    virtual ~EvictionPolicy() = default;
};

// -------------------- LRU --------------------

class LRUEvictionPolicy : public EvictionPolicy {
private:
    struct DLLNode {
        string key;
        DLLNode* prev;
        DLLNode* next;

        DLLNode(const string& k) : key(k), prev(nullptr), next(nullptr) {}
    };

    unordered_map<string, DLLNode*> map;
    DLLNode* head = nullptr;
    DLLNode* tail = nullptr;

    void addToHead(DLLNode* node) {
        node->next = head;
        node->prev = nullptr;
        if (head) head->prev = node;
        head = node;
        if (!tail) tail = node;
    }

    void removeNode(DLLNode* node) {
        if (node->prev) node->prev->next = node->next;
        else head = node->next;

        if (node->next) node->next->prev = node->prev;
        else tail = node->prev;
    }

    void moveToHead(DLLNode* node) {
        removeNode(node);
        addToHead(node);
    }

public:
    void onAccess(const string& key) override {
        if (map.count(key)) {
            moveToHead(map[key]);
        } else {
            DLLNode* node = new DLLNode(key);
            addToHead(node);
            map[key] = node;
        }
    }

    void onDelete(const string& key) override {
        if (!map.count(key)) return;

        DLLNode* node = map[key];
        removeNode(node);
        map.erase(key);
        delete node;
    }

    string evict() override {
        if (!tail) return "";

        string key = tail->key;
        DLLNode* node = tail;

        removeNode(node);
        map.erase(key);
        delete node;

        return key;
    }
};

// -------------------- Cache --------------------

class Cache {
private:
    unordered_map<string, Node> store;

    size_t capacity;

    unique_ptr<EvictionPolicy> eviction;
    unique_ptr<WritePolicy> writePolicy;

    mutex mtx;

    thread cleaner;
    atomic<bool> stop{false};

private:
    bool isExpired(const Node& node) {
        return chrono::steady_clock::now() > node.expiry;
    }

    void expiryWorker() {
        while (!stop) {
            this_thread::sleep_for(chrono::seconds(1));

            lock_guard<mutex> lock(mtx);

            for (auto it = store.begin(); it != store.end();) {
                if (isExpired(it->second)) {
                    cout << "[EXP] key=" << it->first << endl;

                    eviction->onDelete(it->first);
                    it = store.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

public:
    Cache(size_t cap,
          unique_ptr<EvictionPolicy> evict,
          unique_ptr<WritePolicy> write)
        : capacity(cap),
          eviction(move(evict)),
          writePolicy(move(write)) {

        cleaner = thread(&Cache::expiryWorker, this);
    }

    ~Cache() {
        stop = true;
        if (cleaner.joinable()) cleaner.join();
    }

    string get(const string& key) {
        lock_guard<mutex> lock(mtx);

        if (!store.count(key)) {
            cout << "[GET MISS] " << key << endl;
            return "";
        }

        if (isExpired(store[key])) {
            cout << "[GET EXPIRED] " << key << endl;

            eviction->onDelete(key);
            store.erase(key);
            return "";
        }

        eviction->onAccess(key);
        cout << "[GET HIT] " << key << endl;

        return store[key].value;
    }

    void put(const string& key, const string& value, int ttlSeconds) {
        writePolicy->write(key, value);

        auto expiryTime = chrono::steady_clock::now() + chrono::seconds(ttlSeconds);

        lock_guard<mutex> lock(mtx);

        store[key] = Node(value, expiryTime);
        eviction->onAccess(key);

        cout << "[PUT] " << key << endl;

        if (store.size() > capacity) {
            string victim = eviction->evict();

            if (!victim.empty()) {
                store.erase(victim);
                cout << "[EVICT] " << victim << endl;
            }
        }
    }
};

// -------------------- Driver --------------------

int main() {
    DummyDB db;

    auto writePolicy = make_unique<WriteThroughPolicy>(db);
    auto evictionPolicy = make_unique<LRUEvictionPolicy>();

    Cache cache(3, move(evictionPolicy), move(writePolicy));

    cache.put("a", "1", 2);
    cache.put("b", "2", 5);
    cache.put("c", "3", 5);

    cout << cache.get("a") << endl;

    this_thread::sleep_for(chrono::seconds(3));

    cout << cache.get("a") << endl; // expired

    cache.put("d", "4", 5); // triggers eviction

    cout << cache.get("b") << endl;
    cout << cache.get("c") << endl;
    cout << cache.get("d") << endl;

    return 0;
}