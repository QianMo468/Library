#pragma once
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include "Book.h"
#include "Reader.h"
#include "BorrowRecord.h"
#include "User.h"
#include "Exceptions.h"
#include "DateUtils.h"

class Library {
public:
    Library(double baseFinePerDay = 1.0);
    ~Library();
    
    void clearInputBuffer();
    void printSectionHeader(const std::string& title);
    
    // 图书管理
    void addBook(Book* book);
    void removeBook(const std::string& title);
    
    // 读者管理
    void addReader(Reader* reader);
    void removeReader(const std::string& name);
    
    // 借阅功能
    void borrowBook(const std::string& bookTitle, const std::string& readerName);
    
    // 归还功能
    void returnBook(const std::string& bookTitle, const std::string& readerName);
    
    // 支付功能
    void payFine(const std::string& readerName, double amount = -1);
    
    // 显示功能
    void displayBooks() const;
    void displayReaders() const;
    void searchBook(const std::string& bookTitle) const;
    void searchReader(const std::string& readerName) const;
    void displayBorrowRecords() const;
    void displayOverdueBooks() const;
    void displayBooksDueSoon(int days = 3) const;
    
    // 数据持久化
    void saveData();
    void loadData();
    
    // 辅助方法
    Book* findBook(const std::string& title);
    Reader* findReader(const std::string& name);
    User* findUser(const std::string& username);
    int countBooks() const;
    int countReaders() const;
    int countBorrowedBooks() const;
    
    // 菜单系统
    void bookManagementMenu();
    void readerManagementMenu();
    void registerUser();
    void adminViewAllUsers();
    void adminDeleteUser();
    bool login();
    void mainMenu();

private:
    std::vector<Book*> books;
    std::vector<Reader*> readers;
    std::vector<BorrowRecord> borrowRecords;
    std::vector<std::unique_ptr<User>> users;
    User* currentUser = nullptr;
    double baseFinePerDay;
};