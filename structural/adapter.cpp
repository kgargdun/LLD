#include<iostream>
using namespace std;

class Stripe {
public:
    void makePayment(int amountCents) {
        cout << "Stripe processed " << amountCents << " cents\n";
    }
};

class GPay {
public:
    void payTo(int amount, bool isSecure) {
        cout << "Gpay processed " << amount << " dollars\n";
    }
};

class PaymentProcessor {
public:
    virtual void pay(int amount)= 0;
    virtual ~PaymentProcessor() = default;
};

class StripeAdapter : public PaymentProcessor {
public:
    StripeAdapter(Stripe& stripe) : stripe(stripe) {

    }
    void pay(int amount) override {
        stripe.makePayment(amount*100);
    }
private:
    Stripe& stripe;
};



class GPayAdapter : public PaymentProcessor {
public:
    GPayAdapter(GPay &gPay) : gPay(gPay) {

    }
     void pay(int amount) override {
        gPay.payTo(amount, true);
    }
private:
    GPay& gPay;
};


int main() {
    Stripe stripe;
    GPay gPay;
    GPayAdapter gPayAdapter(gPay);
    StripeAdapter stripeAdapter(stripe);
    gPayAdapter.pay(100);
    stripeAdapter.pay(25);

}


