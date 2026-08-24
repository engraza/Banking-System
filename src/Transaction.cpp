#include "Transaction.hpp"
#include "Utils.hpp"
#include <iomanip>
#include <sstream>

Transaction::Transaction(std::string txnId,
                         std::string ts,
                         TransactionType txnType,
                         double amt,
                         double balAfter,
                         std::string relAcc,
                         std::string rem)
    : transactionId(std::move(txnId)),
      timestamp(std::move(ts)),
      type(txnType),
      amount(amt),
      balanceAfter(balAfter),
      relatedAccount(std::move(relAcc)),
      remarks(std::move(rem)) {}

const std::string& Transaction::getTransactionId() const {
    return transactionId;
}

const std::string& Transaction::getTimestamp() const {
    return timestamp;
}

TransactionType Transaction::getType() const {
    return type;
}

std::string Transaction::getTypeString() const {
    switch (type) {
        case TransactionType::DEPOSIT:      return "DEPOSIT";
        case TransactionType::WITHDRAWAL:   return "WITHDRAWAL";
        case TransactionType::TRANSFER_OUT: return "TRANSFER_OUT";
        case TransactionType::TRANSFER_IN:  return "TRANSFER_IN";
        default:                            return "UNKNOWN";
    }
}

double Transaction::getAmount() const {
    return amount;
}

double Transaction::getBalanceAfter() const {
    return balanceAfter;
}

const std::string& Transaction::getRelatedAccount() const {
    return relatedAccount;
}

const std::string& Transaction::getRemarks() const {
    return remarks;
}

void Transaction::display() const {
    std::cout << toFormattedString(true) << "\n";
}

std::string Transaction::toFormattedString(bool useColor) const {
    std::ostringstream oss;
    std::string typeCol = "";
    std::string amtCol = "";
    std::string reset = "";

    if (useColor) {
        reset = Utils::Color::RESET;
        if (type == TransactionType::DEPOSIT || type == TransactionType::TRANSFER_IN) {
            typeCol = Utils::Color::GREEN;
            amtCol = Utils::Color::GREEN + Utils::Color::BOLD;
        } else {
            typeCol = Utils::Color::CORAL;
            amtCol = Utils::Color::CORAL + Utils::Color::BOLD;
        }
    }

    oss << "| " << std::left << std::setw(12) << transactionId
        << "| " << std::setw(20) << timestamp
        << "| " << typeCol << std::setw(14) << getTypeString() << reset
        << "| " << amtCol << std::right << std::setw(12) << Utils::formatCurrency(amount) << reset
        << "| " << std::right << std::setw(12) << Utils::formatCurrency(balanceAfter)
        << "| " << std::left << std::setw(12) << (relatedAccount.empty() ? "-" : relatedAccount)
        << "| " << std::left << std::setw(20) << remarks
        << " |";
    return oss.str();
}

std::string Transaction::generateReceipt(const std::string& bankName,
                                        const std::string& customerName,
                                        const std::string& accountNumber,
                                        bool useColor) const {
    std::ostringstream oss;
    std::string cBrd = useColor ? Utils::Color::CYAN : "";
    std::string cGld = useColor ? Utils::Color::GOLD + Utils::Color::BOLD : "";
    std::string cGrn = useColor ? Utils::Color::GREEN + Utils::Color::BOLD : "";
    std::string cRed = useColor ? Utils::Color::CORAL + Utils::Color::BOLD : "";
    std::string cPur = useColor ? Utils::Color::PURPLE : "";
    std::string cWht = useColor ? Utils::Color::WHITE + Utils::Color::BOLD : "";
    std::string cRst = useColor ? Utils::Color::RESET : "";

    bool isCredit = (type == TransactionType::DEPOSIT || type == TransactionType::TRANSFER_IN);
    std::string amtColor = isCredit ? cGrn : cRed;

    oss << cBrd << "+-----------------------------------------------------------------------+\n" << cRst;
    oss << cBrd << "| " << cGld << std::left << std::setw(69) << ("                " + bankName) << cBrd << " |\n" << cRst;
    oss << cBrd << "| " << cWht << std::left << std::setw(69) << "                     OFFICIAL TRANSACTION RECEIPT" << cBrd << " |\n" << cRst;
    oss << cBrd << "+-----------------------------------------------------------------------+\n" << cRst;
    oss << "  Receipt Timestamp : " << timestamp << "\n";
    oss << "  Transaction ID    : " << cPur << transactionId << cRst << "\n";
    oss << "  Status            : " << cGrn << "[ COMPLETED / AUTHORIZED ]" << cRst << "\n";
    oss << cBrd << "  ---------------------------------------------------------------------\n" << cRst;
    oss << "  Account Holder    : " << customerName << "\n";
    oss << "  Account Number    : " << accountNumber << "\n";
    oss << "  Transaction Type  : " << (isCredit ? cGrn : cRed) << getTypeString() << cRst << "\n";
    if (!relatedAccount.empty()) {
        oss << "  Counterparty Acc  : " << relatedAccount << "\n";
    }
    oss << "  Description       : " << remarks << "\n";
    oss << cBrd << "  ---------------------------------------------------------------------\n" << cRst;
    oss << "  TRANSACTION AMOUNT: " << amtColor << Utils::formatCurrency(amount) << cRst << "\n";
    oss << "  CLOSING BALANCE   : " << cWht << Utils::formatCurrency(balanceAfter) << cRst << "\n";
    oss << cBrd << "  ---------------------------------------------------------------------\n" << cRst;
    oss << "  Verification Seal : " << cPur << "SEC-AUTH-2026-VAL-OK-9921" << cRst << "\n";
    oss << "  Support Contact   : support@raybank.io | 1-800-RAY-BANK\n";
    oss << cBrd << "+-----------------------------------------------------------------------+\n" << cRst;
    return oss.str();
}
