#pragma once

#include "Customer.hpp"
#include "Account.hpp"
#include "Transaction.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <optional>
#include <tuple>

class Bank {
private:
    std::string bankName;
    std::unordered_map<std::string, Customer> customers;
    std::unordered_map<std::string, Account> accounts;

public:
    explicit Bank(std::string name = "RAY Bank");

    [[nodiscard]] const std::string& getBankName() const;

    // Customer operations
    std::string createCustomer(const std::string& name,
                               const std::string& email,
                               const std::string& phone,
                               const std::string& address,
                               std::string customId = "");

    Customer* getCustomer(const std::string& customerId);
    [[nodiscard]] const Customer* getCustomer(const std::string& customerId) const;
    [[nodiscard]] bool customerExists(const std::string& customerId) const;
    [[nodiscard]] const std::unordered_map<std::string, Customer>& getAllCustomers() const;

    // Account operations
    std::string createAccount(const std::string& customerId,
                              AccountType type,
                              double initialDeposit = 0.0,
                              std::string customAccNum = "",
                              std::string securityPin = "1234");

    Account* getAccount(const std::string& accountNumber);
    [[nodiscard]] const Account* getAccount(const std::string& accountNumber) const;
    [[nodiscard]] bool accountExists(const std::string& accountNumber) const;
    [[nodiscard]] const std::unordered_map<std::string, Account>& getAllAccounts() const;
    [[nodiscard]] std::vector<Account> getAccountsForCustomer(const std::string& customerId) const;

    // PIN management
    bool changeAccountPin(const std::string& accountNumber,
                          const std::string& oldPin,
                          const std::string& newPin);

    // Financial operations
    bool deposit(const std::string& accountNumber,
                 double amount,
                 const std::string& remarks = "Cash Deposit");

    bool withdraw(const std::string& accountNumber,
                  double amount,
                  const std::string& pin,
                  const std::string& remarks = "Cash Withdrawal");

    bool transferFunds(const std::string& fromAccNum,
                       const std::string& toAccNum,
                       double amount,
                       const std::string& pin,
                       const std::string& remarks = "Fund Transfer");

    // Receipt Operations
    [[nodiscard]] std::optional<std::tuple<Transaction, std::string, std::string>> findTransaction(const std::string& txnId) const;
    [[nodiscard]] std::string getFormattedReceipt(const std::string& txnId, bool useColor = true) const;
    bool exportReceiptToFile(const std::string& txnId, const std::string& directoryPath = "receipts") const;

    // Display & Reports (Rich Colors)
    void displayCustomerSummary(const std::string& customerId, bool useColor = true) const;
    void displayAccountSummary(const std::string& accountNumber, bool useColor = true) const;
    void displayAccountTransactions(const std::string& accountNumber, size_t count = 0, bool useColor = true) const;
    void displayAllCustomers(bool useColor = true) const;
    void displayAllAccounts(bool useColor = true) const;

    // Sample data initialization
    void seedSampleData();
};
