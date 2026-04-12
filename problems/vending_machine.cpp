#include<iostream>



class Product {
public:

private:
    int productId;
    int value;
};

struct Order {
    std::vector<int>productIds;
    int value;
    int orderId;
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
    std::unique_ptr<PaymentProcessor>paymentProcessor;

};


class Dispatcher {
public:
    bool dispatch(Order &order) {

    }
private:
};

class Inventory {
public:
    void addProduct(std::unique_ptr<Product> roduct) {

    }

    std::shared_ptr<Product> fetchProduct(int productId) {

    }

private:
    std::map<int,vector<std::shared_ptr<Product>>> products;
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