#pragma once
#include <ctime>
#include <iostream>
#include "Book.h"
#include "Reader.h"
#include "DateUtils.h"

class BorrowRecord {
public:
    BorrowRecord(Book* book, Reader* reader, std::time_t borrowDate, std::time_t dueDate);
    
    Book* getBook() const { return book; }
    Reader* getReader() const { return reader; }
    std::time_t getBorrowDate() const { return borrowDate; }
    std::time_t getDueDate() const { return dueDate; }
    std::time_t getReturnDate() const { return returnDate; }
    bool getIsReturned() const { return isReturned; }
    
    void setReturnDate(std::time_t returnDate);
    int getOverdueDays() const;
    double calculateFine() const;
    void display() const;

private:
    Book* book;
    Reader* reader;
    std::time_t borrowDate;
    std::time_t dueDate;
    std::time_t returnDate;
    bool isReturned;
};