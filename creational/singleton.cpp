#include<iostream>
#include<string>
using namespace std;

class Logger {
public:
    static Logger &getInstance() {
        static Logger logger;
        return logger;
    }

    void log(string msg) {
        cout<<" "<<msg<<endl;
    }

   
private:
    Logger() {};
    Logger (const Logger &logger) = delete;
    Logger& operator=(const Logger &logger) = delete;

};

int main() {
    Logger &logger = Logger::getInstance();
    logger.log("hello");

}






