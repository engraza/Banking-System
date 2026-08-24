#include "Bank.hpp"
#include "Utils.hpp"
#include <iostream>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "[-] FAILED: " << message << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            testsFailed++; \
            return; \
        } \
    } while (0)

#define TEST_PASS(testName) \
    do { \
        std::cout << "[+] PASSED: " << testName << "\n"; \
        testsPassed++; \
    } while (0)

int testsPassed = 0;
int testsFailed = 0;

void testCustomerCreation() {
    Bank bank("Test Bank");
    std::string cid = bank.createCustomer("John Doe", "john@example.com", "+1-123-456", "100 Main St");
    TEST_ASSERT(!cid.empty(), "Customer ID should not be empty");
    TEST_ASSERT(bank.customerExists(cid), "Customer should exist in bank");

    const Customer* cust = bank.getCustomer(cid);
    TEST_ASSERT(cust != nullptr, "Customer pointer should not be null");
    TEST_ASSERT(cust->getFullName() == "John Doe", "Customer name mismatch");
    TEST_ASSERT(cust->getEmail() == "john@example.com", "Customer email mismatch");
    TEST_ASSERT(cust->getPhone() == "+1-123-456", "Customer phone mismatch");
    TEST_ASSERT(cust->getAddress() == "100 Main St", "Customer address mismatch");
    TEST_ASSERT(cust->getAccountNumbers().empty(), "Customer should have no accounts initially");

    // Invalid customer (empty name)
    std::string invalidCid = bank.createCustomer("", "a@b.com", "123", "addr");
    TEST_ASSERT(invalidCid.empty(), "Customer with empty name should fail");

    TEST_PASS("testCustomerCreation");
}

void testAccountCreationAndInitialDeposit() {
    Bank bank("Test Bank");
    std::string cid = bank.createCustomer("Sarah Connor", "sarah@cyber.com", "555-1234", "LA");
    
    // Create Savings Account with initial deposit & custom PIN
    std::string acc1 = bank.createAccount(cid, AccountType::SAVINGS, 1000.0, "", "7788");
    TEST_ASSERT(!acc1.empty(), "Account creation should succeed");
    TEST_ASSERT(bank.accountExists(acc1), "Account should exist in bank");

    const Account* a1 = bank.getAccount(acc1);
    TEST_ASSERT(a1 != nullptr, "Account pointer should not be null");
    TEST_ASSERT(a1->getCustomerId() == cid, "Account owner should match customer");
    TEST_ASSERT(a1->getAccountType() == AccountType::SAVINGS, "Account type should be SAVINGS");
    TEST_ASSERT(std::abs(a1->getBalance() - 1000.0) < 1e-6, "Balance should match initial deposit");
    TEST_ASSERT(a1->verifyPin("7788"), "PIN should match 7788");
    TEST_ASSERT(!a1->verifyPin("0000"), "Wrong PIN should fail");

    // Create Checking Account with 0 balance and default PIN
    std::string acc2 = bank.createAccount(cid, AccountType::CHECKING, 0.0);
    const Account* a2 = bank.getAccount(acc2);
    TEST_ASSERT(a2 != nullptr, "Checking account should exist");
    TEST_ASSERT(std::abs(a2->getBalance() - 0.0) < 1e-6, "Balance should be 0");
    TEST_ASSERT(a2->verifyPin("1234"), "Default PIN should be 1234");

    TEST_PASS("testAccountCreationAndInitialDeposit");
}

void testDepositOperations() {
    Bank bank("Test Bank");
    std::string cid = bank.createCustomer("Alice", "alice@test.com", "111", "NYC");
    std::string accNum = bank.createAccount(cid, AccountType::SAVINGS, 500.0, "", "1234");

    // Valid deposit
    bool depOk = bank.deposit(accNum, 250.0, "Salary Bonus");
    TEST_ASSERT(depOk, "Deposit of 250 should succeed");
    
    const Account* acc = bank.getAccount(accNum);
    TEST_ASSERT(std::abs(acc->getBalance() - 750.0) < 1e-6, "Balance should be 750.00");
    TEST_ASSERT(acc->getTransactionHistory().size() == 2, "Should have 2 transactions");

    // Invalid deposit amounts
    TEST_ASSERT(!bank.deposit(accNum, 0.0), "Deposit of 0 should fail");
    TEST_ASSERT(!bank.deposit(accNum, -50.0), "Deposit of negative amount should fail");
    TEST_ASSERT(!bank.deposit("INVALID_ACC", 100.0), "Deposit to invalid account should fail");

    TEST_PASS("testDepositOperations");
}

void testPinVerificationAndWithdrawals() {
    Bank bank("Test Bank");
    std::string cid = bank.createCustomer("Bob", "bob@test.com", "222", "Chicago");
    std::string accNum = bank.createAccount(cid, AccountType::SAVINGS, 1000.0, "", "5555");

    // Wrong PIN withdrawal
    bool wrongPinOk = bank.withdraw(accNum, 200.0, "9999", "Failed attempt");
    TEST_ASSERT(!wrongPinOk, "Withdrawal with wrong PIN must fail");
    TEST_ASSERT(std::abs(bank.getAccount(accNum)->getBalance() - 1000.0) < 1e-6, "Balance should remain unchanged");

    // Correct PIN withdrawal
    bool correctPinOk = bank.withdraw(accNum, 300.0, "5555", "ATM Cash");
    TEST_ASSERT(correctPinOk, "Withdrawal with correct PIN must succeed");
    TEST_ASSERT(std::abs(bank.getAccount(accNum)->getBalance() - 700.0) < 1e-6, "Balance should be 700.00");

    // Insufficient funds with correct PIN
    bool overdrawOk = bank.withdraw(accNum, 1000.0, "5555", "Overdraft");
    TEST_ASSERT(!overdrawOk, "Overdraft must fail even with correct PIN");

    // Change PIN test
    bool badOldPin = bank.changeAccountPin(accNum, "0000", "6666");
    TEST_ASSERT(!badOldPin, "Changing PIN with wrong old PIN must fail");

    bool badNewPinFormat = bank.changeAccountPin(accNum, "5555", "12a4");
    TEST_ASSERT(!badNewPinFormat, "Changing PIN with non-numeric PIN must fail");

    bool pinChangeOk = bank.changeAccountPin(accNum, "5555", "6666");
    TEST_ASSERT(pinChangeOk, "Changing PIN with valid old and new PIN must succeed");
    TEST_ASSERT(bank.getAccount(accNum)->verifyPin("6666"), "New PIN must be 6666");

    TEST_PASS("testPinVerificationAndWithdrawals");
}

void testPinProtectedFundTransfers() {
    Bank bank("Test Bank");
    std::string c1 = bank.createCustomer("Alice", "alice@test.com", "111", "NYC");
    std::string c2 = bank.createCustomer("Bob", "bob@test.com", "222", "Chicago");

    std::string accSender = bank.createAccount(c1, AccountType::CHECKING, 2000.0, "", "1111");
    std::string accReceiver = bank.createAccount(c2, AccountType::SAVINGS, 500.0, "", "2222");

    // Wrong PIN transfer
    bool badPinXfer = bank.transferFunds(accSender, accReceiver, 500.0, "9999", "Failed transfer");
    TEST_ASSERT(!badPinXfer, "Transfer with wrong PIN must fail");
    TEST_ASSERT(std::abs(bank.getAccount(accSender)->getBalance() - 2000.0) < 1e-6, "Sender balance unchanged");
    TEST_ASSERT(std::abs(bank.getAccount(accReceiver)->getBalance() - 500.0) < 1e-6, "Receiver balance unchanged");

    // Correct PIN transfer
    bool goodPinXfer = bank.transferFunds(accSender, accReceiver, 500.0, "1111", "Rent Share");
    TEST_ASSERT(goodPinXfer, "Transfer with correct PIN must succeed");
    TEST_ASSERT(std::abs(bank.getAccount(accSender)->getBalance() - 1500.0) < 1e-6, "Sender balance should be 1500.00");
    TEST_ASSERT(std::abs(bank.getAccount(accReceiver)->getBalance() - 1000.0) < 1e-6, "Receiver balance should be 1000.00");

    TEST_PASS("testPinProtectedFundTransfers");
}

void testReceiptGenerationAndFileExport() {
    Bank bank("RAY Bank");
    std::string cid = bank.createCustomer("Diana Prince", "diana@themyscira.io", "+1-555-8888", "1 Gateway Blvd");
    std::string accNum = bank.createAccount(cid, AccountType::SAVINGS, 5000.0, "", "8888");

    bank.deposit(accNum, 1200.0, "Consulting Fee");
    const Account* acc = bank.getAccount(accNum);
    std::string txnId = acc->getTransactionHistory().back().getTransactionId();

    // Receipt formatting test
    std::string receiptText = bank.getFormattedReceipt(txnId, false);
    TEST_ASSERT(!receiptText.empty(), "Receipt text should not be empty");
    TEST_ASSERT(receiptText.find("Diana Prince") != std::string::npos, "Receipt should contain customer name");
    TEST_ASSERT(receiptText.find(accNum) != std::string::npos, "Receipt should contain account number");
    TEST_ASSERT(receiptText.find("1,200.00") != std::string::npos, "Receipt should contain transaction amount");
    TEST_ASSERT(receiptText.find("6,200.00") != std::string::npos, "Receipt should contain balance after");

    // File export test
    std::string testDir = "build/test_receipts";
    bool exported = bank.exportReceiptToFile(txnId, testDir);
    TEST_ASSERT(exported, "Receipt file export should return true");
    
    std::string expectedFile = testDir + "/receipt_" + txnId + ".txt";
    TEST_ASSERT(std::filesystem::exists(expectedFile), "Exported file must exist on disk");

    std::ifstream inFile(expectedFile);
    TEST_ASSERT(inFile.is_open(), "Exported file must be readable");
    std::string fileContent((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    TEST_ASSERT(fileContent.find(txnId) != std::string::npos, "File content should contain transaction ID");

    TEST_PASS("testReceiptGenerationAndFileExport");
}

int main() {
    std::cout << "\n======================================================\n";
    std::cout << "           RUNNING BANKING SYSTEM TEST SUITE          \n";
    std::cout << "======================================================\n";

    testCustomerCreation();
    testAccountCreationAndInitialDeposit();
    testDepositOperations();
    testPinVerificationAndWithdrawals();
    testPinProtectedFundTransfers();
    testReceiptGenerationAndFileExport();

    std::cout << "\n======================================================\n";
    std::cout << "  TEST RESULTS: " << testsPassed << " Passed, " << testsFailed << " Failed\n";
    std::cout << "======================================================\n";

    return (testsFailed == 0) ? 0 : 1;
}
