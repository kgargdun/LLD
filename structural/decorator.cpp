#include <iostream>
#include <memory>
#include <string>

class Coffee {
public:
    virtual std::string getDesc() const = 0;
    virtual int getCost() const = 0;
    virtual ~Coffee() = default;
};

// Concrete components
class Latte : public Coffee {
public:
    std::string getDesc() const override {
        return "Latte";
    }

    int getCost() const override {
        return 25;
    }
};

class Frappe : public Coffee {
public:
    std::string getDesc() const override {
        return "Frappe";
    }

    int getCost() const override {
        return 40;
    }
};

// Base decorator
class CoffeeDecorator : public Coffee {
public:
    explicit CoffeeDecorator(std::unique_ptr<Coffee> c)
        : coffee(std::move(c)) {}

    std::string getDesc() const override {
        return coffee->getDesc();
    }

    int getCost() const override {
        return coffee->getCost();
    }

    virtual ~CoffeeDecorator() = default;

protected:
    std::unique_ptr<Coffee> coffee;
};

// Concrete decorators
class MilkDecorator : public CoffeeDecorator {
public:
    using CoffeeDecorator::CoffeeDecorator;

    std::string getDesc() const override {
        return coffee->getDesc() + " + Milk";
    }

    int getCost() const override {
        return coffee->getCost() + 15;
    }
};

class ChocoDecorator : public CoffeeDecorator {
public:
    using CoffeeDecorator::CoffeeDecorator;

    std::string getDesc() const override {
        return coffee->getDesc() + " + Choco";
    }

    int getCost() const override {
        return coffee->getCost() + 50;
    }
};

class FrothDecorator : public CoffeeDecorator {
public:
    using CoffeeDecorator::CoffeeDecorator;

    std::string getDesc() const override {
        return coffee->getDesc() + " + Froth";
    }

    int getCost() const override {
        return coffee->getCost() + 5;
    }
};

int main() {
    std::unique_ptr<Coffee> coffee =
        std::make_unique<FrothDecorator>(
            std::make_unique<MilkDecorator>(
                std::make_unique<Latte>()
            )
        );

    std::cout << coffee->getDesc() << "\n";
    std::cout << coffee->getCost() << "\n";
}