#pragma once
#include <string>
#include <vector>
#include "Reader.h" 
class User {
public:
    User(const std::string& username, const std::string& password);
    virtual ~User() = default;
    std::string getUsername() const { return username; }
    bool verifyPassword(const std::string& inputPassword) const;
    virtual bool isAdmin() const { return false; }

private:
    std::string username;
    std::string password;
};

// 管理员类
class Administrator : public User {
public:
    Administrator(const std::string& username, const std::string& password);
    bool isAdmin() const override { return true; }
};

// 读者用户类
class ReaderUser : public User {
public:
    ReaderUser(const std::string& username, const std::string& password, Reader* reader);
    Reader* getReader() const { return reader; }

private:
    Reader* reader;
};