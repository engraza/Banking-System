#include "Bank.hpp"
#include "Utils.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <filesystem>

Bank::Bank(std::string name) : bankName(std::move(name)) {}

const std::string& Bank::getBankName() const {
    return bankName;
}

std::string Bank::createCustomer(const std::string& name,
                                 const std::string& email,
                                 const std::string& phone,
                                 const std::string& address,
                                 std::string customId) {
    if (name.empty()) {
        return "";
    }
    std::string id = customId.empty() ? Utils::generateCustomerId() : customId;
    if (customers.find(id) != customers.end()) {
        return "";
    }
    customers.emplace(id, Customer(id, name, email, phone, address));
    return id;
}

Customer* Bank::getCustomer(const std::string& customerId) {
    auto it = customers.find(customerId);
    if (it != customers.end()) {
        return &(it->second);
    }
    return nullptr;
}

const Customer* Bank::getCustomer(const std::string& customerId) const {
    auto it = customers.find(customerId);
    if (it != customers.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool Bank::customerExists(const std::string& customerId) const {
    return customers.find(customerId) != customers.end();
}

const std::unordered_map<std::string, Customer>& Bank::getAllCustomers() const {
    return customers;
}

std::string Bank::createAccount(const std::string& customerId,
                               AccountType type,
                               double initialDeposit,
                               std::string customAccNum,
                               std::string securityPin) {
    Customer* cust = getCustomer(customerId);
    if (!cust) {
        return "";
    }
    if (initialDeposit < 0.0) {
        return "";
    }

    std::string accNum = customAccNum.empty() ? Utils::generateAccountNumber() : customAccNum;
    if (accounts.find(accNum) != accounts.end()) {
        return "";
    }

    Account acc(accNum, customerId, type, initialDeposit, securityPin);
    accounts.emplace(accNum, std::move(acc));
    cust->addAccount(accNum);
    return accNum;
}

Account* Bank::getAccount(const std::string& accountNumber) {
    auto it = accounts.find(accountNumber);
    if (it != accounts.end()) {
        return &(it->second);
    }
    return nullptr;
}

const Account* Bank::getAccount(const std::string& accountNumber) const {
    auto it = accounts.find(accountNumber);
    if (it != accounts.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool Bank::accountExists(const std::string& accountNumber) const {
    return accounts.find(accountNumber) != accounts.end();
}

const std::unordered_map<std::string, Account>& Bank::getAllAccounts() const {
    return accounts;
}

std::vector<Account> Bank::getAccountsForCustomer(const std::string& customerId) const {
    std::vector<Account> result;
    const Customer* cust = getCustomer(customerId);
    if (!cust) return result;

    for (const auto& accNum : cust->getAccountNumbers()) {
        const Account* acc = getAccount(accNum);
        if (acc) {
            result.push_back(*acc);
        }
    }
    return result;
}

bool Bank::changeAccountPin(const std::string& accountNumber,
                            const std::string& oldPin,
                            const std::string& newPin) {
    Account* acc = getAccount(accountNumber);
    if (!acc) {
        return false;
    }
    return acc->changePin(oldPin, newPin);
}

bool Bank::deposit(const std::string& accountNumber, double amount, const std::string& remarks) {
    Account* acc = getAccount(accountNumber);
    if (!acc || amount <= 0.0) {
        return false;
    }
    return acc->deposit(amount, remarks);
}

bool Bank::withdraw(const std::string& accountNumber,
                    double amount,
                    const std::string& pin,
                    const std::string& remarks) {
    Account* acc = getAccount(accountNumber);
    if (!acc || amount <= 0.0) {
        return false;
    }
    if (!acc->verifyPin(pin)) {
        return false; // Invalid PIN
    }
    if (acc->getBalance() < amount) {
        return false; // Insufficient balance
    }
    return acc->withdraw(amount, remarks);
}

bool Bank::transferFunds(const std::string& fromAccNum,
                        const std::string& toAccNum,
                        double amount,
                        const std::string& pin,
                        const std::string& remarks) {
    if (fromAccNum == toAccNum) {
        return false;
    }
    if (amount <= 0.0) {
        return false;
    }

    Account* fromAcc = getAccount(fromAccNum);
    Account* toAcc = getAccount(toAccNum);

    if (!fromAcc || !toAcc) {
        return false;
    }

    if (!fromAcc->verifyPin(pin)) {
        return false; // PIN verification failed
    }

    if (fromAcc->getBalance() < amount) {
        return false; // Insufficient funds
    }

    std::string timestamp = Utils::getCurrentTimestamp();
    std::string outTxnId = Utils::generateTransactionId();
    std::string inTxnId = Utils::generateTransactionId();
    std::string note = remarks.empty() ? "Fund Transfer" : remarks;

    bool debitOk = fromAcc->debit(amount, note, outTxnId, timestamp, TransactionType::TRANSFER_OUT, toAccNum);
    if (!debitOk) {
        return false;
    }

    bool creditOk = toAcc->credit(amount, note, inTxnId, timestamp, TransactionType::TRANSFER_IN, fromAccNum);
    if (!creditOk) {
        // Rollback
        fromAcc->credit(amount, "Transfer Rollback", Utils::generateTransactionId(), timestamp, TransactionType::DEPOSIT, "");
        return false;
    }

    return true;
}

std::optional<std::tuple<Transaction, std::string, std::string>> Bank::findTransaction(const std::string& txnId) const {
    for (const auto& [accNum, acc] : accounts) {
        for (const auto& txn : acc.getTransactionHistory()) {
            if (txn.getTransactionId() == txnId) {
                std::string custName = "Customer";
                const Customer* cust = getCustomer(acc.getCustomerId());
                if (cust) {
                    custName = cust->getFullName();
                }
                return std::make_tuple(txn, custName, accNum);
            }
        }
    }
    return std::nullopt;
}

std::string Bank::getFormattedReceipt(const std::string& txnId, bool useColor) const {
    auto match = findTransaction(txnId);
    if (!match) {
        return "";
    }
    const auto& [txn, custName, accNum] = *match;
    return txn.generateReceipt(bankName, custName, accNum, useColor);
}

bool Bank::exportReceiptToFile(const std::string& txnId, const std::string& directoryPath) const {
    auto match = findTransaction(txnId);
    if (!match) {
        return false;
    }
    const auto& [txn, custName, accNum] = *match;
    std::string plainReceipt = txn.generateReceipt(bankName, custName, accNum, false);

    try {
        std::filesystem::create_directories(directoryPath);
        std::string filename = directoryPath + "/receipt_" + txnId + ".txt";
        std::ofstream outFile(filename);
        if (!outFile.is_open()) {
            return false;
        }
        outFile << plainReceipt;
        outFile.close();
        return true;
    } catch (...) {
        return false;
    }
}

void Bank::displayCustomerSummary(const std::string& customerId, bool useColor) const {
    const Customer* cust = getCustomer(customerId);
    if (!cust) {
        Utils::printError("Customer ID '" + customerId + "' not found.");
        return;
    }

    cust->displayProfile(useColor);

    std::vector<Account> customerAccounts = getAccountsForCustomer(customerId);
    std::string cBrd = useColor ? Utils::Color::CYAN : "";
    std::string cGrn = useColor ? Utils::Color::GREEN + Utils::Color::BOLD : "";
    std::string cGld = useColor ? Utils::Color::GOLD + Utils::Color::BOLD : "";
    std::string cRst = useColor ? Utils::Color::RESET : "";

    if (customerAccounts.empty()) {
        std::cout << "  No active accounts found for this customer.\n";
    } else {
        std::cout << "\n  " << Utils::Color::PURPLE << Utils::Color::BOLD << "Associated Accounts (" << customerAccounts.size() << "):" << cRst << "\n";
        std::cout << cBrd << "  +----------------+-------------+-------------------+\n" << cRst;
        std::cout << cBrd << "  | " << Utils::Color::WHITE << "Account Number | Type        | Current Balance   " << cBrd << "|\n" << cRst;
        std::cout << cBrd << "  +----------------+-------------+-------------------+\n" << cRst;
        double totalBalance = 0.0;
        for (const auto& acc : customerAccounts) {
            std::cout << cBrd << "  | " << cRst << std::left << std::setw(15) << acc.getAccountNumber()
                      << cBrd << "| " << cRst << std::setw(12) << acc.getAccountTypeString()
                      << cBrd << "| " << cGrn << std::right << std::setw(17) << Utils::formatCurrency(acc.getBalance()) << cBrd << " |\n" << cRst;
            totalBalance += acc.getBalance();
        }
        std::cout << cBrd << "  +----------------+-------------+-------------------+\n" << cRst;
        std::cout << cBrd << "  | " << cGld << "Total Portfolio Balance:     " << cBrd << "|" << cGrn << std::right << std::setw(18) << Utils::formatCurrency(totalBalance) << cBrd << " |\n" << cRst;
        std::cout << cBrd << "  +------------------------------+-------------------+\n" << cRst;
    }
}

void Bank::displayAccountSummary(const std::string& accountNumber, bool useColor) const {
    const Account* acc = getAccount(accountNumber);
    if (!acc) {
        Utils::printError("Account '" + accountNumber + "' not found.");
        return;
    }
    acc->displayAccountDetails(useColor);
}

void Bank::displayAccountTransactions(const std::string& accountNumber, size_t count, bool useColor) const {
    const Account* acc = getAccount(accountNumber);
    if (!acc) {
        Utils::printError("Account '" + accountNumber + "' not found.");
        return;
    }
    acc->displayTransactionHistory(count, useColor);
}

void Bank::displayAllCustomers(bool useColor) const {
    Utils::printHeader("REGISTERED CUSTOMERS (" + std::to_string(customers.size()) + ")", Utils::Color::PURPLE);
    if (customers.empty()) {
        std::cout << "  No customers registered yet.\n";
        return;
    }

    std::string cBrd = useColor ? Utils::Color::PURPLE : "";
    std::string cRst = useColor ? Utils::Color::RESET : "";

    std::cout << cBrd << "+-------------+----------------------+--------------------------+-----------------+----------+\n" << cRst;
    std::cout << cBrd << "| " << Utils::Color::BOLD << "Customer ID | Full Name            | Email                    | Phone           | Accounts " << cBrd << "|\n" << cRst;
    std::cout << cBrd << "+-------------+----------------------+--------------------------+-----------------+----------+\n" << cRst;
    for (const auto& [id, cust] : customers) {
        std::cout << cBrd << "| " << Utils::Color::PURPLE << std::left << std::setw(12) << cust.getCustomerId()
                  << cBrd << "| " << Utils::Color::WHITE << std::setw(21) << cust.getFullName()
                  << cBrd << "| " << Utils::Color::GRAY << std::setw(25) << cust.getEmail()
                  << cBrd << "| " << Utils::Color::GRAY << std::setw(16) << cust.getPhone()
                  << cBrd << "| " << Utils::Color::GREEN << std::right << std::setw(8) << cust.getAccountNumbers().size() << cBrd << " |\n" << cRst;
    }
    std::cout << cBrd << "+-------------+----------------------+--------------------------+-----------------+----------+\n" << cRst;
}

void Bank::displayAllAccounts(bool useColor) const {
    Utils::printHeader("ACTIVE ACCOUNTS (" + std::to_string(accounts.size()) + ")", Utils::Color::CYAN);
    if (accounts.empty()) {
        std::cout << "  No accounts created yet.\n";
        return;
    }

    std::string cBrd = useColor ? Utils::Color::CYAN : "";
    std::string cGrn = useColor ? Utils::Color::GREEN + Utils::Color::BOLD : "";
    std::string cGld = useColor ? Utils::Color::GOLD + Utils::Color::BOLD : "";
    std::string cRst = useColor ? Utils::Color::RESET : "";

    std::cout << cBrd << "+----------------+-------------+-------------+--------------------+--------------+-----------------+\n" << cRst;
    std::cout << cBrd << "| " << Utils::Color::BOLD << "Account Number | Customer ID | Type        | Balance            | Total Txns   | Security Status " << cBrd << "|\n" << cRst;
    std::cout << cBrd << "+----------------+-------------+-------------+--------------------+--------------+-----------------+\n" << cRst;
    double totalBankHoldings = 0.0;
    for (const auto& [accNum, acc] : accounts) {
        std::cout << cBrd << "| " << Utils::Color::WHITE << std::left << std::setw(15) << acc.getAccountNumber()
                  << cBrd << "| " << Utils::Color::PURPLE << std::setw(12) << acc.getCustomerId()
                  << cBrd << "| " << Utils::Color::CYAN << std::setw(12) << acc.getAccountTypeString()
                  << cBrd << "| " << cGrn << std::right << std::setw(18) << Utils::formatCurrency(acc.getBalance())
                  << cBrd << " | " << Utils::Color::WHITE << std::right << std::setw(12) << acc.getTransactionHistory().size()
                  << cBrd << " | " << Utils::Color::GREEN << std::left << std::setw(15) << "PIN [****]"
                  << cBrd << " |\n" << cRst;
        totalBankHoldings += acc.getBalance();
    }
    std::cout << cBrd << "+----------------+-------------+-------------+--------------------+--------------+-----------------+\n" << cRst;
    std::cout << "  " << Utils::Color::WHITE << "Total Bank Vault Holdings: " << cGld << Utils::formatCurrency(totalBankHoldings) << cRst << "\n";
}

void Bank::seedSampleData() {
    // Customer 1: Alice Johnson
    std::string c1 = createCustomer("Alice Johnson", "alice@example.com", "+1-555-0101", "124 Market St, Suite 400");
    std::string a1 = createAccount(c1, AccountType::SAVINGS, 5000.00, "", "1234");
    std::string a2 = createAccount(c1, AccountType::CHECKING, 1200.00, "", "1234");

    // Customer 2: Bob Smith
    std::string c2 = createCustomer("Bob Smith", "bob.smith@example.com", "+1-555-0202", "742 Evergreen Terrace");
    std::string b1 = createAccount(c2, AccountType::SAVINGS, 8500.00, "", "4321");

    // Customer 3: Clara Oswald
    std::string c3 = createCustomer("Clara Oswald", "clara.o@example.com", "+1-555-0303", "221B Baker Street");
    std::string c_acc = createAccount(c3, AccountType::CHECKING, 3500.00, "", "9999");

    // Perform sample operations with PINs
    deposit(a1, 1500.00, "Salary Deposit");
    withdraw(a1, 300.00, "1234", "ATM Cash Withdrawal");
    transferFunds(a1, b1, 750.00, "1234", "Monthly Shared Rent");
    transferFunds(b1, c_acc, 250.00, "4321", "Freelance Design Payment");
    deposit(c_acc, 1000.00, "Dividend Credit");
}
