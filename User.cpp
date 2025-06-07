#include "User.h"

User::User(const std::string& username, const std::string& password)
    : username(username), password(password) {}

bool User::verifyPassword(const std::string& inputPassword) const {
    return password == inputPassword;
}

Administrator::Administrator(const std::string& username, const std::string& password)
    : User(username, password) {}

ReaderUser::ReaderUser(const std::string& username, const std::string& password, Reader* reader)
    : User(username, password), reader(reader) {}