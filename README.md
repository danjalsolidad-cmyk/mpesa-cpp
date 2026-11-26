# mpesa-cpp

A command-line tool written in C++17 that reads M-Pesa style transactions from a CSV file, groups spending by category and flags unusual payments. It is the C++ companion to my Python project, [mpesa-analyzer](https://github.com/danjalsolidad-cmyk/mpesa-analyzer).

## Features

- Parses a CSV file of transactions (date, description, amount, in/out)
- Categorizes spending using keyword rules (groceries, transport, utilities, and more)
- Prints money in, money out and spending per category with percentages
- Flags payments larger than twice the average payment

## Build and run

```bash
g++ -std=c++17 -O2 -Wall -o mpesa main.cpp
./mpesa sample_transactions.csv
```

## CSV format

```
date,description,amount,type
2026-11-02,Rent Payment,12000,out
2026-11-01,Salary Deposit,45000,in
```

`type` is `in` for money received and `out` for money spent. Amounts are in KES.

## C++ concepts used

- `struct` for the transaction record
- STL containers: `std::vector`, `std::map`, `std::pair`
- `std::sort` with lambda comparators
- File input with `std::ifstream` and string parsing with `std::stringstream`
- Exception handling for bad input
- Formatted output with `<iomanip>`

## Ideas for next steps

- Handle quoted CSV fields
- Read the real M-Pesa statement format
- Export the summary to a file
