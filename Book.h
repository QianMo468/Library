#pragma once
#include <string>

class Book {
public:
    Book(const std::string& title, const std::string& author, const std::string& type = "普通图书");
    virtual ~Book() = default;
    
    // Getter方法
    std::string getTitle() const { return title; }
    std::string getAuthor() const { return author; }
    std::string getType() const { return type; }
    bool isBorrowedStatus() const { return isBorrowed; }
    
    // Setter方法
    void setTitle(const std::string& newTitle) { title = newTitle; }
    void setAuthor(const std::string& newAuthor) { author = newAuthor; }
    
    // 操作方法
    void borrow() { isBorrowed = true; }
    void returnBook() { isBorrowed = false; }
    virtual double getFinePerDay() const { return 1.0; }

private:
    std::string title;
    std::string author;
    std::string type;
    bool isBorrowed;
};

// 教科书类
class Textbook : public Book {
public:
    Textbook(const std::string& title, const std::string& author);
    double getFinePerDay() const override { return 2.0; }
};

// 小说类
class Novel : public Book {
public:
    Novel(const std::string& title, const std::string& author);
};

// 杂志类
class Magazine : public Book {
public:
    Magazine(const std::string& title, const std::string& author);
    double getFinePerDay() const override { return 0.5; }
};