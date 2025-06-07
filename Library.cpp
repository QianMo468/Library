#include "Library.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric>
#include <algorithm>
#include <iomanip>

// 构造函数
Library::Library(double baseFinePerDay) : baseFinePerDay(baseFinePerDay) {
    loadData();
    // 添加默认管理员
    if (findUser("admin") == nullptr) {
        users.push_back(std::make_unique<Administrator>("admin", "admin123"));
    }
}

// 析构函数
Library::~Library() {
    saveData();
    for (auto book : books) delete book;
    for (auto reader : readers) delete reader;
}

// 清除输入缓冲区
void Library::clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// 打印标题
void Library::printSectionHeader(const std::string& title) {
    std::cout << "\n\033[1;36m========== " << title << " ==========\033[0m\n";
}

// 图书管理
void Library::addBook(Book* book) { books.push_back(book); }

void Library::removeBook(const std::string& title) {
    auto it = std::remove_if(books.begin(), books.end(), 
        [&](Book* b) { return b->getTitle() == title; });
    if (it == books.end()) {
        throw BookNotFoundException("未找到图书: " + title);
    }
    books.erase(it, books.end());
}

// 读者管理
void Library::addReader(Reader* reader) { readers.push_back(reader); }

void Library::removeReader(const std::string& name) {
    auto it = std::remove_if(readers.begin(), readers.end(), 
        [&](Reader* r) { return r->getName() == name; });
    if (it == readers.end()) {
        throw ReaderNotFoundException("未找到读者: " + name);
    }
    readers.erase(it, readers.end());
}

// 借阅功能
void Library::borrowBook(const std::string& bookTitle, const std::string& readerName) {
    Book* book = findBook(bookTitle);
    Reader* reader = findReader(readerName);
    if (!book) throw BookNotFoundException("未找到图书: " + bookTitle);
    if (!reader) throw ReaderNotFoundException("未找到读者: " + readerName);
    if (book->isBorrowedStatus()) throw BookBorrowedException("图书已被借出: " + bookTitle);
    if (reader->getFine() > 0) {
        std::cout << "\033[1;33m警告: 该读者有未支付的罚款 " << reader->getFine() << " 元，可能影响借阅权限\033[0m\n";
    }
    book->borrow();
    std::time_t now = DateUtils::getCurrentTime();
    borrowRecords.emplace_back(book, reader, now, now + reader->getBorrowPeriod() * 24 * 60 * 60);
    std::cout << "📅 应还日期: " << DateUtils::formatTime(borrowRecords.back().getDueDate()) << "\n";
}

// 归还功能
void Library::returnBook(const std::string& bookTitle, const std::string& readerName) {
    Book* book = findBook(bookTitle);
    Reader* reader = findReader(readerName);
    if (!book) throw BookNotFoundException("未找到图书: " + bookTitle);
    if (!reader) throw ReaderNotFoundException("未找到读者: " + readerName);
    bool foundRecord = false;
    for (auto& record : borrowRecords) {
        if (record.getBook() == book && record.getReader() == reader && !record.getIsReturned()) {
            std::time_t now = DateUtils::getCurrentTime();
            record.setReturnDate(now);
            book->returnBook();
            int overdueDays = record.getOverdueDays();
            if (overdueDays > 0) {
                double fine = record.calculateFine();
                reader->addFine(fine);
                std::cout << "⏰ 超期 " << overdueDays << " 天，";
                std::cout << "图书类型: " << book->getType() << "，";
                std::cout << "罚款标准: " << book->getFinePerDay() << "元/天，";
                std::cout << "读者折扣: " << reader->getFineDiscount() * 100 << "%，";
                std::cout << "需缴纳罚款: \033[1;31m" << fine << "\033[0m 元。\n";
            } else {
                std::cout << "✅ 按时归还，感谢！\n";
            }
            foundRecord = true;
            break;
        }
    }
    if (!foundRecord) throw BookNotBorrowedException("未找到借阅记录: " + bookTitle + " 由 " + readerName + " 借阅");
}

// 支付功能
void Library::payFine(const std::string& readerName, double amount) {
    Reader* reader = findReader(readerName);
    if (!reader) throw ReaderNotFoundException("未找到读者: " + readerName);
    double currentFine = reader->getFine();
    if (currentFine <= 0) {
        std::cout << "✅ 该读者没有未支付的罚款\n";
        return;
    }
    if (amount < 0) {
        reader->payFullFine();
        std::cout << "✅ 已全额支付罚款: " << currentFine << " 元\n";
    } else {
        if (amount > currentFine) {
            std::cout << "⚠️ 支付金额超过欠款，将支付全部欠款: " << currentFine << " 元\n";
            reader->payFullFine();
        } else {
            reader->payFine(amount);
            std::cout << "✅ 已支付罚款: " << amount << " 元，剩余欠款: " << reader->getFine() << " 元\n";
        }
    }
}

// 显示功能
void Library::displayBooks() const {
    std::cout << "📚 图书列表：\n";
    for (const auto& book : books) {
        std::cout << "书名: " << book->getTitle()
            << ", 作者: " << book->getAuthor()
            << ", 类型: " << book->getType()
            << ", 罚款标准: " << book->getFinePerDay() << "元/天"
            << ", 状态: " << (book->isBorrowedStatus() ? "\033[1;31m已借出\033[0m" : "\033[1;32m可借阅\033[0m") << std::endl;
    }
}

void Library::displayReaders() const {
    std::cout << "👥 读者列表：\n";
    for (const auto& reader : readers) {
        std::cout << "姓名: " << reader->getName()
            << ", 类型: " << reader->getTypeName()
            << ", 借阅期限: \033[1;33m" << reader->getBorrowPeriod()
            << "\033[0m 天, 罚款: \033[1;31m" << reader->getFine() << "\033[0m 元\n";
    }
}

void Library::searchBook(const std::string& bookTitle) const {
    bool found = false;
    for (const auto& book : books) {
        if (book->getTitle() == bookTitle) {
            std::cout << "书名: \033[1;33m" << book->getTitle()
                << "\033[0m, 作者: \033[1;33m" << book->getAuthor()
                << "\033[0m, 类型: \033[1;33m" << book->getType()
                << "\033[0m, 罚款标准: " << book->getFinePerDay() << "元/天"
                << ", 状态: " << (book->isBorrowedStatus() ? "\033[1;31m已借出\033[0m" : "\033[1;32m可借阅\033[0m") << std::endl;
            std::cout << "借阅记录：\n";
            bool hasRecord = false;
            for (const auto& record : borrowRecords) {
                if (record.getBook() == book) {
                    record.display();
                    hasRecord = true;
                }
            }
            if (!hasRecord) std::cout << "暂无借阅记录\n";
            found = true;
        }
    }
    if (!found) std::cout << "\033[1;31m未找到相关图书\033[0m\n";
}

void Library::searchReader(const std::string& readerName) const {
    bool found = false;
    for (const auto& reader : readers) {
        if (reader->getName() == readerName) {
            std::cout << "姓名: \033[1;33m" << reader->getName()
                << "\033[0m, 类型: \033[1;33m" << reader->getTypeName()
                << "\033[0m, 借阅期限: \033[1;33m" << reader->getBorrowPeriod()
                << "\033[0m 天, 罚款: \033[1;31m" << reader->getFine() << "\033[0m 元\n";
            std::cout << "借阅记录：\n";
            bool hasRecord = false;
            for (const auto& record : borrowRecords) {
                if (record.getReader() == reader) {
                    record.display();
                    hasRecord = true;
                }
            }
            if (!hasRecord) std::cout << "暂无借阅记录\n";
            found = true;
        }
    }
    if (!found) std::cout << "\033[1;31m未找到相关读者\033[0m\n";
}

void Library::displayBorrowRecords() const {
    std::cout << "📜 所有借阅记录：\n";
    for (const auto& record : borrowRecords) {
        record.display();
    }
}

void Library::displayOverdueBooks() const {
    std::cout << "⚠️ 超期未还图书：\n";
    bool hasOverdue = false;
    for (const auto& record : borrowRecords) {
        if (!record.getIsReturned() && record.getOverdueDays() > 0) {
            std::cout << "书名: " << record.getBook()->getTitle()
                << ", 读者: " << record.getReader()->getName()
                << ", 超期: " << record.getOverdueDays() << "天"
                << ", 罚款: " << record.calculateFine() << "元\n";
            hasOverdue = true;
        }
    }
    if (!hasOverdue) std::cout << "所有图书均按时归还\n";
}

void Library::displayBooksDueSoon(int days) const {
    std::cout << "📅 即将到期的图书（" << days << "天内）：\n";
    bool hasDueSoon = false;
    std::time_t now = DateUtils::getCurrentTime();
    for (const auto& record : borrowRecords) {
        if (!record.getIsReturned()) {
            int daysLeft = (record.getDueDate() - now) / (24 * 60 * 60);
            if (daysLeft >= 0 && daysLeft <= days) {
                std::cout << "书名: " << record.getBook()->getTitle()
                    << ", 读者: " << record.getReader()->getName()
                    << ", 剩余天数: " << daysLeft << "天\n";
                hasDueSoon = true;
            }
        }
    }
    if (!hasDueSoon) std::cout << "没有即将到期的图书\n";
}

// 数据持久化
void Library::saveData() {
    std::ofstream bookFile("books.txt");
    if (bookFile.is_open()) {
        for (const auto& book : books) {
            bookFile << book->getType() << "," << book->getTitle() << ","
                << book->getAuthor() << "," << book->isBorrowedStatus() << "\n";
        }
        bookFile.close();
    }

    std::ofstream readerFile("readers.txt");
    if (readerFile.is_open()) {
        for (const auto& reader : readers) {
            std::string type;
            if (dynamic_cast<RegularMember*>(reader)) type = "RegularMember";
            else if (dynamic_cast<VIPMember*>(reader)) type = "VIPMember";
            else if (dynamic_cast<StudentMember*>(reader)) type = "StudentMember";
            else type = "RegularMember";
            readerFile << type << "," << reader->getName() << ","
                << reader->getBorrowPeriod() << "," << reader->getFine() << "\n";
        }
        readerFile.close();
    }

    std::ofstream recordFile("records.txt");
    if (recordFile.is_open()) {
        for (const auto& record : borrowRecords) {
            recordFile << record.getBook()->getTitle() << "," << record.getReader()->getName()
                << "," << record.getBorrowDate() << "," << record.getDueDate()
                << "," << record.getReturnDate() << "," << record.getIsReturned() << "\n";
        }
        recordFile.close();
    }

    std::ofstream userFile("users.txt");
    if (userFile.is_open()) {
        for (const auto& user : users) {
            if (dynamic_cast<Administrator*>(user.get())) {
                userFile << "Administrator," << user->getUsername() << "," << user->getUsername() << "\n";
            } else if (auto readerUser = dynamic_cast<ReaderUser*>(user.get())) {
                userFile << "ReaderUser," << user->getUsername() << "," << user->getUsername() << "," << readerUser->getReader()->getName() << "\n";
            }
        }
        userFile.close();
    }
}

void Library::loadData() {
    std::ifstream bookFile("books.txt");
    if (bookFile.is_open()) {
        std::string line;
        while (std::getline(bookFile, line)) {
            size_t pos1 = line.find(',');
            size_t pos2 = line.find(',', pos1 + 1);
            size_t pos3 = line.find(',', pos2 + 1);
            std::string type = line.substr(0, pos1);
            std::string title = line.substr(pos1 + 1, pos2 - pos1 - 1);
            std::string author = line.substr(pos2 + 1, pos3 - pos2 - 1);
            bool isBorrowed = (line.substr(pos3 + 1) == "1");
            Book* book;
            if (type == "教科书") book = new Textbook(title, author);
            else if (type == "小说") book = new Novel(title, author);
            else if (type == "杂志") book = new Magazine(title, author);
            else book = new Book(title, author, type);
            if (isBorrowed) book->borrow();
            books.push_back(book);
        }
        bookFile.close();
    }

    std::ifstream readerFile("readers.txt");
    if (readerFile.is_open()) {
        std::string line;
        while (std::getline(readerFile, line)) {
            size_t pos1 = line.find(',');
            size_t pos2 = line.find(',', pos1 + 1);
            size_t pos3 = line.find(',', pos2 + 1);
            std::string type = line.substr(0, pos1);
            std::string name = line.substr(pos1 + 1, pos2 - pos1 - 1);
            double fine = std::stod(line.substr(pos3 + 1));
            Reader* reader;
            if (type == "RegularMember") reader = new RegularMember(name);
            else if (type == "VIPMember") reader = new VIPMember(name);
            else if (type == "StudentMember") reader = new StudentMember(name);
            else reader = new RegularMember(name);
            if (fine > 0) reader->addFine(fine);
            readers.push_back(reader);
        }
        readerFile.close();
    }

    std::ifstream recordFile("records.txt");
    if (recordFile.is_open()) {
        std::string line;
        while (std::getline(recordFile, line)) {
            size_t pos1 = line.find(',');
            size_t pos2 = line.find(',', pos1 + 1);
            size_t pos3 = line.find(',', pos2 + 1);
            size_t pos4 = line.find(',', pos3 + 1);
            size_t pos5 = line.find(',', pos4 + 1);
            std::string bookTitle = line.substr(0, pos1);
            std::string readerName = line.substr(pos1 + 1, pos2 - pos1 - 1);
            std::time_t borrowDate = std::stol(line.substr(pos2 + 1, pos3 - pos2 - 1));
            std::time_t dueDate = std::stol(line.substr(pos3 + 1, pos4 - pos3 - 1));
            std::time_t returnDate = std::stol(line.substr(pos4 + 1, pos5 - pos4 - 1));
            bool isReturned = (line.substr(pos5 + 1) == "1");
            Book* book = findBook(bookTitle);
            Reader* reader = findReader(readerName);
            if (book && reader) {
                borrowRecords.emplace_back(book, reader, borrowDate, dueDate);
                if (isReturned) {
                    borrowRecords.back().setReturnDate(returnDate);
                }
            }
        }
        recordFile.close();
    }

    std::ifstream userFile("users.txt");
    if (userFile.is_open()) {
        std::string line;
        while (std::getline(userFile, line)) {
            size_t pos1 = line.find(',');
            size_t pos2 = line.find(',', pos1 + 1);
            size_t pos3 = line.find(',', pos2 + 1);
            std::string userType = line.substr(0, pos1);
            std::string username = line.substr(pos1 + 1, pos2 - pos1 - 1);
            std::string password = line.substr(pos2 + 1, pos3 - pos2 - 1);
            if (userType == "Administrator") {
                users.push_back(std::make_unique<Administrator>(username, password));
            } else if (userType == "ReaderUser") {
                size_t pos4 = line.find(',', pos3 + 1);
                std::string readerName = line.substr(pos3 + 1, pos4 - pos3 - 1);
                Reader* reader = findReader(readerName);
                if (reader) {
                    users.push_back(std::make_unique<ReaderUser>(username, password, reader));
                }
            }
        }
        userFile.close();
    }
}

// 辅助方法
Book* Library::findBook(const std::string& title) {
    for (auto book : books) {
        if (book->getTitle() == title) return book;
    }
    return nullptr;
}

Reader* Library::findReader(const std::string& name) {
    for (auto reader : readers) {
        if (reader->getName() == name) return reader;
    }
    return nullptr;
}

User* Library::findUser(const std::string& username) {
    for (const auto& user : users) {
        if (user->getUsername() == username) {
            return user.get();
        }
    }
    return nullptr;
}

int Library::countBooks() const { return books.size(); }

int Library::countReaders() const { return readers.size(); }

int Library::countBorrowedBooks() const {
    return std::count_if(books.begin(), books.end(), 
        [](Book* b) { return b->isBorrowedStatus(); });
}

// 菜单系统
void Library::bookManagementMenu() {
    while (true) {
        printSectionHeader("图书管理");
        std::cout << std::setw(4) << " " << "\033[1;33m请选择操作：\033[0m\n";
        std::cout << std::setw(4) << " " << " 1. 添加图书\n";
        std::cout << std::setw(4) << " " << " 2. 删除图书\n";
        std::cout << std::setw(4) << " " << " 3. 查找图书\n";
        std::cout << std::setw(4) << " " << " 4. 显示所有图书\n";
        std::cout << std::setw(4) << " " << " 5. 返回主菜单\n";
        int choice;
        std::cout << "请输入选项 (1-5): ";
        if (!(std::cin >> choice)) {
            clearInputBuffer();
            std::cerr << "\033[1;31m[错误] 请输入有效的数字选项！\033[0m\n";
            continue;
        }
        clearInputBuffer();
        try {
            switch (choice) {
                case 1: {
                    printSectionHeader("添加图书");
                    std::string title, author;
                    int typeChoice;
                    std::cout << "书名: ";
                    std::getline(std::cin, title);
                    std::cout << "作者: ";
                    std::getline(std::cin, author);
                    do {
                        std::cout << "类型选择:\n1. 教科书\n2. 小说\n3. 杂志\n4. 普通图书\n请选择(1-4): ";
                        if (!(std::cin >> typeChoice)) {
                            clearInputBuffer();
                            std::cerr << "\033[1;31m[错误] 请输入数字选项！\033[0m\n";
                            continue;
                        }
                        clearInputBuffer();
                        Book* newBook = nullptr;
                        switch (typeChoice) {
                            case 1: newBook = new Textbook(title, author); break;
                            case 2: newBook = new Novel(title, author); break;
                            case 3: newBook = new Magazine(title, author); break;
                            case 4: newBook = new Book(title, author); break;
                            default:
                                std::cerr << "\033[1;31m[错误] 无效的类型选择！\033[0m\n";
                                continue;
                        }
                        addBook(newBook);
                        std::cout << "\033[1;32m[成功] ✔ 图书添加成功！\033[0m\n";
                        break;
                    } while (true);
                    break;
                }
                case 2: {
                    printSectionHeader("删除图书");
                    std::string bookTitle;
                    std::cout << "请输入要删除的书名: ";
                    std::getline(std::cin, bookTitle);
                    removeBook(bookTitle);
                    std::cout << "\033[1;32m[成功] ✔ 图书删除成功！\033[0m\n";
                    break;
                }
                case 3: {
                    printSectionHeader("查找图书");
                    std::string bookTitle;
                    std::cout << "请输入要查找的书名: ";
                    std::getline(std::cin, bookTitle);
                    searchBook(bookTitle);
                    break;
                }
                case 4:
                    printSectionHeader("所有图书");
                    displayBooks();
                    break;
                case 5:
                    return;
                default:
                    std::cerr << "\033[1;31m[错误] 无效的选项，请重新输入！\033[0m\n";
            }
        } catch (const std::exception& ex) {
            std::cerr << "\033[1;31m[错误] " << ex.what() << "\033[0m\n";
        }
        std::cout << "\n\033[1;33m按回车继续...\033[0m";
        std::cin.get();
    }
}

void Library::readerManagementMenu() {
    while (true) {
        printSectionHeader("读者管理");
        std::cout << std::setw(4) << " " << "\033[1;33m请选择操作：\033[0m\n";
        std::cout << std::setw(4) << " " << " 1. 添加读者\n";
        std::cout << std::setw(4) << " " << " 2. 删除读者\n";
        std::cout << std::setw(4) << " " << " 3. 查找读者\n";
        std::cout << std::setw(4) << " " << " 4. 显示所有读者\n";
        std::cout << std::setw(4) << " " << " 5. 返回主菜单\n";
        int choice;
        std::cout << "请输入选项 (1-5): ";
        if (!(std::cin >> choice)) {
            clearInputBuffer();
            std::cerr << "\033[1;31m[错误] 请输入有效的数字选项！\033[0m\n";
            continue;
        }
        clearInputBuffer();
        try {
            switch (choice) {
                case 1: {
                    printSectionHeader("添加读者");
                    std::string name;
                    int typeChoice;
                    std::cout << "姓名: ";
                    std::getline(std::cin, name);
                    do {
                        std::cout << "类型选择:\n1. 普通会员\n2. VIP会员\n3. 学生会员\n请选择(1-3): ";
                        if (!(std::cin >> typeChoice)) {
                            clearInputBuffer();
                            std::cerr << "\033[1;31m[错误] 请输入数字选项！\033[0m\n";
                            continue;
                        }
                        clearInputBuffer();
                        Reader* newReader = nullptr;
                        switch (typeChoice) {
                            case 1: newReader = new RegularMember(name); break;
                            case 2: newReader = new VIPMember(name); break;
                            case 3: newReader = new StudentMember(name); break;
                            default:
                                std::cerr << "\033[1;31m[错误] 无效的类型选择！\033[0m\n";
                                continue;
                        }
                        addReader(newReader);
                        std::cout << "\033[1;32m[成功] ✔ 读者添加成功！\033[0m\n";
                        break;
                    } while (true);
                    break;
                }
                case 2: {
                    printSectionHeader("删除读者");
                    std::string readerName;
                    std::cout << "请输入要删除的读者姓名: ";
                    std::getline(std::cin, readerName);
                    removeReader(readerName);
                    std::cout << "\033[1;32m[成功] ✔ 读者删除成功！\033[0m\n";
                    break;
                }
                case 3: {
                    printSectionHeader("查找读者");
                    std::string readerName;
                    std::cout << "请输入要查找的读者姓名: ";
                    std::getline(std::cin, readerName);
                    searchReader(readerName);
                    break;
                }
                case 4:
                    printSectionHeader("所有读者");
                    displayReaders();
                    break;
                case 5:
                    return;
                default:
                    std::cerr << "\033[1;31m[错误] 无效的选项，请重新输入！\033[0m\n";
            }
        } catch (const std::exception& ex) {
            std::cerr << "\033[1;31m[错误] " << ex.what() << "\033[0m\n";
        }
        std::cout << "\n\033[1;33m按回车继续...\033[0m";
        std::cin.get();
    }
}

void Library::registerUser() {
    std::string username, password;
    std::string readerName;
    int readerTypeChoice;
    int userTypeChoice;
    std::cout << "请选择用户类型:\n1. 读者\n2. 管理员\n请选择(1-2): ";
    if (!(std::cin >> userTypeChoice)) {
        clearInputBuffer();
        std::cerr << "\033[1;31m[错误] 请输入数字选项！\033[0m\n";
        return;
    }
    clearInputBuffer();
    std::cout << "请输入用户名: ";
    std::getline(std::cin, username);
    if (findUser(username) != nullptr) {
        std::cout << "\033[1;31m[错误] 该用户名已被使用，请选择其他用户名！\033[0m\n";
        return;
    }
    std::cout << "请输入密码: ";
    std::getline(std::cin, password);
    if (userTypeChoice == 1) {
        std::cout << "请输入读者姓名: ";
        std::getline(std::cin, readerName);
        do {
            std::cout << "读者类型选择:\n1. 普通会员\n2. VIP会员\n3. 学生会员\n请选择(1-3): ";
            if (!(std::cin >> readerTypeChoice)) {
                clearInputBuffer();
                std::cerr << "\033[1;31m[错误] 请输入数字选项！\033[0m\n";
                continue;
            }
            clearInputBuffer();
            Reader* newReader = nullptr;
            switch (readerTypeChoice) {
                case 1: newReader = new RegularMember(readerName); break;
                case 2: newReader = new VIPMember(readerName); break;
                case 3: newReader = new StudentMember(readerName); break;
                default:
                    std::cerr << "\033[1;31m[错误] 无效的读者类型选择！\033[0m\n";
                    continue;
            }
            addReader(newReader);
            users.push_back(std::make_unique<ReaderUser>(username, password, newReader));
            std::cout << "\033[1;32m[成功] ✔ 读者用户注册成功！\033[0m\n";
            break;
        } while (true);
    } else if (userTypeChoice == 2) {
        users.push_back(std::make_unique<Administrator>(username, password));
        std::cout << "\033[1;32m[成功] ✔ 管理员用户注册成功！\033[0m\n";
    }
}

void Library::adminViewAllUsers() {
    if (!currentUser || !currentUser->isAdmin()) {
        std::cout << "\033[1;31m[错误] 只有管理员可以查看所有用户信息！\033[0m\n";
        return;
    }
    std::cout << "👥 所有用户信息：\n";
    for (const auto& user : users) {
        std::cout << "用户名: " << user->getUsername();
        if (dynamic_cast<Administrator*>(user.get())) {
            std::cout << ", 用户类型: 管理员\n";
        } else if (auto readerUser = dynamic_cast<ReaderUser*>(user.get())) {
            std::cout << ", 用户类型: 读者, 读者姓名: " << readerUser->getReader()->getName() << "\n";
        }
    }
}

void Library::adminDeleteUser() {
    if (!currentUser || !currentUser->isAdmin()) {
        std::cout << "\033[1;31m[错误] 只有管理员可以删除用户！\033[0m\n";
        return;
    }
    std::string username;
    std::cout << "请输入要删除的用户名: ";
    std::getline(std::cin, username);
    auto it = std::remove_if(users.begin(), users.end(), 
        [&](const std::unique_ptr<User>& user) { return user->getUsername() == username; });
    if (it == users.end()) {
        std::cout << "\033[1;31m[错误] 未找到该用户！\033[0m\n";
    } else {
        users.erase(it, users.end());
        std::cout << "\033[1;32m[成功] ✔ 用户删除成功！\033[0m\n";
    }
}

bool Library::login() {
    std::string username, password;
    std::cout << "请输入用户名: ";
    std::getline(std::cin, username);
    std::cout << "请输入密码: ";
    std::getline(std::cin, password);
    currentUser = findUser(username);
    if (currentUser && currentUser->verifyPassword(password)) {
        std::cout << "\033[1;32m[成功] ✔ 登录成功！\033[0m\n";
        return true;
    } else {
        return false;
    }
}

void Library::mainMenu() {
    while (true) {
        printSectionHeader("图书馆管理系统");
        if (!currentUser) {
            std::cout << std::setw(4) << " " << "\033[1;33m请选择操作：\033[0m\n";
            std::cout << std::setw(4) << " " << " 1. 登录\n";
            std::cout << std::setw(4) << " " << " 2. 注册\n";
            std::cout << std::setw(4) << " " << " 3. 退出\n";
            int choice;
            std::cout << "请输入选项 (1-3): ";
            if (!(std::cin >> choice)) {
                clearInputBuffer();
                std::cerr << "\033[1;31m[错误] 请输入有效的数字选项！\033[0m\n";
                continue;
            }
            clearInputBuffer();
            switch (choice) {
                case 1:
                    if (login()) continue;
                    break;
                case 2:
                    registerUser();
                    break;
                case 3:
                    return;
                default:
                    std::cerr << "\033[1;31m[错误] 无效的选项，请重新输入！\033[0m\n";
            }
        } else {
            std::cout << std::setw(4) << " " << "\033[1;33m欢迎，" << currentUser->getUsername() << "！请选择操作：\033[0m\n";
            if (currentUser->isAdmin()) {
                std::cout << std::setw(4) << " " << " 1. 图书管理\n";
                std::cout << std::setw(4) << " " << " 2. 读者管理\n";
                std::cout << std::setw(4) << " " << " 3. 查看所有借阅记录\n";
                std::cout << std::setw(4) << " " << " 4. 查看超期未还图书\n";
                std::cout << std::setw(4) << " " << " 5. 查看即将到期图书\n";
                std::cout << std::setw(4) << " " << " 6. 查看所有用户信息\n";
                std::cout << std::setw(4) << " " << " 7. 删除用户\n";
                std::cout << std::setw(4) << " " << " 8. 注销登录\n";
            } else {
                auto readerUser = dynamic_cast<ReaderUser*>(currentUser);
                if (readerUser) {
                    std::cout << std::setw(4) << " " << " 1. 借阅图书\n";
                    std::cout << std::setw(4) << " " << " 2. 归还图书\n";
                    std::cout << std::setw(4) << " " << " 3. 支付罚款\n";
                    std::cout << std::setw(4) << " " << " 4. 查看个人借阅记录\n";
                    std::cout << std::setw(4) << " " << " 5. 注销登录\n";
                }
            }
            int choice;
            std::cout << "请输入选项: ";
            if (!(std::cin >> choice)) {
                clearInputBuffer();
                std::cerr << "\033[1;31m[错误] 请输入有效的数字选项！\033[0m\n";
                continue;
            }
            clearInputBuffer();
            try {
                if (currentUser->isAdmin()) {
                    switch (choice) {
                        case 1:
                            bookManagementMenu();
                            break;
                        case 2:
                            readerManagementMenu();
                            break;
                        case 3:
                            displayBorrowRecords();
                            break;
                        case 4:
                            displayOverdueBooks();
                            break;
                        case 5:
                            displayBooksDueSoon();
                            break;
                        case 6:
                            adminViewAllUsers();
                            break;
                        case 7:
                            adminDeleteUser();
                            break;
                        case 8:
                            currentUser = nullptr;
                            break;
                        default:
                            std::cerr << "\033[1;31m[错误] 无效的选项，请重新输入！\033[0m\n";
                    }
                } else {
                    auto readerUser = dynamic_cast<ReaderUser*>(currentUser);
                    if (readerUser) {
                        switch (choice) {
                            case 1: {
                                std::string bookTitle;
                                std::cout << "请输入要借阅的书名: ";
                                std::getline(std::cin, bookTitle);
                                borrowBook(bookTitle, readerUser->getReader()->getName());
                                break;
                            }
                            case 2: {
                                std::string bookTitle;
                                std::cout << "请输入要归还的书名: ";
                                std::getline(std::cin, bookTitle);
                                returnBook(bookTitle, readerUser->getReader()->getName());
                                break;
                            }
                            case 3: {
                                double amount;
                                std::cout << "请输入要支付的罚款金额（输入 -1 全额支付）: ";
                                std::cin >> amount;
                                clearInputBuffer();
                                payFine(readerUser->getReader()->getName(), amount);
                                break;
                            }
                            case 4:
                                searchReader(readerUser->getReader()->getName());
                                break;
                            case 5:
                                currentUser = nullptr;
                                break;
                            default:
                                std::cerr << "\033[1;31m[错误] 无效的选项，请重新输入！\033[0m\n";
                        }
                    }
                }
            } catch (const std::exception& ex) {
                std::cerr << "\033[1;31m[错误] " << ex.what() << "\033[0m\n";
            }
            std::cout << "\n\033[1;33m按回车继续...\033[0m";
            std::cin.get();
        }
    }
}