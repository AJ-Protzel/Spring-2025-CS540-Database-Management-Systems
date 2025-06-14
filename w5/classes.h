/*** This is just a Skeleton/Starter Code for the External Storage Assignment. This is by no means absolute, in terms of assignment approach/ used functions, etc. ***/
/*** You may modify any part of the code, as long as you stick to the assignments requirements we do not have any issue ***/

// Include necessary standard library headers
#include <string>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <bitset>
using namespace std; // Include the standard namespace

class Record {
public:
    int id, manager_id; // Employee ID and their manager's ID
    std::string bio, name; // Fixed length string to store employee name and biography

    Record(vector<std::string> &fields) {
        id = stoi(fields[0]);
        name = fields[1];
        bio = fields[2];
        manager_id = stoi(fields[3]);
    }

    //You may use this for debugging / showing the record to standard output. 
    void print() {
        cout << "\tID: " << id << "\n";
        cout << "\tNAME: " << name << "\n";
        cout << "\tBIO: " << bio << "\n";
        cout << "\tMANAGER_ID: " << manager_id << "\n";
    }

    // Function to get the size of the record
    int get_size() {
        // sizeof(int) is for name/bio size() in serialize function
        return sizeof(id) + sizeof(manager_id) + sizeof(int) + name.size() + sizeof(int) + bio.size(); 
    }
    
    // Take a look at Figure 9.9 and read the Section 9.7.2 [Record Organization for Variable Length Records]
    // *TODO: Consider using a delimiter in the serialize function to separate these items for easier parsing.
    string serialize() const {
        ostringstream oss;
        // oss.write(reinterpret_cast<const char*>(&id), sizeof(id)); // Writes the binary representation of the ID.
        // oss.write(reinterpret_cast<const char*>(&manager_id), sizeof(manager_id)); // Writes the binary representation of the Manager id
        // int name_len = name.size();
        // int bio_len = bio.size();
        // oss.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len)); // // Writes the size of the Name in binary format.
        // oss.write(name.c_str(), name.size()); // writes the name in binary form
        // oss.write(reinterpret_cast<const char*>(&bio_len), sizeof(bio_len)); // // Writes the size of the Bio in binary format. 
        // oss.write(bio.c_str(), bio.size()); // writes bio in binary form

        char delimiter = ',';
        oss << id << delimiter
            << manager_id << delimiter
            << name << delimiter
            << bio;
        return oss.str();
    }
};

class page{ // Take a look at Figure 9.7 and read Section 9.6.2 [Page organization for variable length records] 
public:
    vector <Record> records; // Data Area: Stores records. 
    vector <pair <int, int>> slot_directory; // This slot directory contains the starting position (offset), and size of the record. 
                                        
    int cur_size = 0; // holds the current size of the page

    // Function to insert a record into the page
    bool insert_record_into_page(Record r) {
        int record_size = r.get_size();
        int slot_size = sizeof(int) * 2;
        if (cur_size + record_size + slot_size > 4096) { //Check if page size limit exceeded, considering slot directory size
            return false; // Cannot insert the record into this page
        } else {
            records.push_back(r); // Record stored in current page
            int record_offset = cur_size; // Offset is current size before adding this record
            cur_size += r.get_size(); // Updating page size

            // *TODO: update slot directory information
            slot_directory.push_back({record_offset, record_size}); // Add offset and size

            return true;
        }
    }

    // Function to write the page to a binary file, i.e., EmployeeRelation.dat file
    void write_into_data_file(ostream& out) const { 
        
        char page_data[4096] = {0}; // Write the page contents (records and slot directory) into this char array so that the page can be written to the data file in one go.

        int offset = 0; // Used as an iterator to indicate where the next item should be stored. Section 9.6.2 contains information that will help you with the implementation.

        for (const auto& record : records) { // Writing the records into the page_data
            string serialized = record.serialize();

            memcpy(page_data + offset, serialized.c_str(), serialized.size());

            offset += serialized.size();
        }

        // *TODO: Write out the slot directory. Optimally, it should start at the end of the page and grow backwards
        int slot_entry_size = sizeof(int) * 2; // offset + length
        int slot_offset = 4096 - slot_directory.size() * slot_entry_size;
        int current_slot_pos = 0;

        for (const auto& slots : slot_directory) { // *TODO: Write the slot-directory information into page_data. You'll use slot-directory to retrieve record(s).
            int record_offset = slots.first;
            int record_length = slots.second;

            // Write offset
            memcpy(page_data + slot_offset + current_slot_pos, &record_offset, sizeof(int));
            current_slot_pos += sizeof(int);

            // Write length
            memcpy(page_data + slot_offset + current_slot_pos, &record_length, sizeof(int));
            current_slot_pos += sizeof(int);
        }
        
        out.write(page_data, sizeof(page_data)); // Write the page_data to the EmployeeRelation.dat file 
    }

    // Read a page from a binary input stream, i.e., EmployeeRelation.dat file to populate a page object
    bool read_from_data_file(istream& in) {
        char page_data[4096] = {0}; // Character array used to read 4 KB from the data file to your main memory. 
        in.read(page_data, 4096); // Read a page of 4 KB from the data file 
        streamsize bytes_read = in.gcount(); // used to check if 4KB was actually read from the data file
        if (bytes_read == 4096) {
            
            // *TODO: You may process page_data (4 KB page) and put the information to the records and slot_directory (main memory).
            // *TODO: You may modify this function to process the search for employee ID in the page you just loaded to main memory.
            records.clear();
            slot_directory.clear();

            int slot_entry_size = sizeof(int) * 2;
            int num_slots = 0;
            for (int i = 4096 - slot_entry_size; i >= 0; i -= slot_entry_size) {
                int offset, length;
                memcpy(&offset, page_data + i, sizeof(int));
                memcpy(&length, page_data + i + sizeof(int), sizeof(int));
                if (offset == 0 && length == 0) break; // End of valid slots
                slot_directory.push_back({offset, length});

                string record_data(page_data + offset, length);
                stringstream ss(record_data);
                vector<string> fields;
                string field;
                while (getline(ss, field, ',')) {
                    fields.push_back(field);
                }
                if (fields.size() == 4) {
                    records.emplace_back(fields);
                }
            }

            return true;
        }

        if (bytes_read > 0) { 
            cerr << "Incomplete read: Expected " << 4096 << " bytes, but only read " << bytes_read << " bytes." << endl;
        }

        return false;
    }
};

class StorageManager {

public:
    string filename;  // Name of the file (EmployeeRelation.dat) where we will store the Pages 
    fstream data_file; // fstream to handle both input and output binary file operations
    vector <page> buffer; // You can have maximum of 3 Pages.
    
    // Constructor that opens a data file for binary input/output; truncates any existing data file
    StorageManager(const string& filename) : filename(filename) {
        data_file.open(filename, ios::binary | ios::out | ios::in | ios::trunc);
        if (!data_file.is_open()) {  // Check if the data_file was successfully opened
            cerr << "Failed to open data_file: " << filename << endl;
            exit(EXIT_FAILURE);  // Exit if the data_file cannot be opened
        }
    }

    // Destructor closes the data file if it is still open
    ~StorageManager() {
        if (data_file.is_open()) {
            data_file.close();
        }
    }

    // Reads data from a CSV file and writes it to EmployeeRelation.dat
    void createFromFile(const string& csvFilename) {
        buffer.resize(3); // You can have maximum of 3 Pages.

        ifstream csvFile(csvFilename);  // Open the Employee.csv file for reading
        
        string line, name, bio;
        int id, manager_id;
        int page_number = 0; // Current page we are working on [at most 3 pages]

        while (getline(csvFile, line)) {   // Read each line from the CSV file, parse it, and create Employee objects
            stringstream ss(line);
            string item;
            vector<string> fields;

            while (getline(ss, item, ',')) {
                fields.push_back(item);
            }
            Record r = Record(fields);  //create a record object            

            
            if (!buffer[page_number].insert_record_into_page(r)) { // inserting that record object to the current page
                
                // Current page is full, move to the next page
                page_number++;
 
                if (page_number >= buffer.size()) {    // Checking if page limit has been reached.
                    
                    for (page& p : buffer) { // using write_into_data_file() to write the pages into the data file
                        p.write_into_data_file(data_file);
                    }
                    page_number = 0; // Starting again from page 0
                    
                    // *TODO: When reusing buffer frames (i.e., setting page_number = 0), be aware of any data left over from 
                    //        previous write-outs. Consider destructing pages before using them.    
                    for (page& p : buffer) {
                        p = page(); // Reinitialize each page to clear old records and slot directory
                    }
                }
                buffer[page_number].insert_record_into_page(r); // Reattempting the insertion of record 'r' into the newly created page
            }
            
        }
        
        // *TODO: make sure to write out any records left over in memory
        for (page& p : buffer) {
            if (!p.records.empty()) {
                p.write_into_data_file(data_file);
            }
        }

        
        csvFile.close();  // Close the CSV file
    }

    // Searches for an Employee ID in EmployeeRelation.dat
    void findAndPrintEmployee(int searchId) {
        data_file.seekg(0, ios::beg);  // Rewind the data_file to the beginning for reading

        // *TODO: Read pages from your data file (using read_from_data_file) and search for the employee ID in those pages. Be mindful of the page limit in main memory.        
        page p;
        bool found = false;

        while (data_file.peek() != EOF) {
            if (!p.read_from_data_file(data_file)) {
                break; // Stop if we can't read a full page
            }
            for (Record& r : p.records) {
                if (r.id == searchId) {
                    r.print();
                    found = true;
                    return;
                }
            }
        }

        // *TODO: Print "Record not found" if no records match.
        if (!found) {
            cout << "Record not found" << endl;
        }

    }
};
