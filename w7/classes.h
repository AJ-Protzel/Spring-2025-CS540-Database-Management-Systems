
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <cmath>

using namespace std;

class Record {
public:
    int id, manager_id; // Employee ID and their manager's ID
    string bio, name; // Fixed length string to store employee name and biography

    Record(vector<string> &fields) {
        id = stoi(fields[0]);
        name = fields[1];
        bio = fields[2];
        manager_id = stoi(fields[3]);
    }

    // Updated: Function to get the size of the record
    int get_size() {
        return sizeof(id) + sizeof(manager_id) + sizeof(int) + name.size() + sizeof(int) + bio.size(); // sizeof(int) is for name/bio size() in serialize function
    }

    // Function to serialize the record for writing to file
    string serialize() const {
        ostringstream oss;
        oss.write(reinterpret_cast<const char *>(&id), sizeof(id));
        oss.write(reinterpret_cast<const char *>(&manager_id), sizeof(manager_id));
        int name_len = name.size();
        int bio_len = bio.size();
        oss.write(reinterpret_cast<const char *>(&name_len), sizeof(name_len));
        oss.write(name.c_str(), name.size());
        oss.write(reinterpret_cast<const char *>(&bio_len), sizeof(bio_len));
        oss.write(bio.c_str(), bio.size());
        return oss.str();
    }

    void print() {
        cout << "\tID: " << id << "\n";
        cout << "\tNAME: " << name << "\n";
        cout << "\tBIO: " << bio << "\n";
        cout << "\tMANAGER_ID: " << manager_id << "\n";
    }
};

class Page {
public:
    vector<Record> records; // Data_Area containing the records
    vector<pair<int, int>> slot_directory; // Slot directory containing offset and size of each record
    int cur_size = sizeof(int); // Updated: Current size of the page including the overflow page pointer. if you also write the length of slot directory change it accordingly.
    int overflowPointerIndex; // Offset of overflow page, set to -1 by default

    // Constructor
    Page() : overflowPointerIndex(-1) {}

    // Function to insert a record into the page
    bool insert_record_into_page(Record r) {
        int record_size = r.get_size();
        int slot_size = sizeof(int) * 2;
        if (cur_size + record_size + slot_size > 4096) { // Updated: Check if page size limit exceeded, considering slot directory size
            return false; // Cannot insert the record into this page
        } else {
            records.push_back(r);
            cur_size += record_size + slot_size; //Updated
            return true;
        }
    }

    // Function to write the page to a binary output stream. You may use
    void write_into_data_file(ostream &out) const {
        char page_data[4096] = {0}; // Buffer to hold page data
        int offset = 0;

        // Write records into page_data buffer
        for (const auto &record: records) {
            string serialized = record.serialize();
            memcpy(page_data + offset, serialized.c_str(), serialized.size());
            offset += serialized.size();
        }

        // TODO 1:
        //  - Write slot_directory in reverse order into page_data buffer.
        //  - Write overflowPointerIndex into page_data buffer.
        //  You should write the first entry of the slot_directory, which have the info about the first record at the bottom of the page, before overflowPointerIndex.
        int slot_offset = 4096 - sizeof(int); // Start from the end for overflowPointerIndex
        memcpy(page_data + slot_offset, &overflowPointerIndex, sizeof(int));
        slot_offset -= slot_directory.size() * sizeof(int) * 2;
        for (int i = slot_directory.size() - 1; i >= 0; --i) {
            memcpy(page_data + slot_offset + (i * sizeof(int) * 2), &slot_directory[i].first, sizeof(int));
            memcpy(page_data + slot_offset + (i * sizeof(int) * 2) + sizeof(int), &slot_directory[i].second, sizeof(int));
        }

        // Write the page_data buffer to the output stream
        out.write(page_data, sizeof(page_data));
    }

    bool read_from_data_file(istream &in) {
    char page_data[4096] = {0};
    in.read(page_data, 4096);
    streamsize bytes_read = in.gcount();

    if (bytes_read == 4096) {
        memcpy(&overflowPointerIndex, page_data + 4096 - sizeof(int), sizeof(int));

        int offset = 0;
        records.clear();
        slot_directory.clear();

        while (offset + sizeof(int) * 4 <= 4096 - sizeof(int)) {
            int id, manager_id, name_len, bio_len;

            memcpy(&id, page_data + offset, sizeof(int));
            offset += sizeof(int);

            memcpy(&manager_id, page_data + offset, sizeof(int));
            offset += sizeof(int);

            memcpy(&name_len, page_data + offset, sizeof(int));
            offset += sizeof(int);

            if (name_len < 0 || offset + name_len > 4096) break;
            string name(page_data + offset, name_len);
            offset += name_len;

            memcpy(&bio_len, page_data + offset, sizeof(int));
            offset += sizeof(int);

            if (bio_len < 0 || offset + bio_len > 4096) break;
            string bio(page_data + offset, bio_len);
            offset += bio_len;

            vector<string> fields = {to_string(id), name, bio, to_string(manager_id)};
            records.emplace_back(fields);

            int record_size = sizeof(int) * 4 + name_len + bio_len;
            slot_directory.emplace_back(offset - record_size, record_size);
        }

        return true;
    }

    if (bytes_read > 0) {
        cerr << "Incomplete read: Expected 4096 bytes, but only read " << bytes_read << " bytes." << endl;
    }

    return false;
}

};

class HashIndex {
private:
    const size_t maxCacheSize = 1; // Maximum number of pages in the buffer
    const int Page_SIZE = 4096; // Size of each page in bytes
    vector<int> PageDirectory; // Map h(id) to a bucket location in EmployeeIndex(e.g., the jth bucket)
    // can scan to correct bucket using j*Page_SIZE as offset (using seek function)
    // can initialize to a size of 256 (assume that we will never have more than 256 regular (i.e., non-overflow) buckets)
    int nextFreePage; // Next place to write a bucket
    string fileName;

    fstream indexFile; // replace local file streams

    // Function to compute hash value for a given ID
    int compute_hash_value(int id) {
        int hash_value;

        // TODO 3: Implement the hash function h = id mod 2^16
        hash_value = id % 65536;

        return hash_value;
    }

    // Function to add a new record to an existing page in the index file
    void addRecordToIndex(int pageIndex, Page &page, Record &record) {
        // Open index file in binary mode for updating

        fstream indexFile(fileName, ios::binary | ios::in | ios::out);

        if (!indexFile) {
            cerr << "Error: Unable to open index file for adding record." << endl;
            return;
        }

        // Check if the page has overflow
        if (page.overflowPointerIndex == -1) {
            // TODO 4: Create overflow page using nextFreePage. update nextFreePage index and pageIndex
            if (!page.insert_record_into_page(record)) {
                Page overflowPage;
                overflowPage.insert_record_into_page(record);
                page.overflowPointerIndex = nextFreePage++;
                indexFile.seekp(pageIndex * Page_SIZE, ios::beg);
                page.write_into_data_file(indexFile);
                indexFile.seekp(page.overflowPointerIndex * Page_SIZE, ios::beg);
                overflowPage.write_into_data_file(indexFile);
                return;
            }

        }

        // Seek to the appropriate position in the index file
        indexFile.seekp(pageIndex * Page_SIZE, ios::beg);
        // TODO 5: Insert record to page and write data to file
        page.insert_record_into_page(record);
        indexFile.seekp(pageIndex * Page_SIZE, ios::beg);
        page.write_into_data_file(indexFile);

        // Close the index file
        indexFile.close();
    }

    // Function to search for a record by ID in a given page of the index file
    void searchRecordByIdInPage(int pageIndex, int id) {
        // Open index file in binary mode for reading
        ifstream indexFile(fileName, ios::binary | ios::in);

        // Seek to the appropriate position in the index file
        indexFile.seekg(pageIndex * Page_SIZE, ios::beg);

        // Read the page from the index file
        Page page;
        page.read_from_data_file(indexFile);

        // TODO 6:
        //  - Search for the record by ID in the page
        //  - Check for overflow pages and report if record with given ID is not found

        while (true) {
            for (const auto& record : page.records) {
                if (record.id == id) {
                    cout << "Record found:\n";
                    Record temp = record; // Create a non-const copy
                    temp.print();         // Call non-const print
                    return;
                }
            }
            if (page.overflowPointerIndex == -1) {
                cout << "Record with ID " << id << " not found.\n";
                return;
            }
            // Load overflow page
            indexFile.seekg(page.overflowPointerIndex * Page_SIZE, ios::beg);
            page.read_from_data_file(indexFile);
        }

    }

public:
    // HashIndex(string indexFileName) : nextFreePage(0), fileName(indexFileName) {
    // }

    // Constructor that opens the index file for binary I/O and truncates it
    HashIndex(const string& indexFileName) : nextFreePage(0), fileName(indexFileName) {
        indexFile.open(fileName, ios::binary | ios::in | ios::out | ios::trunc);
        if (!indexFile.is_open()) {
            cerr << "Failed to open index file: " << fileName << endl;
            exit(EXIT_FAILURE);
        }
    }

    // Destructor to close the file
    ~HashIndex() {
        if (indexFile.is_open()) {
            indexFile.close();
        }
    }

    // Function to create hash index from Employee CSV file
    void createFromFile(string csvFileName) {
        // Read CSV file and add records to index
        // Open the CSV file for reading
        ifstream csvFile(csvFileName);

        string line;
        // Read each line from the CSV file
        unordered_map<int, Page> pageCache;
        while (getline(csvFile, line)) {
            // Parse the line and create a Record object
            stringstream ss(line);
            string item;
            vector<string> fields;
            while (getline(ss, item, ',')) {
                fields.push_back(item);
            }
            Record record(fields);

            // TODO 7:
            //   - Compute hash value for the record's ID using compute_hash_value() function.
            //   - Get the page index from PageDirectory. If it's not in PageDirectory, define a new page using nextFreePage.
            //   - Insert the record into the appropriate page in the index file using addRecordToIndex() function.
            int hash = compute_hash_value(record.id);
            if (PageDirectory.size() <= hash) {
                PageDirectory.resize(hash + 1, -1);
            }
            if (PageDirectory[hash] == -1) {
                PageDirectory[hash] = nextFreePage++;
            }
            if (pageCache.find(PageDirectory[hash]) == pageCache.end()) {
                pageCache[PageDirectory[hash]] = Page();
            }
            addRecordToIndex(PageDirectory[hash], pageCache[PageDirectory[hash]], record);

        }

        // Close the CSV file
        csvFile.close();
    }

    // Function to search for a record by ID in the hash index
    void findAndPrintEmployee(int id) {
        // Open index file in binary mode for reading
        ifstream indexFile(fileName, ios::binary | ios::in);

        // TODO 8:
        //  - Compute hash value for the given ID using compute_hash_value() function
        //  - Search for the record in the page corresponding to the hash value using searchRecordByIdInPage() function

        int hash = compute_hash_value(id);
        if (PageDirectory.size() <= hash || PageDirectory[hash] == -1) {
            cout << "No page found for ID " << id << ".\n";
            return;
        }
        searchRecordByIdInPage(PageDirectory[hash], id);

        // Close the index file
        indexFile.close();
    }
};
