# 🏦 Banking System

A full-featured **console-based Banking System** built in **C++17** with a companion **web frontend**. Supports customer registration, multiple account types, PIN-protected transactions, fund transfers, and receipt generation.

---

## ✨ Features

- 👤 **Customer Management** — Register customers with name, email, phone, and address
- 🏧 **Account Types** — Savings & Checking accounts with PIN protection
- 💰 **Deposits & Withdrawals** — With custom remarks and optional receipts
- 🔁 **Fund Transfers** — Between accounts with PIN verification
- 📄 **Transaction History** — Full history or mini-statement (last N transactions)
- 🧾 **Receipt System** — View, reprint, or export receipts to `.txt` files
- 🔐 **PIN Management** — Set and change 4-digit security PINs
- 🌐 **Web Frontend** — Companion UI in `web/` (HTML + CSS + JS)
- 🧪 **Automated Tests** — Test suite powered by CMake/CTest

---

## 📁 Project Structure

```
Banking system/
├── CMakeLists.txt          # CMake build configuration
├── src/                    # C++ source files
│   ├── main.cpp            # Application entry point & menu
│   ├── Bank.cpp            # Core bank logic
│   ├── Account.cpp         # Account operations
│   ├── Customer.cpp        # Customer model
│   ├── Transaction.cpp     # Transaction model
│   └── Utils.cpp           # Terminal UI helpers
├── include/                # Header files
│   ├── Bank.hpp
│   ├── Account.hpp
│   ├── Customer.hpp
│   ├── Transaction.hpp
│   └── Utils.hpp
├── tests/                  # Automated test suite
│   └── test_banking.cpp
└── web/                    # Web frontend
    ├── index.html
    ├── style.css
    └── app.js
```

---

## 🛠️ Build Instructions

### Prerequisites
- CMake >= 3.15
- A C++17 compatible compiler (GCC, Clang, or MSVC)

### Steps

```bash
# 1. Clone the repository
git clone https://github.com/your-username/banking-system.git
cd banking-system

# 2. Create build directory
mkdir build
cd build

# 3. Configure with CMake
cmake ..

# 4. Build
cmake --build .
```

---

## 🚀 Run the Application

```bash
# From the build/ directory:
./banking_app        # Linux/macOS
banking_app.exe      # Windows
```

---

## 🧪 Run Tests

```bash
# From the build/ directory:
ctest --verbose
```

---

## 🌐 Web Frontend

Open `web/index.html` directly in your browser — no server required.

---

## 📋 Menu Options

| Option | Action |
|--------|--------|
| 1 | Register New Customer |
| 2 | Open New Bank Account (PIN Protected) |
| 3 | Deposit Funds |
| 4 | Withdraw Funds (Requires PIN) |
| 5 | Transfer Funds (Requires PIN) |
| 6 | View Account Details & Balance |
| 7 | View Transaction Statement / History |
| 8 | Lookup & Export Transaction Receipt |
| 9 | Change Account Security PIN |
| 10 | View Customer Profile & Accounts |
| 11 | List All Registered Customers |
| 12 | List All Active Accounts |
| 13 | Load Demo Sample Data |
| 0 | Exit |

---

## 🔑 Demo Data (Option 13)

| Customer | Default PIN |
|----------|-------------|
| Alice    | 1234        |
| Bob      | 4321        |
| Clara    | 9999        |

---

## 🧰 Tech Stack

- **Language:** C++17
- **Build System:** CMake
- **Frontend:** HTML5, CSS3, Vanilla JavaScript

---

## 📜 License

This project is open source. Feel free to use and modify it.
