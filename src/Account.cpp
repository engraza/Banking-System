#include "Account.hpp"
#include "Utils.hpp"
#include <iomanip>
#include <algorithm>
#include <cctype>

Account::Account(std::string accNum,
                 std::string custId,
                 AccountType type,
                 double initialDeposit,
                 std::string securityPin)
    : accountNumber(std::move(accNum)),
      customerId(std::move(custId)),
      accountType(type),
      balance(0.0),
      pin(isValidPin(securityPin) ? securityPin : "1234") {
    if (initialDeposit > 0.0) {
        credit(initialDeposit, "Initial Account Opening Deposit", "", "", TransactionType::DEPOSIT, "");
    }
}

const std::string& Account::getAccountNumber() const {
    return accountNumber;
}

const std::string& Account::getCustomerId() const {
    return customerId;
}

AccountType Account::getAccountType() const {
    return accountType;
}

std::string Account::getAccountTypeString() const {
    switch (accountType) {
        case AccountType::SAVINGS:  return "SAVINGS";
        case AccountType::CHECKING: return "CHECKING";
        default:                    return "UNKNOWN";
    }
}

double Account::getBalance() const {
    return balance;
}

const std::vector<Transaction>& Account::getTransactionHistory() const {
    return transactionHistory;
}

bool Account::verifyPin(const std::string& inputPin) const {
    return inputPin == pin;
}

bool Account::changePin(const std::string& oldPin, const std::string& newPin) {
    if (!verifyPin(oldPin)) {
        return false;
    }
    if (!isValidPin(newPin)) {
        return false;
    }
    pin = newPin;
    return true;
}

bool Account::isValidPin(const std::string& testPin) {
    if (testPin.length() != 4) {
        return false;
    }
    for (char ch : testPin) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    return true;
}

bool Account::credit(double amount,
                     const std::string& remarks,
                     const std::string& txnId,
                     const std::string& timestamp,
                     TransactionType type,
                     const std::string& relatedAccount) {
    if (amount <= 0.0) {
        return false;
    }
    balance += amount;
    std::string id = txnId.empty() ? Utils::generateTransactionId() : txnId;
    std::string ts = timestamp.empty() ? Utils::getCurrentTimestamp() : timestamp;
    transactionHistory.emplace_back(id, ts, type, amount, balance, relatedAccount, remarks);
    return true;
}

bool Account::debit(double amount,
                    const std::string& remarks,
                    const std::string& txnId,
                    const std::string& timestamp,
                    TransactionType type,
                    const std::string& relatedAccount) {
    if (amount <= 0.0 || amount > balance) {
        return false;
    }
    balance -= amount;
    std::string id = txnId.empty() ? Utils::generateTransactionId() : txnId;
    std::string ts = timestamp.empty() ? Utils::getCurrentTimestamp() : timestamp;
    transactionHistory.emplace_back(id, ts, type, amount, balance, relatedAccount, remarks);
    return true;
}

bool Account::deposit(double amount, const std::string& remarks, const std::string& txnId, const std::string& timestamp) {
    return credit(amount, remarks, txnId, timestamp, TransactionType::DEPOSIT, "");
}

bool Account::withdraw(double amount, const std::string& remarks, const std::string& txnId, const std::string& timestamp) {
    return debit(amount, remarks, txnId, timestamp, TransactionType::WITHDRAWAL, "");
}

void Account::recordTransaction(const Transaction& transaction) {
    transactionHistory.push_back(transaction);
}

std::vector<Transaction> Account::getRecentTransactions(size_t count) const {
    if (count == 0 || count >= transactionHistory.size()) {
        return transactionHistory;
    }
    return std::vector<Transaction>(transactionHistory.end() - count, transactionHistory.end());
}

void Account::displayAccountDetails(bool useColor) const {
    std::string cBrd = useColor ? Utils::Color::CYAN : "";
    std::string cTtl = useColor ? Utils::Color::GOLD + Utils::Color::BOLD : "";
    std::string cLbl = useColor ? Utils::Color::WHITE : "";
    std::string cVal = useColor ? Utils::Color::CYAN + Utils::Color::BOLD : "";
    std::string cGrn = useColor ? Utils::Color::GREEN + Utils::Color::BOLD : "";
    std::string cPur = useColor ? Utils::Color::PURPLE : "";
    std::string cRst = useColor ? Utils::Color::RESET : "";

    std::cout << "\n" << cBrd << "+-------------------------------------------------------------+\n" << cRst;
    std::cout << cBrd << "| " << cTtl << "                      ACCOUNT SUMMARY                        " << cBrd << "|\n" << cRst;
    std::cout << cBrd << "+-------------------------------------------------------------+\n" << cRst;
    std::cout << "  " << cLbl << "Account Number  : " << cVal << accountNumber << cRst << "\n";
    std::cout << "  " << cLbl << "Owner ID        : " << cPur << customerId << cRst << "\n";
    std::cout << "  " << cLbl << "Account Type    : " << cVal << getAccountTypeString() << cRst << "\n";
    std::cout << "  " << cLbl << "Current Balance : " << cGrn << Utils::formatCurrency(balance) << cRst << "\n";
    std::cout << "  " << cLbl << "Security Status : " << cGrn << "[ PIN PROTECTED **** ]" << cRst << "\n";
    std::cout << "  " << cLbl << "Total Txns Log  : " << cVal << transactionHistory.size() << cRst << "\n";
    std::cout << cBrd << "+-------------------------------------------------------------+\n" << cRst;
}

void Account::displayTransactionHistory(size_t count, bool useColor) const {
    std::vector<Transaction> txns = getRecentTransactions(count);
    std::string cBrd = useColor ? Utils::Color::CYAN : "";
    std::string cTtl = useColor ? Utils::Color::GOLD + Utils::Color::BOLD : "";
    std::string cGrn = useColor ? Utils::Color::GREEN + Utils::Color::BOLD : "";
    std::string cRst = useColor ? Utils::Color::RESET : "";
    
    std::cout << "\n" << cBrd << "=======================================================================================================\n" << cRst;
    std::cout << cTtl << "                           TRANSACTION STATEMENT FOR " << accountNumber << cRst << "\n";
    if (count > 0 && count < transactionHistory.size()) {
        std::cout << Utils::Color::PURPLE << "                              (Showing Last " << count << " of " << transactionHistory.size() << " Transactions)\n" << cRst;
    } else {
        std::cout << Utils::Color::CYAN << "                                  (Total: " << transactionHistory.size() << " Transactions)\n" << cRst;
    }
    std::cout << cBrd << "=======================================================================================================\n" << cRst;
    
    if (txns.empty()) {
        std::cout << "  No transactions recorded for this account yet.\n";
        std::cout << cBrd << "=======================================================================================================\n" << cRst;
        return;
    }

    std::cout << cBrd << "+-------------+---------------------+---------------+-------------+-------------+-------------+----------------------+\n" << cRst;
    std::cout << cBrd << "| " << Utils::Color::BOLD << "TXN ID      | Date & Time         | Type          |      Amount |     Balance | Related Acc | Remarks              " << cBrd << "|\n" << cRst;
    std::cout << cBrd << "+-------------+---------------------+---------------+-------------+-------------+-------------+----------------------+\n" << cRst;
    
    for (const auto& txn : txns) {
        std::cout << txn.toFormattedString(useColor) << "\n";
    }
    
    std::cout << cBrd << "+-------------+---------------------+---------------+-------------+-------------+-------------+----------------------+\n" << cRst;
    std::cout << "  " << Utils::Color::WHITE << "Current Account Balance: " << cGrn << Utils::formatCurrency(balance) << cRst << "\n";
    std::cout << cBrd << "=======================================================================================================\n" << cRst;
}
