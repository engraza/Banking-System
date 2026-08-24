#pragma once

#include <string>
#include <iostream>

enum class TransactionType {
    DEPOSIT,
    WITHDRAWAL,
    TRANSFER_OUT,
    TRANSFER_IN
};

class Transaction {
private:
    std::string transactionId;
    std::string timestamp;
    TransactionType type;
    double amount;
    double balanceAfter;
    std::string relatedAccount; // Source or Destination account for transfers
    std::string remarks;

public:
    Transaction(std::string txnId,
                std::string ts,
                TransactionType txnType,
                double amt,
                double balAfter,
                std::string relAcc = "",
                std::string rem = "");

    // Getters
    [[nodiscard]] const std::string& getTransactionId() const;
    [[nodiscard]] const std::string& getTimestamp() const;
    [[nodiscard]] TransactionType getType() const;
    [[nodiscard]] std::string getTypeString() const;
    [[nodiscard]] double getAmount() const;
    [[nodiscard]] double getBalanceAfter() const;
    [[nodiscard]] const std::string& getRelatedAccount() const;
    [[nodiscard]] const std::string& getRemarks() const;

    // Display & Receipt Formatting
    void display() const;
    [[nodiscard]] std::string toFormattedString(bool useColor = false) const;
    [[nodiscard]] std::string generateReceipt(const std::string& bankName,
                                              const std::string& customerName,
                                              const std::string& accountNumber,
                                              bool useColor = false) const;
};
