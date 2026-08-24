#pragma once

#include <string>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <iostream>
#include <limits>

namespace Utils {

    // Rich ANSI Color Palette (256-color & Standard ANSI)
    namespace Color {
        inline const std::string RESET       = "\033[0m";
        inline const std::string BOLD        = "\033[1m";
        inline const std::string DIM         = "\033[2m";
        inline const std::string ITALIC      = "\033[3m";
        inline const std::string UNDERLINE   = "\033[4m";

        // Foreground Colors
        inline const std::string CYAN        = "\033[38;5;51m";   // Neon Aqua / Cyan
        inline const std::string GREEN       = "\033[38;5;48m";   // Electric Emerald Green
        inline const std::string GOLD        = "\033[38;5;220m";  // Bright Amber / Gold
        inline const std::string PURPLE      = "\033[38;5;141m";  // Neon Violet / Purple
        inline const std::string CORAL       = "\033[38;5;203m";  // Vivid Coral / Rose
        inline const std::string BLUE        = "\033[38;5;39m";   // Deep Sky Blue
        inline const std::string MAGENTA     = "\033[38;5;201m";  // Neon Magenta
        inline const std::string WHITE       = "\033[38;5;255m";  // Crisp White
        inline const std::string GRAY        = "\033[38;5;245m";  // Slate Gray
        inline const std::string DARK_GRAY   = "\033[38;5;238m";  // Dark Slate

        // Background Accents
        inline const std::string BG_DARK_BLUE = "\033[48;5;17m";
        inline const std::string BG_PURPLE    = "\033[48;5;54m";
        inline const std::string BG_CYAN      = "\033[48;5;24m";
    }

    // Windows Terminal ANSI support initialization
    void initTerminal();

    // Formatting & Time
    std::string getCurrentTimestamp();
    std::string formatCurrency(double amount);

    // ID Generators
    std::string generateCustomerId();
    std::string generateAccountNumber();
    std::string generateTransactionId();
    void resetIdCounters(int custStart = 1001, int accStart = 1001, int txnStart = 10001);

    // Console UI Helpers (Mixed Colors)
    void printBanner(const std::string& bankName);
    void printHeader(const std::string& title, const std::string& color = Color::CYAN);
    void printSubHeader(const std::string& title, const std::string& color = Color::PURPLE);
    void printDivider(char ch = '=', int length = 75, const std::string& color = Color::GRAY);
    void printSuccess(const std::string& msg);
    void printError(const std::string& msg);
    void printInfo(const std::string& msg);
    void printWarning(const std::string& msg);
    void pressEnterToContinue();

    // Input Handlers
    std::string readLine(const std::string& prompt);
    std::string readMaskedPin(const std::string& prompt, size_t expectedLength = 4);
    double readDouble(const std::string& prompt, double minVal = 0.0, double maxVal = 1e9);
    int readInt(const std::string& prompt, int minVal = 0, int maxVal = 1000000);
}
