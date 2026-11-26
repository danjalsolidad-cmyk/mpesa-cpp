// mpesa-cpp: a small command-line tool that reads M-Pesa style transactions
// from a CSV file, groups spending by category and flags unusual payments.
//
// Build:  g++ -std=c++17 -O2 -Wall -o mpesa main.cpp
// Run:    ./mpesa sample_transactions.csv

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

struct Transaction {
    std::string date;
    std::string description;
    double amount = 0.0;  // in KES
    bool moneyOut = true; // true = spent, false = received
};

// Keyword -> category rules. The first keyword found in the description wins.
static const std::vector<std::pair<std::string, std::string>> kRules = {
    {"safaricom", "Airtime & Data"}, {"airtime", "Airtime & Data"},
    {"kplc", "Utilities"},           {"water", "Utilities"},
    {"naivas", "Groceries"},         {"quickmart", "Groceries"},
    {"uber", "Transport"},           {"bolt", "Transport"},
    {"matatu", "Transport"},         {"netflix", "Entertainment"},
    {"rent", "Rent"},                {"fuliza", "Loan Repayment"},
};

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::vector<std::string> split(const std::string& line, char delim) {
    std::vector<std::string> parts;
    std::string item;
    std::stringstream ss(line);
    while (std::getline(ss, item, delim)) {
        parts.push_back(item);
    }
    return parts;
}

std::string categorize(const std::string& description) {
    const std::string text = toLower(description);
    for (const auto& rule : kRules) {
        if (text.find(rule.first) != std::string::npos) {
            return rule.second;
        }
    }
    return "Other";
}

// Expected columns: date,description,amount,type   (type = in | out)
bool loadTransactions(const std::string& path, std::vector<Transaction>& out) {
    std::ifstream file(path);
    if (!file) {
        return false;
    }
    std::string line;
    std::getline(file, line); // skip header row
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        const auto cols = split(line, ',');
        if (cols.size() < 4) {
            std::cerr << "Skipping malformed line: " << line << "\n";
            continue;
        }
        try {
            Transaction t;
            t.date = cols[0];
            t.description = cols[1];
            t.amount = std::stod(cols[2]);
            t.moneyOut = (toLower(cols[3]) == "out");
            out.push_back(t);
        } catch (const std::exception&) {
            std::cerr << "Skipping line with bad amount: " << line << "\n";
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <transactions.csv>\n";
        return 1;
    }

    std::vector<Transaction> transactions;
    if (!loadTransactions(argv[1], transactions)) {
        std::cerr << "Could not open file: " << argv[1] << "\n";
        return 1;
    }

    double totalIn = 0.0;
    double totalOut = 0.0;
    int countOut = 0;
    std::map<std::string, double> byCategory;

    for (const auto& t : transactions) {
        if (t.moneyOut) {
            totalOut += t.amount;
            ++countOut;
            byCategory[categorize(t.description)] += t.amount;
        } else {
            totalIn += t.amount;
        }
    }

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Transactions read: " << transactions.size() << "\n";
    std::cout << "Money in:  KES " << totalIn << "\n";
    std::cout << "Money out: KES " << totalOut << "\n\n";

    // Sort categories by amount spent, highest first.
    std::vector<std::pair<std::string, double>> sorted(byCategory.begin(),
                                                       byCategory.end());
    std::sort(sorted.begin(), sorted.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    std::cout << "Spending by category\n";
    for (const auto& entry : sorted) {
        const double share = totalOut > 0 ? entry.second / totalOut * 100.0 : 0.0;
        std::cout << "  " << std::left << std::setw(16) << entry.first
                  << std::right << std::setw(12) << entry.second << "  ("
                  << std::setprecision(1) << share << "%)"
                  << std::setprecision(2) << "\n";
    }

    // Flag payments larger than twice the average payment.
    if (countOut > 0) {
        const double threshold = 2.0 * (totalOut / countOut);
        std::cout << "\nUnusual payments (over KES " << threshold << ")\n";
        bool any = false;
        for (const auto& t : transactions) {
            if (t.moneyOut && t.amount > threshold) {
                std::cout << "  " << t.date << "  " << t.description << "  KES "
                          << t.amount << "\n";
                any = true;
            }
        }
        if (!any) {
            std::cout << "  None found\n";
        }
    }
    return 0;
}
