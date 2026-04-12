
#include<iostream>
using namespace std;

class Visitor;


class Shape {
public:
    virtual void accept(Visitor& visitor) = 0;
    virtual ~Shape() = default;
};

class Square : public Shape {
public:
    Square(int side) : side(side) {}
    void accept(Visitor& visitor) override;
    int side;

};

class Circle : public Shape {
public:
    Circle(int radius) : radius(radius) {}
    void accept(Visitor& visitor) override;
    int radius;
};

class Visitor {
public:
    virtual void visit(Square &) = 0;
    virtual void visit(Circle &) = 0;
    virtual ~Visitor() = default;
};

class AreaVisitor : public Visitor {
public:
    void visit(Square &sq) override {
        cout<<"Area is "<<sq.side*sq.side<<endl;
    }
    void visit(Circle &clc) override {
        cout<<"Area is "<<3.14*clc.radius*clc.radius<<endl;
    }
};

class PerimeterVisitor : public Visitor {
public:
     void visit(Square &sq) override {
        cout<<"Perimeter is "<<sq.side*4<<endl;
    }
    void visit(Circle &clc) override {
        cout<<"Perimeter is "<<3.14*2*clc.radius<<endl;
    }

};

void Square::accept(Visitor &visitor) {
    visitor.visit(*this);
}

void Circle::accept(Visitor &visitor)  {
    visitor.visit(*this);
}



int main() {
    Shape *shape = new Circle(12);
    Visitor *visitor = new PerimeterVisitor();
    shape->accept(*visitor);
}

