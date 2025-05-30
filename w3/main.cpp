#include <string>
#include <ios>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include "classes.h"

#include <vector>

using namespace std;

int main(int argc, char* argv[]) {
    // Initialize the Storage Manager Class with the Binary .dat file name we want to create
    StorageManager manager("EmployeeRelation.dat");

    // Assuming the Employee.CSV file is in the same directory, 
    // we want to read from the Employee.csv and write into the new data_file
    manager.createFromFile("Employee.csv");

    // Searching for Employee IDs Using [manager.findAndPrintEmployee(id)]
    /***TO_DO***/ 

    string input;
    vector<int> numbers;

    while (true) {
        cout << "Enter an Employee ID ('0' to stop): ";
        getline(cin, input);

        if (input == "0") {
            break;
        }

        int id = stoi(input);
        numbers.push_back(id);

        manager.findAndPrintEmployee(id);
        cout << endl;
    }

    return 0;
}