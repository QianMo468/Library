#pragma once
#include <stdexcept>
#include <string>

// 异常类
class BookNotFoundException : public std::runtime_error {
public:
    BookNotFoundException(const std::string& message) : std::runtime_error(message) {}
};
class ReaderNotFoundException : public std::runtime_error {
public:
    ReaderNotFoundException(const std::string& message) : std::runtime_error(message) {}
};
class BookBorrowedException : public std::runtime_error {
public:
    BookBorrowedException(const std::string& message) : std::runtime_error(message) {}
};
class BookNotBorrowedException : public std::runtime_error {
public:
    BookNotBorrowedException(const std::string& message) : std::runtime_error(message) {}
};
class InvalidInputException : public std::runtime_error {
public:
    InvalidInputException(const std::string& message) : std::runtime_error(message) {}
};