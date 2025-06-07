#pragma once
#include <string>
#include <vector>
#include "Exceptions.h"

class Reader {
public:
    Reader(const std::string& name, int borrowPeriod, double fine = 0.0);
    virtual ~Reader() = default;
    
    // Getter方法
    std::string getName() const { return name; }
    int getBorrowPeriod() const { return borrowPeriod; }
    double getFine() const { return fine; }
    
    // 罚款操作
    void addFine(double amount);
    void payFine(double amount);
    void payFullFine() { fine = 0.0; }
    
    // 虚函数
    virtual double getFineDiscount() const { return 1.0; }
    virtual std::string getTypeName() const { return "普通会员"; }

protected:
    std::string name;
    int borrowPeriod;
    double fine;
};

// 普通会员类
class RegularMember : public Reader {
public:
    RegularMember(const std::string& name) : Reader(name, 30) {}
};

// VIP会员类
class VIPMember : public Reader {
public:
    VIPMember(const std::string& name) : Reader(name, 60) {}
    double getFineDiscount() const override { return 0.9; }
    std::string getTypeName() const override { return "VIP会员"; }
};

// 学生会员类
class StudentMember : public Reader {
public:
    StudentMember(const std::string& name) : Reader(name, 45) {}
    double getFineDiscount() const override { return 0.8; }
    std::string getTypeName() const override { return "学生会员"; }
};