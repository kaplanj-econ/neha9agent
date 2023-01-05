#ifndef GROVERSBANK_HPP
#define GROVERSBANK_HPP
#include <vector>
#include <sstream>
using namespace std;

// This class is used to keep the information of all the grovers

class Groversbank;

class Groversbank {
private:
  double profit;
  double hlbseverity;
  string behaviortype;

  

public:

    Groversbank();
    Groversbank(double profit,double hlbseverity,string behaviortype);

    double getgroversbankprofit() { return this->profit; }
    double getgroversbankhlbseverity() { return this->hlbseverity; }
    string getgroversbankbehaviortype() { return this->behaviortype; }

    void setgroverbankparameters(double,double,string);


};


#endif