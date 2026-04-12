#include<iostream>
#include<string>
using namespace std;

class Car {
public:
    Car(string engine, string color, bool gps, bool sunroof) {
        this->engine = engine;
        this->color = color;
        this->gps = gps;
        this->sunroof = sunroof;
    }
    void describe() {
        cout<<engine<<" "<<color<<" "<<gps<<" "<<sunroof<<endl;
    }
private:
    string engine;
    string color;
    bool gps;
    bool sunroof;
};

class CarBuilder {
public:

    CarBuilder() {

    }

    CarBuilder& setEngine(const string &str) {
        this->engine = str;
        return *this;
    }

    CarBuilder& setColor(const string &str) {
        this->color = str;
        return *this;
    }

    CarBuilder& setGPS(bool gps) {
        this->gps = gps;
        return *this;
    }

    CarBuilder& setSunRoof(bool sunroof) {
        this->sunroof = sunroof;
        return *this;
    }

    Car build() {
        return Car(engine, color, gps, sunroof);
    }

private:
    string engine = "RedBull";
    string color = "Red";
    bool gps = false;
    bool sunroof = false;

};

int main() {    
    CarBuilder builder;
    Car car = builder.setEngine("V8").setSunRoof(true).build();
    car.describe();
}





