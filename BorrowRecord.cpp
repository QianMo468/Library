#include "BorrowRecord.h"

BorrowRecord::BorrowRecord(Book* book, Reader* reader, std::time_t borrowDate, std::time_t dueDate)
    : book(book), reader(reader), borrowDate(borrowDate), dueDate(dueDate), returnDate(0), isReturned(false) {}

void BorrowRecord::setReturnDate(std::time_t returnDate) {
    this->returnDate = returnDate;
    isReturned = true;
}

int BorrowRecord::getOverdueDays() const {
    if (!isReturned) {
        std::time_t now = DateUtils::getCurrentTime();
        return now > dueDate ? (now - dueDate) / (24 * 60 * 60) : 0;
    }
    return returnDate > dueDate ? (returnDate - dueDate) / (24 * 60 * 60) : 0;
}

double BorrowRecord::calculateFine() const {
    int overdueDays = getOverdueDays();
    if (overdueDays <= 0) return 0.0;
    return overdueDays * book->getFinePerDay() * reader->getFineDiscount();
}

void BorrowRecord::display() const {
    std::cout << "📖 书名: " << book->getTitle() << "\n";
    std::cout << "👤 读者: " << reader->getName() << " (" << reader->getTypeName() << ")\n";
    std::cout << "📅 借阅日期: " << DateUtils::formatTime(borrowDate) << "\n";
    std::cout << "📅 应还日期: " << DateUtils::formatTime(dueDate) << "\n";
    if (isReturned) {
        std::cout << "📅 归还日期: " << DateUtils::formatTime(returnDate) << "\n";
        int overdueDays = getOverdueDays();
        if (overdueDays > 0) {
            std::cout << "⏰ 超期天数: " << overdueDays << "天\n";
            std::cout << "💰 逾期罚款: " << calculateFine() << "元\n";
        }
    } else {
        int overdueDays = getOverdueDays();
        if (overdueDays > 0) {
            std::cout << "⚠️ 已超期: " << overdueDays << "天\n";
            std::cout << "💰 逾期罚款: " << calculateFine() << "元\n";
        } else {
            int daysLeft = (dueDate - DateUtils::getCurrentTime()) / (24 * 60 * 60);
            std::cout << "⌛ 剩余天数: " << daysLeft << "天\n";
        }
    }
    std::cout << "------------------------\n";
}