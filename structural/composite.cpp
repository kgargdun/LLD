#include<iostream>
using namespace std;

class FSNode {
public:
    virtual int getSize() = 0;
    virtual ~FSNode() = default;
};

class File : public FSNode {
public:
    File(int size) : size(size) {

    }
    int getSize() override {
        return size;
    }
private:
    int size;
};

class Folder : public FSNode {
public:

    Folder(initializer_list<shared_ptr<FSNode>> contents) : contents(contents) {
    }

    int getSize() override {
        int ret = 0;
        for(auto content: contents) {
            ret += content->getSize();
        }
        return ret;
    }
    void add(shared_ptr<FSNode> content) {
        contents.push_back(content);
    }
private:
    vector<shared_ptr<FSNode>> contents;
};


int main() {
    shared_ptr<File> f1 = make_shared<File>(250);
    shared_ptr<File> f2 = make_shared<File>(92);
    shared_ptr<File> f3 = make_shared<File>(305);

    shared_ptr<Folder> dir = make_shared<Folder>(initializer_list<shared_ptr<FSNode>>{f1,f2,f3});
    cout<<dir->getSize()<<endl;
    
}

