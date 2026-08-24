#include "Bank.hpp"
#include "Utils.hpp"
#include <iostream>

void offerReceipt(const Bank& bank, const std::string& txnId) {
    if (txnId.empty()) return;
    std::string choice = Utils::readLine("\nWould you like a transaction receipt? (y/N): ");
    if (choice == "y" || choice == "Y" || choice == "yes" || choice == "YES") {
        std::cout << "\n" << bank.getFormattedReceipt(txnId, true) << "\n";
        std::string exp = Utils::readLine("Export receipt to text file? (y/N): ");
        if (exp == "y" || exp == "Y" || exp == "yes" || exp == "YES") {
            if (bank.exportReceiptToFile(txnId, "receipts")) {
                Utils::printSuccess("Receipt successfully exported to 'receipts/receipt_" + txnId + ".txt'");
            } else {
                Utils::printError("Failed to export receipt file.");
            }
        }
    }
}

void handleCreateCustomer(Bank& bank) {
    Utils::printSubHeader("REGISTER NEW CUSTOMER", Utils::Color::PURPLE);
    std::string name = Utils::readLine("Enter Customer Full Name  : ");
    if (name.empty()) {
        Utils::printError("Customer name cannot be empty.");
        return;
    }
    std::string email = Utils::readLine("Enter Email Address       : ");
    std::string phone = Utils::readLine("Enter Phone Number         : ");
    std::string address = Utils::readLine("Enter Residential Address : ");

    std::string customerId = bank.createCustomer(name, email, phone, address);
    if (!customerId.empty()) {
        Utils::printSuccess("Customer registered successfully!");
        std::cout << "  " << Utils::Color::WHITE << "Assigned Customer ID: " << Utils::Color::PURPLE << Utils::Color::BOLD << customerId << Utils::Color::RESET << "\n";
    } else {
        Utils::printError("Failed to register customer.");
    }
}

void handleCreateAccount(Bank& bank) {
    Utils::printSubHeader("OPEN NEW ACCOUNT", Utils::Color::CYAN);
    std::string custId = Utils::readLine("Enter Customer ID: ");
    if (!bank.customerExists(custId)) {
        Utils::printError("Customer ID '" + custId + "' does not exist. Please register the customer first.");
        return;
    }

    std::cout << "\n" << Utils::Color::BOLD << "Select Account Type:\n" << Utils::Color::RESET;
    std::cout << "  " << Utils::Color::GREEN << "[1]" << Utils::Color::WHITE << " SAVINGS ACCOUNT\n" << Utils::Color::RESET;
    std::cout << "  " << Utils::Color::CYAN << "[2]" << Utils::Color::WHITE << " CHECKING ACCOUNT\n" << Utils::Color::RESET;
    int typeChoice = Utils::readInt("Choice (1-2): ", 1, 2);
    AccountType type = (typeChoice == 1) ? AccountType::SAVINGS : AccountType::CHECKING;

    double initialDeposit = Utils::readDouble("Enter Initial Deposit ($): ", 0.0, 1e9);

    std::string pin;
    while (true) {
        pin = Utils::readMaskedPin("Set 4-Digit Security PIN: ");
        if (Account::isValidPin(pin)) {
            break;
        }
        Utils::printError("PIN must be exactly 4 numeric digits (e.g. 1234).");
    }

    std::string accNum = bank.createAccount(custId, type, initialDeposit, "", pin);
    if (!accNum.empty()) {
        Utils::printSuccess("Account opened successfully with PIN protection!");
        std::cout << "  " << Utils::Color::WHITE << "New Account Number: " << Utils::Color::CYAN << Utils::Color::BOLD << accNum << Utils::Color::RESET << "\n";
        std::cout << "  " << Utils::Color::WHITE << "Opening Balance   : " << Utils::Color::GREEN << Utils::formatCurrency(initialDeposit) << Utils::Color::RESET << "\n";
        std::cout << "  " << Utils::Color::WHITE << "Security PIN      : " << Utils::Color::GOLD << "**** (Configured)" << Utils::Color::RESET << "\n";

        const Account* acc = bank.getAccount(accNum);
        if (acc && !acc->getTransactionHistory().empty()) {
            offerReceipt(bank, acc->getTransactionHistory().front().getTransactionId());
        }
    } else {
        Utils::printError("Failed to open account.");
    }
}

void handleDeposit(Bank& bank) {
    Utils::printSubHeader("DEPOSIT FUNDS", Utils::Color::GREEN);
    std::string accNum = Utils::readLine("Enter Account Number: ");
    Account* acc = bank.getAccount(accNum);
    if (!acc) {
        Utils::printError("Account '" + accNum + "' not found.");
        return;
    }

    std::cout << "Current Balance: " << Utils::Color::GREEN << Utils::formatCurrency(acc->getBalance()) << Utils::Color::RESET << "\n";
    double amount = Utils::readDouble("Enter Deposit Amount ($): ", 0.01, 1e9);
    std::string remarks = Utils::readLine("Enter Remarks / Note (Press enter for default): ");
    if (remarks.empty()) remarks = "Cash Deposit";

    if (bank.deposit(accNum, amount, remarks)) {
        Utils::printSuccess("Deposit processed successfully!");
        std::cout << "  " << Utils::Color::WHITE << "Deposited Amount: " << Utils::Color::GREEN << Utils::formatCurrency(amount) << Utils::Color::RESET << "\n";
        std::cout << "  " << Utils::Color::WHITE << "Updated Balance : " << Utils::Color::GREEN << Utils::Color::BOLD << Utils::formatCurrency(acc->getBalance()) << Utils::Color::RESET << "\n";
        
        offerReceipt(bank, acc->getTransactionHistory().back().getTransactionId());
    } else {
        Utils::printError("Failed to process deposit.");
    }
}

void handleWithdraw(Bank& bank) {
    Utils::printSubHeader("WITHDRAW FUNDS", Utils::Color::CORAL);
    std::string accNum = Utils::readLine("Enter Account Number: ");
    Account* acc = bank.getAccount(accNum);
    if (!acc) {
        Utils::printError("Account '" + accNum + "' not found.");
        return;
    }

    std::string pin = Utils::readMaskedPin("Enter Account Security PIN: ");
    if (!acc->verifyPin(pin)) {
        Utils::printError("Security PIN verification failed! Access denied.");
        return;
    }

    std::cout << "Available Balance: " << Utils::Color::GREEN << Utils::formatCurrency(acc->getBalance()) << Utils::Color::RESET << "\n";
    double amount = Utils::readDouble("Enter Withdrawal Amount ($): ", 0.01, 1e9);

    if (amount > acc->getBalance()) {
        Utils::printError("Insufficient funds! Account balance is " + Utils::formatCurrency(acc->getBalance()) + ".");
        return;
    }

    std::string remarks = Utils::readLine("Enter Remarks / Note (Press enter for default): ");
    if (remarks.empty()) remarks = "Cash Withdrawal";

    if (bank.withdraw(accNum, amount, pin, remarks)) {
        Utils::printSuccess("Withdrawal processed successfully!");
        std::cout << "  " << Utils::Color::WHITE << "Withdrawn Amount: " << Utils::Color::CORAL << Utils::formatCurrency(amount) << Utils::Color::RESET << "\n";
        std::cout << "  " << Utils::Color::WHITE << "Remaining Balance: " << Utils::Color::GREEN << Utils::Color::BOLD << Utils::formatCurrency(acc->getBalance()) << Utils::Color::RESET << "\n";

        offerReceipt(bank, acc->getTransactionHistory().back().getTransactionId());
    } else {
        Utils::printError("Failed to process withdrawal.");
    }
}

void handleTransfer(Bank& bank) {
    Utils::printSubHeader("TRANSFER FUNDS", Utils::Color::GOLD);
    std::string fromAccNum = Utils::readLine("Enter Source Account Number     : ");
    Account* fromAcc = bank.getAccount(fromAccNum);
    if (!fromAcc) {
        Utils::printError("Source account '" + fromAccNum + "' not found.");
        return;
    }

    std::string toAccNum = Utils::readLine("Enter Destination Account Number: ");
    if (fromAccNum == toAccNum) {
        Utils::printError("Source and Destination accounts cannot be the same.");
        return;
    }

    Account* toAcc = bank.getAccount(toAccNum);
    if (!toAcc) {
        Utils::printError("Destination account '" + toAccNum + "' not found.");
        return;
    }

    std::string pin = Utils::readMaskedPin("Enter Source Account PIN: ");
    if (!fromAcc->verifyPin(pin)) {
        Utils::printError("PIN verification failed for source account! Transfer aborted.");
        return;
    }

    std::cout << "Available Balance in " << fromAccNum << ": " << Utils::Color::GREEN << Utils::formatCurrency(fromAcc->getBalance()) << Utils::Color::RESET << "\n";
    double amount = Utils::readDouble("Enter Transfer Amount ($)        : ", 0.01, 1e9);

    if (amount > fromAcc->getBalance()) {
        Utils::printError("Insufficient funds in source account! Available: " + Utils::formatCurrency(fromAcc->getBalance()) + ".");
        return;
    }

    std::string remarks = Utils::readLine("Enter Transfer Description/Note  : ");
    if (remarks.empty()) remarks = "Fund Transfer";

    if (bank.transferFunds(fromAccNum, toAccNum, amount, pin, remarks)) {
        Utils::printSuccess("Fund Transfer executed successfully!");
        std::cout << "  " << Utils::Color::WHITE << "Transferred Amount       : " << Utils::Color::GOLD << Utils::Color::BOLD << Utils::formatCurrency(amount) << Utils::Color::RESET << "\n";
        std::cout << "  " << Utils::Color::WHITE << "Source Balance (" << fromAccNum << ") : " << Utils::Color::GREEN << Utils::formatCurrency(fromAcc->getBalance()) << Utils::Color::RESET << "\n";
        std::cout << "  " << Utils::Color::WHITE << "Target Balance (" << toAccNum << ") : " << Utils::Color::GREEN << Utils::formatCurrency(toAcc->getBalance()) << Utils::Color::RESET << "\n";

        offerReceipt(bank, fromAcc->getTransactionHistory().back().getTransactionId());
    } else {
        Utils::printError("Transfer transaction failed.");
    }
}

void handleChangePin(Bank& bank) {
    Utils::printSubHeader("CHANGE ACCOUNT SECURITY PIN", Utils::Color::GOLD);
    std::string accNum = Utils::readLine("Enter Account Number: ");
    Account* acc = bank.getAccount(accNum);
    if (!acc) {
        Utils::printError("Account '" + accNum + "' not found.");
        return;
    }

    std::string oldPin = Utils::readMaskedPin("Enter Current PIN: ");
    if (!acc->verifyPin(oldPin)) {
        Utils::printError("Incorrect current PIN. Cannot modify security settings.");
        return;
    }

    std::string newPin = Utils::readMaskedPin("Enter New 4-Digit PIN: ");
    if (!Account::isValidPin(newPin)) {
        Utils::printError("New PIN must be exactly 4 digits.");
        return;
    }

    std::string confirmPin = Utils::readMaskedPin("Confirm New 4-Digit PIN: ");
    if (newPin != confirmPin) {
        Utils::printError("PIN confirmation does not match.");
        return;
    }

    if (bank.changeAccountPin(accNum, oldPin, newPin)) {
        Utils::printSuccess("Account PIN updated successfully!");
    } else {
        Utils::printError("Failed to update PIN.");
    }
}

void handleViewAccount(Bank& bank) {
    Utils::printSubHeader("VIEW ACCOUNT DETAILS", Utils::Color::CYAN);
    std::string accNum = Utils::readLine("Enter Account Number: ");
    bank.displayAccountSummary(accNum, true);
}

void handleViewTransactions(Bank& bank) {
    Utils::printSubHeader("VIEW TRANSACTION HISTORY", Utils::Color::PURPLE);
    std::string accNum = Utils::readLine("Enter Account Number: ");
    Account* acc = bank.getAccount(accNum);
    if (!acc) {
        Utils::printError("Account '" + accNum + "' not found.");
        return;
    }

    std::cout << "\n" << Utils::Color::BOLD << "Select History Option:\n" << Utils::Color::RESET;
    std::cout << "  " << Utils::Color::CYAN << "[1]" << Utils::Color::WHITE << " View All Transactions\n" << Utils::Color::RESET;
    std::cout << "  " << Utils::Color::PURPLE << "[2]" << Utils::Color::WHITE << " View Last N Recent Transactions (Mini-Statement)\n" << Utils::Color::RESET;
    int choice = Utils::readInt("Choice (1-2): ", 1, 2);

    if (choice == 1) {
        bank.displayAccountTransactions(accNum, 0, true);
    } else {
        int n = Utils::readInt("Enter number of recent transactions to display: ", 1, 1000);
        bank.displayAccountTransactions(accNum, static_cast<size_t>(n), true);
    }
}

void handleReceiptLookup(Bank& bank) {
    Utils::printSubHeader("RECEIPT LOOKUP & REPRINT", Utils::Color::GOLD);
    std::string txnId = Utils::readLine("Enter Transaction ID (e.g. TXN-10001): ");
    std::string receipt = bank.getFormattedReceipt(txnId, true);
    if (receipt.empty()) {
        Utils::printError("Transaction ID '" + txnId + "' not found.");
        return;
    }

    std::cout << "\n" << receipt << "\n";
    std::string exp = Utils::readLine("Export receipt to text file? (y/N): ");
    if (exp == "y" || exp == "Y" || exp == "yes" || exp == "YES") {
        if (bank.exportReceiptToFile(txnId, "receipts")) {
            Utils::printSuccess("Receipt exported to 'receipts/receipt_" + txnId + ".txt'");
        } else {
            Utils::printError("Failed to export receipt file.");
        }
    }
}

void handleViewCustomer(Bank& bank) {
    Utils::printSubHeader("VIEW CUSTOMER PORTFOLIO", Utils::Color::PURPLE);
    std::string custId = Utils::readLine("Enter Customer ID: ");
    bank.displayCustomerSummary(custId, true);
}

int main() {
    Utils::initTerminal();
    Bank bank("RAY Bank");

    bool running = true;
    while (running) {
        Utils::printBanner(bank.getBankName());
        std::cout << "  " << Utils::Color::PURPLE << "[1] " << Utils::Color::WHITE  << " Register New Customer\n";
        std::cout << "  " << Utils::Color::CYAN   << "[2] " << Utils::Color::WHITE  << " Open New Bank Account " << Utils::Color::GOLD << "(PIN Protected)\n";
        std::cout << "  " << Utils::Color::GREEN  << "[3] " << Utils::Color::WHITE  << " Deposit Funds\n";
        std::cout << "  " << Utils::Color::CORAL  << "[4] " << Utils::Color::WHITE  << " Withdraw Funds " << Utils::Color::CORAL << "(Requires PIN)\n";
        std::cout << "  " << Utils::Color::GOLD   << "[5] " << Utils::Color::WHITE  << " Transfer Funds " << Utils::Color::GOLD << "(Requires PIN)\n";
        std::cout << "  " << Utils::Color::CYAN   << "[6] " << Utils::Color::WHITE  << " View Account Details & Balance\n";
        std::cout << "  " << Utils::Color::PURPLE << "[7] " << Utils::Color::WHITE  << " View Transaction Statement / History\n";
        std::cout << "  " << Utils::Color::GOLD   << "[8] " << Utils::Color::WHITE  << " Lookup & Export Transaction Receipt\n";
        std::cout << "  " << Utils::Color::MAGENTA<< "[9] " << Utils::Color::WHITE  << " Change Account Security PIN\n";
        std::cout << "  " << Utils::Color::PURPLE << "[10]" << Utils::Color::WHITE  << " View Customer Profile & Accounts\n";
        std::cout << "  " << Utils::Color::CYAN   << "[11]" << Utils::Color::WHITE  << " List All Registered Customers\n";
        std::cout << "  " << Utils::Color::GREEN  << "[12]" << Utils::Color::WHITE  << " List All Active Accounts\n";
        std::cout << "  " << Utils::Color::GOLD   << "[13]" << Utils::Color::WHITE  << " Load Demo Sample Data " << Utils::Color::GRAY << "(Alice, Bob, Clara)\n";
        std::cout << "  " << Utils::Color::CORAL  << "[0] " << Utils::Color::WHITE  << " Exit System\n";
        Utils::printDivider('-', 75, Utils::Color::CYAN);

        int choice = Utils::readInt("Enter your selection (0-13): ", 0, 13);

        switch (choice) {
            case 1:  handleCreateCustomer(bank); break;
            case 2:  handleCreateAccount(bank); break;
            case 3:  handleDeposit(bank); break;
            case 4:  handleWithdraw(bank); break;
            case 5:  handleTransfer(bank); break;
            case 6:  handleViewAccount(bank); break;
            case 7:  handleViewTransactions(bank); break;
            case 8:  handleReceiptLookup(bank); break;
            case 9:  handleChangePin(bank); break;
            case 10: handleViewCustomer(bank); break;
            case 11: bank.displayAllCustomers(true); break;
            case 12: bank.displayAllAccounts(true); break;
            case 13:
                bank.seedSampleData();
                Utils::printSuccess("Demo sample data loaded successfully! (Default PINs: Alice=1234, Bob=4321, Clara=9999)");
                break;
            case 0:
                Utils::printInfo("Thank you for using " + bank.getBankName() + ". Goodbye!");
                running = false;
                break;
            default:
                break;
        }

        if (running) {
            Utils::pressEnterToContinue();
        }
    }

    return 0;
}
