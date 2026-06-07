# Service-Queue-Ticketing-System
A C++ console-based Service Queue Ticketing System implementing FCFS queue management, dynamic memory allocation, file handling, and ticket history tracking

## Overview

This project simulates a service queue system commonly used in:

- Banks
- Clinics
- Restaurants
- Customer service centers

Customers receive sequential ticket numbers and are served on a First-Come-First-Serve (FCFS) basis.

## Features

- Take a new ticket
- Serve next customer
- View waiting queue
- View served history
- Save ticket records to file
- Load ticket records from file
- Dynamic memory allocation
- Input validation
- File handling

## Technologies Used

- C++
- Dynamic Arrays
- Structures (struct)
- File I/O
- Queue Management
- Console User Interface

## Project Structure

```
service-queue-ticketing-system/
├── src/
├── docs/
├── data/
├── screenshots/
└── README.md
```

## How to Compile

### Using g++

```bash
g++ src/service_queue_ticketing_system.cpp -o ticket_system
```

### Run

```bash
./ticket_system
```

## Example Menu

```
1) Take a ticket
2) Serve next customer
3) Show waiting queue
4) Show served history
5) Save to file
6) Load from file
0) Exit
```

## Future Improvements

- Replace manual arrays with STL vectors
- Use STL queue container
- Add timestamps
- Add multiple service counters
- Add ticket priorities
- GUI version using Qt

## Authors

Ashutosh Ballan

## Academic Project

Final Programming Project – Service Queue Ticketing System
