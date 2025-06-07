#include "Book.h"

Book::Book(const std::string& title, const std::string& author, const std::string& type)
    : title(title), author(author), type(type), isBorrowed(false) {}

Textbook::Textbook(const std::string& title, const std::string& author)
    : Book(title, author, "教科书") {}

Novel::Novel(const std::string& title, const std::string& author)
    : Book(title, author, "小说") {}

Magazine::Magazine(const std::string& title, const std::string& author)
    : Book(title, author, "杂志") {}