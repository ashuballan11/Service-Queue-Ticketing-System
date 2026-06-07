// Service Queue Ticketing System
// This program manages a queue-based ticketing system for service counters
// Features: Take tickets, serve customers, view queue/history, and save/load data to/from files

#include <iostream>
#include <queue>
#include <string>
#include <fstream>
using namespace std;

// Struct to represent a ticket in the system
struct Ticket {
    int ticketNumber;           // Unique identifier for each ticket
    int ticketState;            // 0 = waiting, 1 = served
    string customerName;        // Name of the customer
    string serviceReason;       // Reason for service (optional, for file compatibility)
};

// Function to remove pipe characters from text (used as file format delimiter)
string sanitizeText(const string& text) {
    string cleanedText = text;
    int position = 0;
    // Replace all pipe characters with spaces to prevent file format corruption
    while (position < (int)cleanedText.size()) {
        if (cleanedText[position] == '|') cleanedText[position] = ' ';
        position++;
    }
    return cleanedText;
}

// Function to safely read an integer from user input with error handling
int readInteger() {
    int value;
    while (true) {
        cin >> value;
        // Check if input was successfully read
        if (!cin.fail()) {
            cin.ignore(1000, '\n');  // Clear the input buffer
            return value;
        }
        // If input failed, clear error state and prompt user to try again
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "invalid input try again: ";
    }
}

// Function to read a full line of text from user input
string readLineText() {
    string lineText;
    getline(cin, lineText);  // Read entire line including spaces
    return lineText;
}

// Function to display the main menu with all available options
void printMenu() {
    cout << "\n=============================\n";
    cout << " Service Queue Ticket System \n";
    cout << "=============================\n";
    cout << "1) Take a ticket\n";
    cout << "2) Serve next customer\n";
    cout << "3) Show waiting queue\n";
    cout << "4) Show served history\n";
    cout << "5) Save to file\n";
    cout << "6) Load from file\n";
    cout << "0) Exit\n";
    cout << "Choose option (0-6): ";
}

// Function to display a single ticket's information in a formatted way
void printTicket(const Ticket& ticket) {
    cout << "#" << ticket.ticketNumber;
    // Display customer name if available
    if (ticket.customerName.size() > 0) cout << " | " << ticket.customerName;
    // Display ticket status (waiting or served)
    cout << " | " << (ticket.ticketState == 0 ? "waiting" : "served");
    // Display service reason if available
    if (ticket.serviceReason.size() > 0) cout << " | " << ticket.serviceReason;
    cout << endl;
}

// Function to dynamically resize the ticket array when it needs more space
// Uses doubling strategy to minimize reallocations
void ensureCapacity(Ticket*& ticketArray, int& arrayCapacity, int neededCapacity) {
    // If array has enough space, no need to resize
    if (neededCapacity <= arrayCapacity) return;

    // Double the capacity until it's large enough
    int newCapacity = arrayCapacity;
    while (newCapacity < neededCapacity) newCapacity = newCapacity * 2;

    // Create new array with larger capacity
    Ticket* newArray = new Ticket[newCapacity];

    // Copy existing tickets to new array
    int position = 0;
    while (position < arrayCapacity) {
        newArray[position] = ticketArray[position];
        position++;
    }

    // Free old array and update pointer
    delete[] ticketArray;
    ticketArray = newArray;
    arrayCapacity = newCapacity;
}

// Function to add a ticket to the waiting queue
void addWaitingTicket(Ticket*& waitingTickets, int& waitingCount, int& waitingCapacity, const Ticket& ticket) {
    // Ensure array has space for new ticket
    ensureCapacity(waitingTickets, waitingCapacity, waitingCount + 1);
    // Add ticket at the end and increment count
    waitingTickets[waitingCount] = ticket;
    waitingCount++;
}

// Function to remove and return the first ticket from the waiting queue (FIFO)
// Returns 1 if successful, 0 if queue is empty
int serveWaitingTicket(Ticket*& waitingTickets, int& waitingCount, Ticket& servedTicket) {
    // Check if queue is empty
    if (waitingCount == 0) return 0;

    // Get the first ticket
    servedTicket = waitingTickets[0];

    // Shift all remaining tickets forward by one position
    int position = 1;
    while (position < waitingCount) {
        waitingTickets[position - 1] = waitingTickets[position];
        position++;
    }

    // Decrement count and return success
    waitingCount--;
    return 1;
}

// Function to add a ticket to the served history
void addServedTicket(Ticket*& servedTickets, int& servedCount, int& servedCapacity, const Ticket& ticket) {
    // Ensure array has space for new ticket
    ensureCapacity(servedTickets, servedCapacity, servedCount + 1);
    // Add ticket at the end and increment count
    servedTickets[servedCount] = ticket;
    servedCount++;
}

// Function to display all tickets currently in the waiting queue
void showWaitingQueue(Ticket* waitingTickets, int waitingCount) {
    if (waitingCount == 0) {
        cout << "queue empty" << endl;
        return;
    }

    // Print all waiting tickets
    int position = 0;
    while (position < waitingCount) {
        printTicket(waitingTickets[position]);
        position++;
    }
}

// Function to display all tickets that have been served
void showServedHistory(Ticket* servedTickets, int servedCount) {
    if (servedCount == 0) {
        cout << "no served tickets yet" << endl;
        return;
    }

    // Print all served tickets
    int position = 0;
    while (position < servedCount) {
        printTicket(servedTickets[position]);
        position++;
    }
}

// Function to parse a pipe-delimited line into four separate parts
// Used for reading ticket data from file (format: number|state|name|reason)
// Returns 1 if successful, 0 if format is invalid
int splitLineIntoFourParts(const string& line, string& part0, string& part1, string& part2, string& part3) {
    // Find first pipe separator
    int firstSeparator = (int)line.find('|');
    if (firstSeparator < 0) return 0;

    // Find second pipe separator
    int secondSeparator = (int)line.find('|', firstSeparator + 1);
    if (secondSeparator < 0) return 0;

    // Find third pipe separator
    int thirdSeparator = (int)line.find('|', secondSeparator + 1);
    if (thirdSeparator < 0) return 0;

    // Extract the four parts separated by pipes
    part0 = line.substr(0, firstSeparator);
    part1 = line.substr(firstSeparator + 1, secondSeparator - (firstSeparator + 1));
    part2 = line.substr(secondSeparator + 1, thirdSeparator - (secondSeparator + 1));
    part3 = line.substr(thirdSeparator + 1);
    return 1;
}

// Function to convert a string to an integer
// Handles leading whitespace and negative numbers
int parseInteger(const string& text) {
    int position = 0;
    int sign = 1;
    long long value = 0;

    // Skip leading whitespace
    while (position < (int)text.size() && (text[position] == ' ' || text[position] == '\t')) position++;
    
    // Check for negative sign
    if (position < (int)text.size() && text[position] == '-') { sign = -1; position++; }

    // Convert digits to integer
    while (position < (int)text.size() && text[position] >= '0' && text[position] <= '9') {
        value = value * 10 + (text[position] - '0');
        position++;
    }

    return (int)(value * sign);
}

// Function to save all tickets (waiting and served) to a file in pipe-delimited format
// Returns 1 if successful, 0 if file cannot be opened
int saveToFile(const string& fileName,
    Ticket* waitingTickets, int waitingCount,
    Ticket* servedTickets, int servedCount) {

    // Open file for writing
    ofstream fileStream(fileName);
    if (!fileStream.is_open()) return 0;

    // Write all waiting tickets to file
    int waitingPosition = 0;
    while (waitingPosition < waitingCount) {
        Ticket ticket = waitingTickets[waitingPosition];
        // Format: ticketNumber|state|customerName|serviceReason
        fileStream << ticket.ticketNumber << "|" << ticket.ticketState << "|"
            << ticket.customerName << "|" << ticket.serviceReason << "\n";
        waitingPosition++;
    }

    // Write all served tickets to file
    int servedPosition = 0;
    while (servedPosition < servedCount) {
        Ticket ticket = servedTickets[servedPosition];
        fileStream << ticket.ticketNumber << "|" << ticket.ticketState << "|"
            << ticket.customerName << "|" << ticket.serviceReason << "\n";
        servedPosition++;
    }

    fileStream.close();
    return 1;
}

// Function to load all tickets from file and restore the system state
// Separates tickets into waiting and served queues based on their state
// Returns 1 if successful, 0 if file cannot be opened
int loadFromFile(const string& fileName,
    Ticket*& waitingTickets, int& waitingCount, int& waitingCapacity,
    Ticket*& servedTickets, int& servedCount, int& servedCapacity,
    int& nextTicketNumber) {

    // Open file for reading
    ifstream fileStream(fileName);
    if (!fileStream.is_open()) return 0;

    // Reset queues
    waitingCount = 0;
    servedCount = 0;

    int maximumTicketNumber = 0;

    // Read file line by line
    string line;
    while (getline(fileStream, line)) {
        if (line.size() == 0) continue;  // Skip empty lines

        // Parse the pipe-delimited line
        string part0, part1, part2, part3;
        if (!splitLineIntoFourParts(line, part0, part1, part2, part3)) continue;

        // Convert parsed strings to Ticket struct
        Ticket ticket;
        ticket.ticketNumber = parseInteger(part0);
        ticket.ticketState = parseInteger(part1);
        ticket.customerName = part2;
        ticket.serviceReason = part3;

        // Track the maximum ticket number to continue numbering correctly
        if (ticket.ticketNumber > maximumTicketNumber) maximumTicketNumber = ticket.ticketNumber;

        // Add ticket to appropriate queue (waiting or served)
        if (ticket.ticketState == 0) {
            addWaitingTicket(waitingTickets, waitingCount, waitingCapacity, ticket);
        }
        else {
            addServedTicket(servedTickets, servedCount, servedCapacity, ticket);
        }
    }

    fileStream.close();

    // Set next ticket number to be greater than any existing ticket number
    nextTicketNumber = maximumTicketNumber + 1;
    if (nextTicketNumber < 1) nextTicketNumber = 1;

    return 1;
}

// Main function - Entry point of the program
int main() {
    // File name for saving/loading data
    string fileName = "data.txt";

    // Initial capacity for dynamic arrays
    int waitingCapacity = 20;
    int servedCapacity = 20;

    // Allocate memory for ticket queues
    Ticket* waitingTickets = new Ticket[waitingCapacity];
    Ticket* servedTickets = new Ticket[servedCapacity];

    // Track number of tickets in each queue
    int waitingCount = 0;
    int servedCount = 0;

    // Counter for generating unique ticket numbers
    int nextTicketNumber = 1;

    // Main program loop - continues until user selects exit (option 0)
    while (true) {
        printMenu();
        int option = readInteger();

        // Exit the program
        if (option == 0) break;

        // Option 1: Take a new ticket
        if (option == 1) {
            cout << "Enter customer name (press Enter to skip): ";
            string customerName = sanitizeText(readLineText());

            // Create new ticket with unique number
            Ticket newTicket;
            newTicket.ticketNumber = nextTicketNumber;
            newTicket.ticketState = 0;  // 0 = waiting
            newTicket.customerName = customerName;
            newTicket.serviceReason = "";

            // Increment for next ticket
            nextTicketNumber = nextTicketNumber + 1;

            // Add to waiting queue
            addWaitingTicket(waitingTickets, waitingCount, waitingCapacity, newTicket);

            // Display the ticket number to the customer
            cout << "Your ticket number is #" << newTicket.ticketNumber << endl;
        }
        // Option 2: Serve the next customer in queue
        else if (option == 2) {
            Ticket servedTicket;

            // Try to serve the next ticket from the queue
            if (!serveWaitingTicket(waitingTickets, waitingCount, servedTicket)) {
                cout << "queue empty" << endl;
            }
            else {
                // Mark ticket as served (state 1) and add to history
                servedTicket.ticketState = 1;
                addServedTicket(servedTickets, servedCount, servedCapacity, servedTicket);

                cout << "served: ";
                printTicket(servedTicket);
            }
        }
        // Option 3: Display all customers waiting in queue
        else if (option == 3) {
            cout << "waiting count: " << waitingCount << endl;
            showWaitingQueue(waitingTickets, waitingCount);
        }
        // Option 4: Display history of all served customers
        else if (option == 4) {
            cout << "served history:" << endl;
            showServedHistory(servedTickets, servedCount);
        }
        // Option 5: Save all tickets to file
        else if (option == 5) {
            if (saveToFile(fileName, waitingTickets, waitingCount, servedTickets, servedCount)) {
                cout << "saved to " << fileName << endl;
            }
            else {
                cout << "save failed" << endl;
            }
        }
        // Option 6: Load tickets from file to restore previous session
        else if (option == 6) {
            if (loadFromFile(fileName,
                waitingTickets, waitingCount, waitingCapacity,
                servedTickets, servedCount, servedCapacity,
                nextTicketNumber)) {
                cout << "loaded from " << fileName << endl;
            }
            else {
                cout << "load failed" << endl;
            }
        }
        // Invalid option handling
        else {
            cout << "invalid option" << endl;
        }
    }

    // Free allocated memory before exit
    delete[] waitingTickets;
    delete[] servedTickets;
    
    return 0;
}
