#include "Reader.h"

Reader::Reader(const std::string& name, int borrowPeriod, double fine)
    : name(name), borrowPeriod(borrowPeriod), fine(fine) {}

void Reader::addFine(double amount) {
    if (amount < 0) throw InvalidInputException("罚款金额不能为负数");
    fine += amount;
}

void Reader::payFine(double amount) {
    if (amount < 0) throw InvalidInputException("支付金额不能为负数");
    if (amount > fine) throw InvalidInputException("支付金额不能超过欠款");
    fine -= amount;
}