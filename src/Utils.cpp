#include "Utils.hpp"
#include <atomic>
#include <ctime>
#include <cmath>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <conio.h>
#include <io.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

namespace {
    std::atomic<int> nextCustomerId{1001};
    std::atomic<int> nextAccountNumber{1001};
    std::atomic<int> nextTransactionId{10001};
}

namespace Utils {

    void initTerminal() {
#if defined(_WIN32) || defined(_WIN64)
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode)) {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);
            }
        }
        SetConsoleOutputCP(CP_UTF8);
#endif
    }

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf{};
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm_buf, &now_c);
#else
        localtime_r(&now_c, &tm_buf);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    std::string formatCurrency(double amount) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        
        bool isNegative = (amount < 0.0);
        double absAmt = std::abs(amount);
        
        long long intPart = static_cast<long long>(absAmt);
        int decPart = static_cast<int>(std::round((absAmt - intPart) * 100));
        if (decPart >= 100) {
            intPart += 1;
            decPart = 0;
        }

        std::string intStr = std::to_string(intPart);
        std::string formattedInt;
        int count = 0;
        for (int i = static_cast<int>(intStr.length()) - 1; i >= 0; --i) {
            formattedInt = intStr[i] + formattedInt;
            count++;
            if (count % 3 == 0 && i > 0) {
                formattedInt = "," + formattedInt;
            }
        }

        if (isNegative) {
            oss << "-$" << formattedInt << "." << (decPart < 10 ? "0" : "") << decPart;
        } else {
            oss << "$" << formattedInt << "." << (decPart < 10 ? "0" : "") << decPart;
        }

        return oss.str();
    }

    std::string generateCustomerId() {
        return "CUST-" + std::to_string(nextCustomerId.fetch_add(1));
    }

    std::string generateAccountNumber() {
        return "ACC-" + std::to_string(nextAccountNumber.fetch_add(1));
    }

    std::string generateTransactionId() {
        return "TXN-" + std::to_string(nextTransactionId.fetch_add(1));
    }

    void resetIdCounters(int custStart, int accStart, int txnStart) {
        nextCustomerId.store(custStart);
        nextAccountNumber.store(accStart);
        nextTransactionId.store(txnStart);
    }

    void printBanner(const std::string& bankName) {
        std::cout << "\n";
        std::cout << Color::CYAN << "  =========================================================================\n" << Color::RESET;
        std::cout << Color::CYAN << "  || " << Color::BOLD << Color::GOLD << " [X] " << Color::PURPLE << bankName << Color::GOLD << " - CORE BANKING SYSTEM " 
                  << Color::CYAN << "             ||\n" << Color::RESET;
        std::cout << Color::CYAN << "  || " << Color::DIM << Color::WHITE << " Secure PIN Protection  *  Real-Time Ledger  *  Digital Receipts" 
                  << Color::CYAN << "     ||\n" << Color::RESET;
        std::cout << Color::CYAN << "  =========================================================================\n" << Color::RESET;
    }

    void printHeader(const std::string& title, const std::string& color) {
        std::cout << "\n";
        printDivider('=', 75, color);
        int padding = (75 - static_cast<int>(title.length())) / 2;
        if (padding < 0) padding = 0;
        std::cout << color << Color::BOLD << std::string(padding, ' ') << title << Color::RESET << "\n";
        printDivider('=', 75, color);
    }

    void printSubHeader(const std::string& title, const std::string& color) {
        std::cout << "\n" << color << Color::BOLD << "--- [ " << title << " ] ---" << Color::RESET << "\n";
    }

    void printDivider(char ch, int length, const std::string& color) {
        std::cout << color << std::string(length, ch) << Color::RESET << "\n";
    }

    void printSuccess(const std::string& msg) {
        std::cout << "\n" << Color::GREEN << Color::BOLD << "[SUCCESS] " << Color::RESET << Color::WHITE << msg << "\n";
    }

    void printError(const std::string& msg) {
        std::cout << "\n" << Color::CORAL << Color::BOLD << "[ERROR] " << Color::RESET << Color::CORAL << msg << Color::RESET << "\n";
    }

    void printInfo(const std::string& msg) {
        std::cout << "\n" << Color::CYAN << Color::BOLD << "[INFO] " << Color::RESET << Color::WHITE << msg << "\n";
    }

    void printWarning(const std::string& msg) {
        std::cout << "\n" << Color::GOLD << Color::BOLD << "[WARNING] " << Color::RESET << Color::GOLD << msg << Color::RESET << "\n";
    }

    void pressEnterToContinue() {
        std::cout << "\n" << Color::GRAY << "Press Enter to continue..." << Color::RESET;
        std::string dummy;
        std::getline(std::cin, dummy);
    }

    std::string readLine(const std::string& prompt) {
        std::cout << Color::BOLD << prompt << Color::RESET;
        std::string input;
        if (!std::getline(std::cin, input)) {
            return "";
        }
        size_t start = input.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = input.find_last_not_of(" \t\r\n");
        return input.substr(start, end - start + 1);
    }

    std::string readMaskedPin(const std::string& prompt, size_t expectedLength) {
        std::cout << Color::BOLD << prompt << Color::RESET;
        std::string pin;

#if defined(_WIN32) || defined(_WIN64)
        // Check if stdin is an interactive console terminal
        if (_isatty(_fileno(stdin))) {
            char ch = 0;
            while (true) {
                ch = static_cast<char>(_getch());
                if (ch == '\r' || ch == '\n') {
                    std::cout << "\n";
                    break;
                } else if (ch == '\b') { // Backspace
                    if (!pin.empty()) {
                        pin.pop_back();
                        std::cout << "\b \b";
                    }
                } else if (std::isdigit(static_cast<unsigned char>(ch))) {
                    if (pin.length() < expectedLength) {
                        pin.push_back(ch);
                        std::cout << Color::GOLD << "*" << Color::RESET;
                    }
                }
            }
            return pin;
        }
#endif
        // Fallback for piped input / non-interactive environment
        std::getline(std::cin, pin);
        size_t start = pin.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = pin.find_last_not_of(" \t\r\n");
        return pin.substr(start, end - start + 1);
    }

    double readDouble(const std::string& prompt, double minVal, double maxVal) {
        while (true) {
            std::cout << Color::BOLD << prompt << Color::RESET;
            std::string line;
            if (!std::getline(std::cin, line)) {
                return minVal;
            }
            size_t start = line.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) {
                continue;
            }
            size_t end = line.find_last_not_of(" \t\r\n");
            line = line.substr(start, end - start + 1);

            try {
                size_t idx = 0;
                double val = std::stod(line, &idx);
                if (idx == line.length() && val >= minVal && val <= maxVal) {
                    return val;
                }
            } catch (...) {}
            std::cout << Color::CORAL << "Invalid input. Please enter an amount between " 
                      << formatCurrency(minVal) << " and " << formatCurrency(maxVal) << ".\n" << Color::RESET;
        }
    }

    int readInt(const std::string& prompt, int minVal, int maxVal) {
        while (true) {
            std::cout << Color::BOLD << prompt << Color::RESET;
            std::string line;
            if (!std::getline(std::cin, line)) {
                return minVal;
            }
            size_t start = line.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) {
                continue;
            }
            size_t end = line.find_last_not_of(" \t\r\n");
            line = line.substr(start, end - start + 1);

            try {
                size_t idx = 0;
                int val = std::stoi(line, &idx);
                if (idx == line.length() && val >= minVal && val <= maxVal) {
                    return val;
                }
            } catch (...) {}
            std::cout << Color::CORAL << "Invalid input. Please enter a valid number between " 
                      << minVal << " and " << maxVal << ".\n" << Color::RESET;
        }
    }
}
