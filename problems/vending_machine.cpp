#include<iostream>


class Product {
public:

private:
    int productId;
    int value;
};

class Order {
private:
    int productId;
    int orderId;
};

struct ProductStock {
    int productId;
    int count;
};


class Inventory {
public:
    void addProduct(std::unique_ptr<Product> roduct) {

    }

    std::shared_ptr<Product> fetchProduct(int productId) {

    }

private:
    std::map<int,std::shared_ptr<ProductStock>> stock;
};
 


class PaymentStrategy {
    virtual void pay(int amount) = 0;
    virtual ~PaymentProcessor() = default;
};

class UPIPaymentStrategy : public PaymentStrategy {
    void pay(int amount) {

    }
};

class PaymentProcessor {
public:
    void pay(int amount);
private:
    std::unique_ptr<PaymentStrategy>paymentStrategy;

};




class VendingMachineState {
public:

    virtual ~State() = default;

    virtual void placeOrder();



};


class IdleState : public VendingMachineState {

};

class ProcessingState : public VendingMachineState {

};

class DispatchState : public VendingMachineState {

};

class MaintenanceState : public VendingMachineState {

};

class VendingMachine {
public:

private:
    VendingMachineState *state;
};




int main() {




}