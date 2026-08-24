#include "Customer.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <iomanip>

Customer::Customer(std::string id,
                   std::string name,
                   std::string mail,
                   std::string ph,
                   std::string addr)
    : customerId(std::move(id)),
      fullName(std::move(name)),
      email(std::move(mail)),
      phone(std::move(ph)),
      address(std::move(addr)) {}

const std::string& Customer::getCustomerId() const {
    return customerId;
}

const std::string& Customer::getFullName() const {
    return fullName;
}

const std::string& Customer::getEmail() const {
    return email;
}

const std::string& Customer::getPhone() const {
    return phone;
}

const std::string& Customer::getAddress() const {
    return address;
}

const std::vector<std::string>& Customer::getAccountNumbers() const {
    return accountNumbers;
}

void Customer::setFullName(const std::string& name) {
    fullName = name;
}

void Customer::setEmail(const std::string& mail) {
    email = mail;
}

void Customer::setPhone(const std::string& ph) {
    phone = ph;
}

void Customer::setAddress(const std::string& addr) {
    address = addr;
}

void Customer::addAccount(const std::string& accountNumber) {
    if (!hasAccount(accountNumber)) {
        accountNumbers.push_back(accountNumber);
    }
}

bool Customer::removeAccount(const std::string& accountNumber) {
    auto it = std::find(accountNumbers.begin(), accountNumbers.end(), accountNumber);
    if (it != accountNumbers.end()) {
        accountNumbers.erase(it);
        return true;
    }
    return false;
}

bool Customer::hasAccount(const std::string& accountNumber) const {
    return std::find(accountNumbers.begin(), accountNumbers.end(), accountNumber) != accountNumbers.end();
}

void Customer::displayProfile(bool useColor) const {
    std::string cBrd = useColor ? Utils::Color::CYAN : "";
    std::string cTtl = useColor ? Utils::Color::GOLD + Utils::Color::BOLD : "";
    std::string cLbl = useColor ? Utils::Color::WHITE : "";
    std::string cVal = useColor ? Utils::Color::CYAN + Utils::Color::BOLD : "";
    std::string cPur = useColor ? Utils::Color::PURPLE + Utils::Color::BOLD : "";
    std::string cRst = useColor ? Utils::Color::RESET : "";

    std::cout << "\n" << cBrd << "+-------------------------------------------------------------+\n" << cRst;
    std::cout << cBrd << "| " << cTtl << "                      CUSTOMER PROFILE                       " << cBrd << "|\n" << cRst;
    std::cout << cBrd << "+-------------------------------------------------------------+\n" << cRst;
    std::cout << "  " << cLbl << "Customer ID  : " << cPur << customerId << cRst << "\n";
    std::cout << "  " << cLbl << "Full Name    : " << cVal << fullName << cRst << "\n";
    std::cout << "  " << cLbl << "Email        : " << cLbl << email << cRst << "\n";
    std::cout << "  " << cLbl << "Phone        : " << cLbl << phone << cRst << "\n";
    std::cout << "  " << cLbl << "Address      : " << cLbl << address << cRst << "\n";
    std::cout << "  " << cLbl << "Accounts (" << accountNumbers.size() << ") : ";
    if (accountNumbers.empty()) {
        std::cout << Utils::Color::GRAY << "None" << cRst;
    } else {
        for (size_t i = 0; i < accountNumbers.size(); ++i) {
            std::cout << Utils::Color::GREEN << accountNumbers[i] << cRst << (i + 1 < accountNumbers.size() ? ", " : "");
        }
    }
    std::cout << "\n" << cBrd << "+-------------------------------------------------------------+\n" << cRst;
}
