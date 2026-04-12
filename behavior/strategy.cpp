

#include<iostream>
#include<memory>

class PaymentStrategy {
public:
    virtual void pay() = 0;
    virtual ~PaymentStrategy() = default;
};

class PaymentProcessor {
public:

    PaymentProcessor(std::unique_ptr<PaymentStrategy> paymentStrategy) {
        this->paymentStrategy = std::move(paymentStrategy);
    };
    void setStrategy(std::unique_ptr<PaymentStrategy> paymentStrategy) {
        this->paymentStrategy = std::move(paymentStrategy);
    }
    void process() {
        this->paymentStrategy->pay();
    }
private:
    std::unique_ptr<PaymentStrategy>paymentStrategy;
};

class UPIStrategy: public PaymentStrategy {
public:
    void pay() {
        std::cout<<"upi pay"<<std::endl;
    }
};

class CreditStrategy: public PaymentStrategy {
public:
    void pay() {
        std::cout<<"credit pay"<<std::endl;
    }
};

int main() {
    PaymentProcessor paymentProcessor(std::make_unique<UPIStrategy>());
    paymentProcessor.process();
    return 0;
}