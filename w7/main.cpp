
#include <string>
#include <iostream>
#include <stdexcept>
#include "classes.h"

using namespace std;


int main() {
    // Create the index
    HashIndex hashIndex(std::string("EmployeeIndex.dat"));

    std::cout << "loading records please wait." << endl << endl;
    hashIndex.createFromFile("Employee.csv");
    // hashIndex.createFromFile("Employee_large.csv"); // larger file

    // Loop to lookup IDs until user is ready to quit
    std::string searchID;

    std::cout << "Enter the employee ID to find or type exit to terminate: ";
    while (std::cin >> searchID && searchID != "exit") {
        int64_t id = std::stoll(searchID);
        std::string record;
        hashIndex.findAndPrintEmployee(id);

        std::cout << "Enter the employee ID to find or type exit to terminate: ";
    }


// easy tester
    // HashIndex hashIndex(std::string("EmployeeIndex.dat"));

    // std::cout << "loading small records please wait." << endl << endl;
    // hashIndex.createFromFile("Employee.csv");
    // std::vector<int64_t> searchIDs = {11432121, 11432138, 11432150, 11432136, 11432119}; 
    // for (int64_t id : searchIDs) {
    //     hashIndex.findAndPrintEmployee(id);
    //     std::cout << std::endl << std::endl;
    // }

    // std::cout << "loading large records please wait." << endl << endl;
    // hashIndex.createFromFile("Employee_large.csv");
    // searchIDs = {32188894, 91746734, 56730731, 69782613, 47030446}; 
    // for (int64_t id : searchIDs) {
    //     hashIndex.findAndPrintEmployee(id);
    //     std::cout << std::endl << std::endl;
    // }



    return 0;
}