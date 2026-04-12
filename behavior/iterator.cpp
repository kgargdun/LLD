#include <iostream>
#include <vector>
using namespace std;

/* Iterator */
class Iterator {
public:
    virtual bool hasNext() = 0;
    virtual int next() = 0;
    virtual ~Iterator() = default;
};

/* Collection */
class IntArray {
public:
    IntArray(initializer_list<int> list) : data(list) {}

    class ArrayIterator : public Iterator {
    public:
        ArrayIterator(const vector<int>& d) : data(d) {}

        bool hasNext() override {
            return index < data.size();
        }

        int next() override {
            return data[index++];
        }

    private:
        const vector<int>& data;
        size_t index = 0;
    };

    Iterator* createIterator() const {
        return new ArrayIterator(data);
    }

private:
    vector<int> data;
};

int main() {
    IntArray arr{1,2,3};

    Iterator* it = arr.createIterator();

    while (it->hasNext())
        cout << it->next() << " ";
}
