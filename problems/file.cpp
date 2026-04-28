#include <bits/stdc++.h>
using namespace std;

class Node {
public:
    Node(string name) : name(name), parent(nullptr) {}
    virtual int getSize() = 0;
    virtual ~Node() = default;
    virtual bool isDirectory() = 0;

    const string& getName() const { return name; }
    void setParent(Node* p) { parent = p; }
    Node* getParent() { return parent; }

protected:
    string name;
    Node* parent;
};

class File : public Node {
public:
    File(string name) : Node(name) {}

    int getSize() override {
        return contents.size();
    }

    bool isDirectory() override {
        return false;
    }

    void write(const string& content) {
        contents = content;
    }

private:
    string contents;
};

class Directory : public Node {
public:
    Directory(string name) : Node(name) {}

    int getSize() override {
        int ret = 0;
        for (auto& [_, child] : children) {
            ret += child->getSize();
        }
        return ret;
    }

    bool add(shared_ptr<Node> node) {
        if (children.count(node->getName())) return false;
        node->setParent(this);
        children[node->getName()] = node;
        return true;
    }

    bool remove(const string& name) {
        if (!children.count(name)) return false;
        children.erase(name);
        return true;
    }

    bool isDirectory() override {
        return true;
    }

    Node* getChild(const string& name) {
        if (!children.count(name)) return nullptr;
        return children[name].get();
    }

private:
    unordered_map<string, shared_ptr<Node>> children;
};

// Forward declaration
class FileSystem;

class Command {
public:
    virtual bool execute() = 0;
    virtual void undo() = 0;
    virtual ~Command() = default;
};

class MkdirCommand : public Command {
public:
    MkdirCommand(FileSystem* fs, const string& path)
        : fs(fs), path(path) {}

    bool execute() override;

    void undo() override;

private:
    FileSystem* fs;
    string path;

    vector<pair<Directory*, string>> createdNodes;
};

class FileSystem {
public:
    FileSystem() {
        root = make_shared<Directory>("");
    }

    // Command-based mkdir
    bool mkdir(const string& path) {
        auto cmd = make_shared<MkdirCommand>(this, path);
        if (cmd->execute()) {
            history.push(cmd);
            return true;
        }
        return false;
    }

    bool undo() {
        if (history.empty()) return false;
        auto cmd = history.top();
        history.pop();
        cmd->undo();
        return true;
    }

    // Simple createFile (no command)
    bool createFile(const string& path) {
        vector<string> parts = split(path);
        if (parts.empty()) return false;

        Directory* curr = root.get();

        for (int i = 0; i < parts.size() - 1; i++) {
            Node* child = curr->getChild(parts[i]);
            if (child == nullptr || !child->isDirectory()) return false;
            curr = static_cast<Directory*>(child);
        }

        string fileName = parts.back();
        if (curr->getChild(fileName)) return false;

        auto file = make_shared<File>(fileName);
        curr->add(file);
        return true;
    }

private:
    shared_ptr<Directory> root;
    stack<shared_ptr<Command>> history;

    vector<string> split(const string& path) {
        vector<string> res;
        string temp;
        for (char c : path) {
            if (c == '/') {
                if (!temp.empty()) {
                    res.push_back(temp);
                    temp.clear();
                }
            } else {
                temp += c;
            }
        }
        if (!temp.empty()) res.push_back(temp);
        return res;
    }

    // Core mkdir logic used by command
    bool mkdirInternal(const string& path,
                       vector<pair<Directory*, string>>& createdNodes) {

        vector<string> parts = split(path);
        Directory* curr = root.get();

        for (const string& part : parts) {
            Node* child = curr->getChild(part);

            if (child == nullptr) {
                auto newDir = make_shared<Directory>(part);
                curr->add(newDir);

                createdNodes.push_back({curr, part});
                curr = newDir.get();
            } else {
                if (!child->isDirectory()) return false;
                curr = static_cast<Directory*>(child);
            }
        }
        return true;
    }

    friend class MkdirCommand;
};

// ---- MkdirCommand definitions ----

bool MkdirCommand::execute() {
    createdNodes.clear();
    return fs->mkdirInternal(path, createdNodes);
}

void MkdirCommand::undo() {
    for (int i = createdNodes.size() - 1; i >= 0; i--) {
        auto [parent, name] = createdNodes[i];
        parent->remove(name);
    }
}