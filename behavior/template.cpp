#include <iostream>
using namespace std;

/* Base */
class DataProcessor {
public:
    void run() {          // template method
        load();
        process();
        save();
    }

protected:
    void load() {
        cout << "loading data\n";
    }

    virtual void process() = 0;  // step to override

    void save() {
        cout << "saving results\n";
    }
};

/* Concrete */
class CSVProcessor : public DataProcessor {
protected:
    void process() override {
        cout << "processing CSV\n";
    }
};

class JSONProcessor : public DataProcessor {
protected:
    void process() override {
        cout << "processing JSON\n";
    }
};

int main() {
    CSVProcessor csv;
    JSONProcessor json;

    csv.run();
    json.run();
}
