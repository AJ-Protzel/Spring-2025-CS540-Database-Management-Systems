make network drive :Z protzela@hadoop-master.engr.oregonstate.edu\Users\protzela

login vscode: ssh protzela@hadoop-master.engr.oregonstate.edu

run code:
g++ -std=c++11 main.cpp -o main
terminal: ./main

g++ -std=c++11 main.cpp -o main.out
file: ./main.out

TODO:
1. Write slot directory and overflow pointer into the page buffer.
2. Deserialize page data to reconstruct records, slot directory, and overflow pointer.
3. Implement hash function: id mod 2^16.
4. Create and link an overflow page when needed.
5. Insert record into page and write it to the index file.
6. Search for a record by ID, including checking overflow pages.
7. Build hash index from CSV: compute hash, manage page directory, insert records.
8. Search for and print employee record by ID using the hash index.
