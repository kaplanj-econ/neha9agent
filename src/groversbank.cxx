#include "../headers/groversbank.hpp"


Groversbank::Groversbank() {
   
}

Groversbank::Groversbank(double profit,double hlbseverity,string behaviortype) {
    this->profit = profit;
    this->hlbseverity = hlbseverity;
    this->behaviortype = behaviortype;
}


void Groversbank::setgroverbankparameters(double profit,double hlbseverity,string behaviortype) {
        this->profit = profit;
        this->hlbseverity = hlbseverity;
        this->behaviortype = behaviortype;
}