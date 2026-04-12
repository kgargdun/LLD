#include<iostream>
#include<string>
#include<memory>
#include<map>
#include<functional>
using namespace std;


class Document {
public:
    virtual void open() = 0;
    virtual ~Document() = default;
};

class PDF : public Document {
public:
    void open() override {
        cout<<"open pdf"<<endl;
    }
};

class Word : public Document {
public:
    void open() override {
        cout<<"open word"<<endl;
    }
};

class Factory
{
public:
   using Creator = function<unique_ptr<Document>()>; 
    void add(const string  &type, Creator creator) {
        registry[type] = creator;
    }
    unique_ptr<Document> create(const string &type) {
        if(registry.find(type)==registry.end()) return nullptr;
        return registry[type]();
    }
private:
    map<string,Creator> registry;
};


int main() {
    unique_ptr<Factory> factory = make_unique<Factory>();
    factory->add("pdf",[]() {
        return make_unique<PDF>();
    });
    unique_ptr<Document> doc = factory->create("pdf");
    doc->open();

}



