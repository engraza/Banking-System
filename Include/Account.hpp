#pragma once

#include "Transaction.hpp"
#include <string>
#include <vector>
#include <iostream>

enum class AccountType {
    SAVINGS,
    CHECKING
};

class Account {
private:
    std::string accountNumber;
    std::string customerId;
    AccountType accountType;
    double balance;
    std::string pin; // 4-digit security PIN
    std::vector<Transaction> transactionHistory;

public:
    Account() = default;
    Account(std::string accNum,
            std::string custId,
            AccountType type,
            double initialDeposit = 0.0,
            std::string securityPin = "1234");

    // Getters
    [[nodiscard]] const std::string& getAccountNumber() const;
    [[nodiscard]] const std::string& getCustomerId() const;
    [[nodiscard]] AccountType getAccountType() const;
    [[nodiscard]] std::string getAccountTypeString() const;
    [[nodiscard]] double getBalance() const;
    [[nodiscard]] const std::vector<Transaction>& getTransactionHistory() const;

    // PIN Security
    [[nodiscard]] bool verifyPin(const std::string& inputPin) const;
    bool changePin(const std::string& oldPin, const std::string& newPin);
    static bool isValidPin(const std::string& testPin);

    // Financial operations
    bool deposit(double amount,
                 const std::string& remarks = "Cash Deposit",
                 const std::string& txnId = "",
                 const std::string& timestamp = "");

    bool withdraw(double amount,
                  const std::string& remarks = "Cash Withdrawal",
                  const std::string& txnId = "",
                  const std::string& timestamp = "");

    bool debit(double amount,
               const std::string& remarks,
               const std::string& txnId,
               const std::string& timestamp,
               TransactionType type,
               const std::string& relatedAccount = "");

    bool credit(double amount,
                const std::string& remarks,
                const std::string& txnId,
                const std::string& timestamp,
                TransactionType type,
                const std::string& relatedAccount = "");

    void recordTransaction(const Transaction& transaction);

    // Query & History
    [[nodiscard]] std::vector<Transaction> getRecentTransactions(size_t count) const;
    void displayAccountDetails(bool useColor = true) const;
    void displayTransactionHistory(size_t count = 0, bool useColor = true) const;
};
