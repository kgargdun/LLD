#include <iostream>
#include <vector>
#include <string>
using namespace std;

class User;  // forward

class ChatRoom {
public:
    void add(User* u) { users.push_back(u); }

    void broadcast(const string& msg, User* sender);

private:
    vector<User*> users;
};

class User {
public:
    User(string name, ChatRoom* room)
        : name(name), room(room) {}

    void send(const string& msg) {
        room->broadcast(msg, this);
    }

    void receive(const string& msg) {
        cout << name << " received: " << msg << "\n";
    }

private:
    string name;
    ChatRoom* room;
};

void ChatRoom::broadcast(const string& msg, User* sender) {
    for (auto u : users) {
        if (u != sender)
            u->receive(msg);
    }
}

int main() {
    ChatRoom room;

    User a("Alice", &room);
    User b("Bob", &room);
    User c("Charlie", &room);

    room.add(&a);
    room.add(&b);
    room.add(&c);

    a.send("Hello everyone");
}
