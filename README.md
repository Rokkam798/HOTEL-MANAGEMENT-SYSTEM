# Hotel Management System

## Overview

This project is an implementation of a hotel management system using various OS concepts such as Linux commands, process creation, inter-process communication (IPC), `wait()`, `sleep()`, and more. The system simulates a hotel environment with different entities like Admin, Hotel Manager, Tables, Customers, and Waiters, represented as processes. 

## Structure

1. **table.c**: Manages table processes and customer processes.
2. **waiter.c**: Handles waiter processes assigned to specific tables.
3. **hotelmanager.c**: Oversees total earnings, waiter earnings, and handles termination.
4. **admin.c**: Manages hotel closure initiation.

## Getting Started

### Prerequisites

- Ubuntu 22.04
- POSIX-compliant C compiler

### Files

- `table.c`
- `waiter.c`
- `hotelmanager.c`
- `admin.c`
- `menu.txt`: Pre-created menu file with food items and prices.

### Compilation

```sh
gcc table.c -o table.out
gcc waiter.c -o waiter.out
gcc hotelmanager.c -o hotelmanager.out
gcc admin.c -o admin.out
```

### Execution

#### Table Process

1. Run in a separate terminal:
    ```sh
    ./table.out
    ```
2. Follow prompts to enter table number and number of customers.

#### Waiter Process

1. Run in a separate terminal:
    ```sh
    ./waiter.out
    ```
2. Follow prompt to enter Waiter ID.

#### Hotel Manager Process

1. Run in a separate terminal:
    ```sh
    ./hotelmanager.out
    ```
2. Follow prompt to enter the total number of tables.

#### Admin Process

1. Run in a separate terminal:
    ```sh
    ./admin.out
    ```
2. Follow prompts to manage hotel closure.

## Workflow

1. **Table Process**:
   - Asks for table number and number of customers.
   - Displays menu from `menu.txt`.
   - Creates customer processes for each customer.
   - Collects orders via pipes and communicates with the corresponding waiter via shared memory.

2. **Waiter Process**:
   - Receives orders from table process via shared memory.
   - Validates and calculates total bill.
   - Sends bill to table process and hotel manager.

3. **Hotel Manager Process**:
   - Collects total earnings from waiters.
   - Writes earnings to `earnings.txt`.
   - On termination, calculates and displays total earnings, wages, and profit.

4. **Admin Process**:
   - Monitors for hotel closure.
   - Informs hotel manager to terminate upon receiving user input.

## IPC Mechanisms

- **Pipes**: Used for communication between table and customer processes.
- **Shared Memory**: Used for communication between table and waiter processes, and waiter and hotel manager processes.

## Termination

- Table and waiter processes terminate after no more customers are seated.
- Admin process initiates hotel closure.
- Hotel manager process ensures all processes terminate cleanly and calculates final earnings.
