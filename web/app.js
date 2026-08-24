// Apex Global Bank - Modern Core Banking Engine & Web Controller

class BankingSystem {
    constructor() {
        this.bankName = "Apex Global Bank";
        this.customers = {};
        this.accounts = {};
        this.nextCustId = 1001;
        this.nextAccNum = 1001;
        this.nextTxnId = 10001;
        this.activeCustomerId = null;
        this.activeAccountNumber = null;

        this.loadState();
        if (Object.keys(this.customers).length === 0) {
            this.seedInitialData();
        }
    }

    saveState() {
        const state = {
            customers: this.customers,
            accounts: this.accounts,
            nextCustId: this.nextCustId,
            nextAccNum: this.nextAccNum,
            nextTxnId: this.nextTxnId
        };
        localStorage.setItem("apex_bank_state", JSON.stringify(state));
    }

    loadState() {
        const saved = localStorage.getItem("apex_bank_state");
        if (saved) {
            try {
                const parsed = JSON.parse(saved);
                this.customers = parsed.customers || {};
                this.accounts = parsed.accounts || {};
                this.nextCustId = parsed.nextCustId || 1001;
                this.nextAccNum = parsed.nextAccNum || 1001;
                this.nextTxnId = parsed.nextTxnId || 10001;
            } catch (e) {
                console.error("Failed to load saved bank state", e);
            }
        }
    }

    getTimestamp() {
        const now = new Date();
        const pad = (n) => String(n).padStart(2, '0');
        return `${now.getFullYear()}-${pad(now.getMonth() + 1)}-${pad(now.getDate())} ${pad(now.getHours())}:${pad(now.getMinutes())}:${pad(now.getSeconds())}`;
    }

    formatCurrency(amount) {
        return new Intl.NumberFormat('en-US', {
            style: 'currency',
            currency: 'USD'
        }).format(amount);
    }

    createCustomer(name, email, phone, address) {
        const id = `CUST-${this.nextCustId++}`;
        this.customers[id] = {
            id,
            name,
            email,
            phone,
            address,
            accountNumbers: []
        };
        this.saveState();
        return id;
    }

    createAccount(customerId, type, initialDeposit = 0, pin = "1234") {
        if (!this.customers[customerId]) return null;
        const accNum = `ACC-${this.nextAccNum++}`;
        const account = {
            accountNumber: accNum,
            customerId,
            type, // "SAVINGS" | "CHECKING"
            balance: 0,
            pin: (pin && pin.length === 4) ? pin : "1234",
            transactions: []
        };

        if (initialDeposit > 0) {
            account.balance = initialDeposit;
            const txnId = `TXN-${this.nextTxnId++}`;
            account.transactions.push({
                transactionId: txnId,
                timestamp: this.getTimestamp(),
                type: "DEPOSIT",
                amount: initialDeposit,
                balanceAfter: account.balance,
                relatedAccount: "",
                remarks: "Initial Account Opening Deposit"
            });
        }

        this.accounts[accNum] = account;
        this.customers[customerId].accountNumbers.push(accNum);
        this.saveState();
        return accNum;
    }

    deposit(accountNumber, amount, remarks = "Cash Deposit") {
        const acc = this.accounts[accountNumber];
        if (!acc || amount <= 0) return false;

        acc.balance += amount;
        const txnId = `TXN-${this.nextTxnId++}`;
        const txn = {
            transactionId: txnId,
            timestamp: this.getTimestamp(),
            type: "DEPOSIT",
            amount,
            balanceAfter: acc.balance,
            relatedAccount: "",
            remarks: remarks || "Cash Deposit"
        };
        acc.transactions.push(txn);
        this.saveState();
        return txn;
    }

    withdraw(accountNumber, amount, pin, remarks = "Cash Withdrawal") {
        const acc = this.accounts[accountNumber];
        if (!acc || amount <= 0 || acc.pin !== pin || acc.balance < amount) return false;

        acc.balance -= amount;
        const txnId = `TXN-${this.nextTxnId++}`;
        const txn = {
            transactionId: txnId,
            timestamp: this.getTimestamp(),
            type: "WITHDRAWAL",
            amount,
            balanceAfter: acc.balance,
            relatedAccount: "",
            remarks: remarks || "Cash Withdrawal"
        };
        acc.transactions.push(txn);
        this.saveState();
        return txn;
    }

    transferFunds(fromAccNum, toAccNum, amount, pin, remarks = "Fund Transfer") {
        if (fromAccNum === toAccNum || amount <= 0) return false;
        const fromAcc = this.accounts[fromAccNum];
        const toAcc = this.accounts[toAccNum];

        if (!fromAcc || !toAcc || fromAcc.pin !== pin || fromAcc.balance < amount) return false;

        const timestamp = this.getTimestamp();
        const outTxnId = `TXN-${this.nextTxnId++}`;
        const inTxnId = `TXN-${this.nextTxnId++}`;
        const note = remarks || "Fund Transfer";

        fromAcc.balance -= amount;
        fromAcc.transactions.push({
            transactionId: outTxnId,
            timestamp,
            type: "TRANSFER_OUT",
            amount,
            balanceAfter: fromAcc.balance,
            relatedAccount: toAccNum,
            remarks: note
        });

        toAcc.balance += amount;
        toAcc.transactions.push({
            transactionId: inTxnId,
            timestamp,
            type: "TRANSFER_IN",
            amount,
            balanceAfter: toAcc.balance,
            relatedAccount: fromAccNum,
            remarks: note
        });

        this.saveState();
        return fromAcc.transactions[fromAcc.transactions.length - 1];
    }

    changePin(accountNumber, oldPin, newPin) {
        const acc = this.accounts[accountNumber];
        if (!acc || acc.pin !== oldPin || !newPin || newPin.length !== 4) return false;
        acc.pin = newPin;
        this.saveState();
        return true;
    }

    findTransaction(txnId) {
        for (const accNum in this.accounts) {
            const acc = this.accounts[accNum];
            for (const txn of acc.transactions) {
                if (txn.transactionId === txnId) {
                    const cust = this.customers[acc.customerId];
                    return {
                        txn,
                        customerName: cust ? cust.name : "Customer",
                        accountNumber: accNum
                    };
                }
            }
        }
        return null;
    }

    seedInitialData() {
        this.customers = {};
        this.accounts = {};
        this.nextCustId = 1001;
        this.nextAccNum = 1001;
        this.nextTxnId = 10001;

        // Customer 1: Alice Johnson
        const c1 = this.createCustomer("Alice Johnson", "alice@example.com", "+1-555-0101", "124 Market St, Suite 400");
        const a1 = this.createAccount(c1, "SAVINGS", 5000.00, "1234");
        const a2 = this.createAccount(c1, "CHECKING", 1200.00, "1234");

        // Customer 2: Bob Smith
        const c2 = this.createCustomer("Bob Smith", "bob.smith@example.com", "+1-555-0202", "742 Evergreen Terrace");
        const b1 = this.createAccount(c2, "SAVINGS", 8500.00, "4321");

        // Customer 3: Clara Oswald
        const c3 = this.createCustomer("Clara Oswald", "clara.o@example.com", "+1-555-0303", "221B Baker Street");
        const c_acc = this.createAccount(c3, "CHECKING", 3500.00, "9999");

        // Sample Transactions
        this.deposit(a1, 1500.00, "Salary Deposit");
        this.withdraw(a1, 300.00, "1234", "ATM Cash Withdrawal");
        this.transferFunds(a1, b1, 750.00, "1234", "Monthly Shared Rent");
        this.transferFunds(b1, c_acc, 250.00, "4321", "Freelance Design Payment");
        this.deposit(c_acc, 1000.00, "Dividend Credit");

        this.saveState();
    }
}

// UI Controller
document.addEventListener("DOMContentLoaded", () => {
    const bank = new BankingSystem();

    // DOM Elements
    const custSelect = document.getElementById("customer-select");
    const custName = document.getElementById("cust-name");
    const custIdBadge = document.getElementById("cust-id-badge");
    const custAvatar = document.getElementById("cust-avatar");
    const custEmail = document.getElementById("cust-email");
    const custPhone = document.getElementById("cust-phone");
    const custAddress = document.getElementById("cust-address");
    const custAccountCount = document.getElementById("cust-account-count");
    const accountCardsList = document.getElementById("account-cards-list");

    const statVault = document.getElementById("stat-total-vault");
    const statAccounts = document.getElementById("stat-total-accounts");
    const statTxns = document.getElementById("stat-total-txns");

    const activeAccTypeBadge = document.getElementById("active-acc-type-badge");
    const activeAccNumDisplay = document.getElementById("active-acc-num-display");
    const activeAccBalance = document.getElementById("active-acc-balance");
    const activeAccTxnCount = document.getElementById("active-acc-txn-count");

    const txTableBody = document.getElementById("transaction-table-body");
    const txSearchInput = document.getElementById("tx-search-input");
    let currentFilter = "ALL";

    function showToast(msg, type = "info") {
        const container = document.getElementById("toast-container");
        const toast = document.createElement("div");
        toast.className = `toast ${type}`;
        const icons = { success: "✓", error: "✕", info: "ℹ" };
        toast.innerHTML = `<span>${icons[type] || "•"}</span> <span>${msg}</span>`;
        container.appendChild(toast);
        setTimeout(() => {
            toast.style.opacity = "0";
            setTimeout(() => toast.remove(), 300);
        }, 3500);
    }

    function updateTopStats() {
        let totalVault = 0;
        let totalTxns = 0;
        const allAccs = Object.values(bank.accounts);
        allAccs.forEach(acc => {
            totalVault += acc.balance;
            totalTxns += acc.transactions.length;
        });

        statVault.textContent = bank.formatCurrency(totalVault);
        statAccounts.textContent = allAccs.length;
        statTxns.textContent = totalTxns;
    }

    function populateCustomerDropdown() {
        custSelect.innerHTML = "";
        const custs = Object.values(bank.customers);
        custs.forEach(c => {
            const opt = document.createElement("option");
            opt.value = c.id;
            opt.textContent = `${c.name} (${c.id})`;
            custSelect.appendChild(opt);
        });

        if (custs.length > 0) {
            if (!bank.activeCustomerId || !bank.customers[bank.activeCustomerId]) {
                bank.activeCustomerId = custs[0].id;
            }
            custSelect.value = bank.activeCustomerId;
            renderCustomerPortfolio(bank.activeCustomerId);
        }
    }

    function renderCustomerPortfolio(custId) {
        const cust = bank.customers[custId];
        if (!cust) return;

        custName.textContent = cust.name;
        custIdBadge.textContent = cust.id;
        custAvatar.textContent = cust.name.split(" ").map(w => w[0]).join("").substring(0, 2).toUpperCase();
        custEmail.textContent = cust.email;
        custPhone.textContent = cust.phone;
        custAddress.textContent = cust.address;
        custAccountCount.textContent = cust.accountNumbers.length;

        // Render account cards
        accountCardsList.innerHTML = "";
        if (cust.accountNumbers.length === 0) {
            accountCardsList.innerHTML = `<div class="empty-state">No accounts opened yet.</div>`;
            bank.activeAccountNumber = null;
            renderActiveAccountSummary();
            return;
        }

        if (!bank.activeAccountNumber || !cust.accountNumbers.includes(bank.activeAccountNumber)) {
            bank.activeAccountNumber = cust.accountNumbers[0];
        }

        cust.accountNumbers.forEach(accNum => {
            const acc = bank.accounts[accNum];
            if (!acc) return;
            const card = document.createElement("div");
            card.className = `acc-card ${acc.type.toLowerCase()} ${accNum === bank.activeAccountNumber ? 'active' : ''}`;
            card.innerHTML = `
                <div class="acc-card-top">
                    <span class="acc-type-badge">${acc.type}</span>
                    <span class="acc-num">${acc.accountNumber}</span>
                </div>
                <div class="acc-card-bottom">
                    <span class="acc-bal">${bank.formatCurrency(acc.balance)}</span>
                    <span class="acc-sec-badge">🔒 PIN Protected</span>
                </div>
            `;
            card.addEventListener("click", () => {
                bank.activeAccountNumber = accNum;
                renderCustomerPortfolio(custId);
            });
            accountCardsList.appendChild(card);
        });

        renderActiveAccountSummary();
    }

    function renderActiveAccountSummary() {
        const acc = bank.accounts[bank.activeAccountNumber];
        if (!acc) {
            activeAccTypeBadge.textContent = "NO ACCOUNT SELECTED";
            activeAccNumDisplay.textContent = "---";
            activeAccBalance.textContent = "$0.00";
            activeAccTxnCount.textContent = "0 Transactions";
            txTableBody.innerHTML = `<tr><td colspan="8" class="empty-state">Please select an account.</td></tr>`;
            return;
        }

        activeAccTypeBadge.textContent = `${acc.type} ACCOUNT`;
        activeAccNumDisplay.textContent = acc.accountNumber;
        activeAccBalance.textContent = bank.formatCurrency(acc.balance);
        activeAccTxnCount.textContent = `${acc.transactions.length} Transactions Logged`;

        renderTransactions();
        updateTopStats();
    }

    function renderTransactions() {
        const acc = bank.accounts[bank.activeAccountNumber];
        if (!acc || acc.transactions.length === 0) {
            txTableBody.innerHTML = `<tr><td colspan="8" class="empty-state">No transactions recorded for this account yet.</td></tr>`;
            return;
        }

        const query = (txSearchInput.value || "").toLowerCase();
        let list = [...acc.transactions].reverse();

        if (currentFilter !== "ALL") {
            if (currentFilter === "TRANSFER") {
                list = list.filter(t => t.type.startsWith("TRANSFER"));
            } else {
                list = list.filter(t => t.type === currentFilter);
            }
        }

        if (query) {
            list = list.filter(t => 
                t.transactionId.toLowerCase().includes(query) ||
                t.remarks.toLowerCase().includes(query) ||
                t.relatedAccount.toLowerCase().includes(query)
            );
        }

        if (list.length === 0) {
            txTableBody.innerHTML = `<tr><td colspan="8" class="empty-state">No matching transactions found.</td></tr>`;
            return;
        }

        txTableBody.innerHTML = "";
        list.forEach(t => {
            const isCredit = (t.type === "DEPOSIT" || t.type === "TRANSFER_IN");
            const badgeClass = isCredit ? (t.type === "DEPOSIT" ? "badge-emerald" : "badge-cyan") : (t.type === "WITHDRAWAL" ? "badge-coral" : "badge-amber");
            const amtClass = isCredit ? "credit" : "debit";
            const amtSign = isCredit ? "+" : "-";

            const tr = document.createElement("tr");
            tr.innerHTML = `
                <td class="tx-id">${t.transactionId}</td>
                <td class="tx-date">${t.timestamp}</td>
                <td><span class="badge ${badgeClass}">${t.type}</span></td>
                <td class="tx-amt ${amtClass}">${amtSign}${bank.formatCurrency(t.amount)}</td>
                <td class="tx-balance">${bank.formatCurrency(t.balanceAfter)}</td>
                <td>${t.relatedAccount ? `<span class="badge badge-purple">${t.relatedAccount}</span>` : '<span class="text-muted">-</span>'}</td>
                <td>${t.remarks}</td>
                <td><button class="btn btn-sm btn-outline-cyan btn-view-rcpt" data-txnid="${t.transactionId}">Receipt</button></td>
            `;
            txTableBody.appendChild(tr);
        });

        // Bind receipt buttons
        document.querySelectorAll(".btn-view-rcpt").forEach(btn => {
            btn.addEventListener("click", () => {
                openReceiptModal(btn.getAttribute("data-txnid"));
            });
        });
    }

    // Modal Helpers
    function openModal(id) {
        document.getElementById(id).classList.add("active");
    }
    function closeModal(id) {
        document.getElementById(id).classList.remove("active");
    }

    document.querySelectorAll("[data-close]").forEach(btn => {
        btn.addEventListener("click", () => {
            closeModal(btn.getAttribute("data-close"));
        });
    });

    // Populate Account Select Options in Modals
    function populateAccountSelects() {
        const selects = [
            "deposit-account-select",
            "withdraw-account-select",
            "transfer-from-select",
            "transfer-to-select",
            "pin-account-select"
        ];
        selects.forEach(id => {
            const sel = document.getElementById(id);
            if (!sel) return;
            sel.innerHTML = "";
            Object.values(bank.accounts).forEach(a => {
                const opt = document.createElement("option");
                opt.value = a.accountNumber;
                const cust = bank.customers[a.customerId];
                opt.textContent = `${a.accountNumber} - ${cust ? cust.name : 'Unknown'} (${a.type}: ${bank.formatCurrency(a.balance)})`;
                sel.appendChild(opt);
            });
            if (bank.activeAccountNumber) {
                sel.value = bank.activeAccountNumber;
            }
        });

        // Customer Select for New Account modal
        const custAccSel = document.getElementById("acc-input-customer");
        if (custAccSel) {
            custAccSel.innerHTML = "";
            Object.values(bank.customers).forEach(c => {
                const opt = document.createElement("option");
                opt.value = c.id;
                opt.textContent = `${c.name} (${c.id})`;
                custAccSel.appendChild(opt);
            });
            if (bank.activeCustomerId) {
                custAccSel.value = bank.activeCustomerId;
            }
        }
    }

    // Receipt Modal Logic
    let currentReceiptData = null;
    function openReceiptModal(txnId) {
        const data = bank.findTransaction(txnId);
        if (!data) {
            showToast("Transaction receipt not found", "error");
            return;
        }
        currentReceiptData = data;
        const { txn, customerName, accountNumber } = data;

        document.getElementById("rcpt-timestamp").textContent = txn.timestamp;
        document.getElementById("rcpt-txnid").textContent = txn.transactionId;
        document.getElementById("rcpt-custname").textContent = customerName;
        document.getElementById("rcpt-accnum").textContent = accountNumber;
        document.getElementById("rcpt-type").textContent = txn.type;
        
        const cpRow = document.getElementById("rcpt-counterparty-row");
        if (txn.relatedAccount) {
            cpRow.style.display = "flex";
            document.getElementById("rcpt-counterparty").textContent = txn.relatedAccount;
        } else {
            cpRow.style.display = "none";
        }

        document.getElementById("rcpt-remarks").textContent = txn.remarks;
        document.getElementById("rcpt-amount").textContent = bank.formatCurrency(txn.amount);
        document.getElementById("rcpt-balance").textContent = bank.formatCurrency(txn.balanceAfter);

        openModal("modal-receipt");
    }

    document.getElementById("btn-print-receipt").addEventListener("click", () => {
        window.print();
    });

    document.getElementById("btn-download-receipt").addEventListener("click", () => {
        if (!currentReceiptData) return;
        const { txn, customerName, accountNumber } = currentReceiptData;
        const text = `
+-----------------------------------------------------------------------+
|                APEX GLOBAL BANK                                       |
|                     OFFICIAL TRANSACTION RECEIPT                      |
+-----------------------------------------------------------------------+
  Receipt Timestamp : ${txn.timestamp}
  Transaction ID    : ${txn.transactionId}
  Status            : [ COMPLETED / AUTHORIZED ]
  ---------------------------------------------------------------------
  Account Holder    : ${customerName}
  Account Number    : ${accountNumber}
  Transaction Type  : ${txn.type}
  Counterparty Acc  : ${txn.relatedAccount || '-'}
  Description       : ${txn.remarks}
  ---------------------------------------------------------------------
  TRANSACTION AMOUNT: ${bank.formatCurrency(txn.amount)}
  CLOSING BALANCE   : ${bank.formatCurrency(txn.balanceAfter)}
  ---------------------------------------------------------------------
  Verification Seal : SEC-AUTH-2026-VAL-OK-9921
  Support Contact   : support@apexbank.io | 1-800-APEX-BANK
+-----------------------------------------------------------------------+
`;
        const blob = new Blob([text], { type: "text/plain" });
        const url = URL.createObjectURL(blob);
        const a = document.createElement("a");
        a.href = url;
        a.download = `receipt_${txn.transactionId}.txt`;
        a.click();
        URL.revokeObjectURL(url);
        showToast("Receipt file downloaded successfully", "success");
    });

    // Event Listeners
    custSelect.addEventListener("change", (e) => {
        bank.activeCustomerId = e.target.value;
        renderCustomerPortfolio(bank.activeCustomerId);
    });

    // Filter pills
    document.querySelectorAll(".pill").forEach(p => {
        p.addEventListener("click", () => {
            document.querySelectorAll(".pill").forEach(x => x.classList.remove("active"));
            p.classList.add("active");
            currentFilter = p.getAttribute("data-filter");
            renderTransactions();
        });
    });

    txSearchInput.addEventListener("input", renderTransactions);

    // Button Triggers
    document.getElementById("btn-seed-data").addEventListener("click", () => {
        bank.seedInitialData();
        populateCustomerDropdown();
        showToast("Demo Data Seeded (Alice, Bob, Clara)", "success");
    });

    document.getElementById("btn-open-new-customer").addEventListener("click", () => openModal("modal-customer"));
    document.getElementById("btn-add-customer-quick").addEventListener("click", () => openModal("modal-customer"));

    document.getElementById("btn-open-account-modal").addEventListener("click", () => {
        populateAccountSelects();
        openModal("modal-account");
    });

    document.getElementById("btn-action-deposit").addEventListener("click", () => {
        populateAccountSelects();
        openModal("modal-deposit");
    });

    document.getElementById("btn-action-withdraw").addEventListener("click", () => {
        populateAccountSelects();
        openModal("modal-withdraw");
    });

    document.getElementById("btn-action-transfer").addEventListener("click", () => {
        populateAccountSelects();
        openModal("modal-transfer");
    });

    document.getElementById("btn-action-pin").addEventListener("click", () => {
        populateAccountSelects();
        openModal("modal-change-pin");
    });

    document.getElementById("btn-action-receipt").addEventListener("click", () => {
        const id = prompt("Enter Transaction ID (e.g. TXN-10001):");
        if (id) openReceiptModal(id.trim());
    });

    // Form Submissions
    document.getElementById("form-create-customer").addEventListener("submit", (e) => {
        e.preventDefault();
        const name = document.getElementById("cust-input-name").value;
        const email = document.getElementById("cust-input-email").value;
        const phone = document.getElementById("cust-input-phone").value;
        const address = document.getElementById("cust-input-address").value;

        const id = bank.createCustomer(name, email, phone, address);
        closeModal("modal-customer");
        e.target.reset();
        populateCustomerDropdown();
        bank.activeCustomerId = id;
        custSelect.value = id;
        renderCustomerPortfolio(id);
        showToast(`Customer ${name} registered (${id})`, "success");
    });

    document.getElementById("form-create-account").addEventListener("submit", (e) => {
        e.preventDefault();
        const custId = document.getElementById("acc-input-customer").value;
        const type = document.querySelector('input[name="account-type-choice"]:checked').value;
        const deposit = parseFloat(document.getElementById("acc-input-deposit").value) || 0;
        const pin = document.getElementById("acc-input-pin").value;

        const accNum = bank.createAccount(custId, type, deposit, pin);
        closeModal("modal-account");
        e.target.reset();
        bank.activeAccountNumber = accNum;
        renderCustomerPortfolio(custId);
        showToast(`Account ${accNum} opened with PIN protection`, "success");
    });

    document.getElementById("form-deposit").addEventListener("submit", (e) => {
        e.preventDefault();
        const accNum = document.getElementById("deposit-account-select").value;
        const amount = parseFloat(document.getElementById("deposit-amount").value);
        const remarks = document.getElementById("deposit-remarks").value;

        const txn = bank.deposit(accNum, amount, remarks);
        if (txn) {
            closeModal("modal-deposit");
            e.target.reset();
            renderCustomerPortfolio(bank.activeCustomerId);
            showToast(`Deposited ${bank.formatCurrency(amount)} into ${accNum}`, "success");
            openReceiptModal(txn.transactionId);
        } else {
            showToast("Deposit failed. Check amount.", "error");
        }
    });

    document.getElementById("form-withdraw").addEventListener("submit", (e) => {
        e.preventDefault();
        const accNum = document.getElementById("withdraw-account-select").value;
        const amount = parseFloat(document.getElementById("withdraw-amount").value);
        const pin = document.getElementById("withdraw-pin").value;
        const remarks = document.getElementById("withdraw-remarks").value;

        const txn = bank.withdraw(accNum, amount, pin, remarks);
        if (txn) {
            closeModal("modal-withdraw");
            e.target.reset();
            renderCustomerPortfolio(bank.activeCustomerId);
            showToast(`Withdrawn ${bank.formatCurrency(amount)} from ${accNum}`, "success");
            openReceiptModal(txn.transactionId);
        } else {
            showToast("Withdrawal failed. Incorrect PIN or insufficient balance.", "error");
        }
    });

    document.getElementById("form-transfer").addEventListener("submit", (e) => {
        e.preventDefault();
        const fromAcc = document.getElementById("transfer-from-select").value;
        const toAcc = document.getElementById("transfer-to-select").value;
        const amount = parseFloat(document.getElementById("transfer-amount").value);
        const pin = document.getElementById("transfer-pin").value;
        const remarks = document.getElementById("transfer-remarks").value;

        if (fromAcc === toAcc) {
            showToast("Source and Destination accounts cannot be identical", "error");
            return;
        }

        const txn = bank.transferFunds(fromAcc, toAcc, amount, pin, remarks);
        if (txn) {
            closeModal("modal-transfer");
            e.target.reset();
            renderCustomerPortfolio(bank.activeCustomerId);
            showToast(`Transferred ${bank.formatCurrency(amount)} to ${toAcc}`, "success");
            openReceiptModal(txn.transactionId);
        } else {
            showToast("Transfer failed. Incorrect PIN or insufficient funds.", "error");
        }
    });

    document.getElementById("form-change-pin").addEventListener("submit", (e) => {
        e.preventDefault();
        const accNum = document.getElementById("pin-account-select").value;
        const oldPin = document.getElementById("pin-old").value;
        const newPin = document.getElementById("pin-new").value;
        const confirmPin = document.getElementById("pin-confirm").value;

        if (newPin !== confirmPin) {
            showToast("New PIN confirmation does not match", "error");
            return;
        }

        if (bank.changePin(accNum, oldPin, newPin)) {
            closeModal("modal-change-pin");
            e.target.reset();
            showToast(`Security PIN for ${accNum} updated successfully`, "success");
        } else {
            showToast("Failed to change PIN. Incorrect current PIN.", "error");
        }
    });

    // Initial Load
    populateCustomerDropdown();
    updateTopStats();
});
