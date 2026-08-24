#pragma once

#include <string>
#include <vector>
#include <iostream>

class Customer {
private:
    std::string customerId;
    std::string fullName;
    std::string email;
    std::string phone;
    std::string address;
    std::vector<std::string> accountNumbers;

public:
    Customer() = default;
    Customer(std::string id,
             std::string name,
             std::string mail,
             std::string ph,
             std::string addr);

    // Getters
    [[nodiscard]] const std::string& getCustomerId() const;
    [[nodiscard]] const std::string& getFullName() const;
    [[nodiscard]] const std::string& getEmail() const;
    [[nodiscard]] const std::string& getPhone() const;
    [[nodiscard]] const std::string& getAddress() const;
    [[nodiscard]] const std::vector<std::string>& getAccountNumbers() const;

    // Setters
    void setFullName(const std::string& name);
    void setEmail(const std::string& mail);
    void setPhone(const std::string& ph);
    void setAddress(const std::string& addr);

    // Account management
    void addAccount(const std::string& accountNumber);
    bool removeAccount(const std::string& accountNumber);
    [[nodiscard]] bool hasAccount(const std::string& accountNumber) const;

    // Display
    void displayProfile(bool useColor = true) const;
};
