#include<iostream>
#include<string>
#include<memory>
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
    virtual unique_ptr<Document> create() = 0;
    virtual ~Factory() = default;
};

class WordFactory : public Factory {
public:
    unique_ptr<Document> create() override {
        return make_unique<Word>();
    }
};

class PDFFactory : public Factory {
public:
    unique_ptr<Document> create() override {
        return make_unique<PDF>();
    }
};


int main() {
    unique_ptr<Factory> factory = make_unique<WordFactory>();
    unique_ptr<Document> doc = factory->create();
    doc->open();
    factory = make_unique<PDFFactory>();
    doc = factory->create();
    doc->open();

}



