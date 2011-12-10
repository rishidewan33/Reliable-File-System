/* 
 * File:   testpart1.cc
 * Author: rishi
 *
 * Created on November 29, 2011, 12:40 AM
 */

#include "ADisk.h"
#include <cstdlib>
#include <iostream>
#include <cassert>

using namespace std;

int main(int argc, char** argv) {
    Disk d;
    bool format;
    char choice = 'z';
    while (choice != 'y' && choice != 'n') {
        std::cout << "Format Disk? (y/n) ";
        std::cin >> choice;
        switch (choice) {
            case 'y':
                format = true;
                break;
            case 'n':
                format = false;
                break;
        }
    }
    ADisk adisk(d, format);
    TransID transid;
    int option;
    int sectornum;
    char read_buffer[SECTOR_SIZE];
    char value;
    std::cout << "------------WELCOME TO ATOMIC DISK------------ " << std::endl;
    std::cout << "---------------Choose an option--------------- " << std::endl;

    while (1) {
        std::cout << "1. Create a transaction\n2. Write to a sector\n3. Read from a sector\n4. View all active transaction IDs.\n5. Abort Transaction\n6. Commit Transaction\n7. Print buffer\n8. Quit\n\n";
        std::cout << "Pick an option (1-8): ";
        std::cin >> option;
        switch (option) {
            case 1:
            {
                if(adisk.getActiveTransactionIDs().size() == (unsigned int)MAX_CONCURRENT_TRANSACTIONS)
                {
                    std::cout << "You currently have the max allowed active transactions. Commit or abort some before creating more.\n";
                    break;
                }
                adisk.beginTransaction();
                std::cout << "Transaction Created.\n\n";
                break;
            }
            case 2:
            {
                std::cout << "Enter a transaction ID: ";
                std::cin >> transid;
                if (std::cin.rdstate() & (ios_base::badbit || ios_base::failbit)) {
                    std::cout << "Parameter error (TransID).\n\n";
                    break;
                }
                std::cout << "Enter a sector number: ";
                std::cin >> sectornum;
                if (std::cin.rdstate() & (ios_base::badbit || ios_base::failbit)) {
                    std::cout << "Parameter error (Sector Number).\n\n";
                    break;
                }
                std::cout << "Enter a value (one character only): ";
                std::cin >> value;
                if (std::cin.rdstate() & (ios_base::badbit || ios_base::failbit)) {
                    std::cout << "Parameter error (Character).\n\n";
                    break;
                }
                char* write_buffer = new char[SECTOR_SIZE]();
                for (int i = 0; i < SECTOR_SIZE; i++)
                    write_buffer[i] = value;
                if (adisk.writeSector(transid, sectornum, write_buffer) == FAILURE)
                    std::cout << "Sector Write Failed.\n\n";
                break;
            }
            case 3:
            {
                std::cout << "Enter a transaction ID: ";
                std::cin >> transid;
                if (std::cin.rdstate() & (ios_base::badbit || ios_base::failbit)) {
                    std::cout << "Parameter error (TransID).\n\n";
                    break;
                }
                std::cout << "Enter a sector number: ";
                std::cin >> sectornum;
                if (std::cin.rdstate() & (ios_base::badbit || ios_base::failbit)) {
                    std::cout << "Parameter error (Sector Number).\n\n";
                    break;
                }
                if (adisk.readSector(transid, sectornum, read_buffer) == FAILURE)
                    std::cout << "Sector Read Failed.\n\n";
                break;
            }
            case 4:
            {
                std::vector<TransID> ids = adisk.getActiveTransactionIDs();
                for (std::vector<TransID>::iterator it = ids.begin(); it != ids.end(); ++it)
                    std::cout << (*it) << " ";
                std::cout << std::endl;
                break;
            }
            case 5:
            {
                std::cout << "Enter a transaction ID: ";
                std::cin >> transid;
                if (std::cin.rdstate() & (ios_base::badbit || ios_base::failbit)) {
                    std::cout << "Parameter error (TransID).\n\n";
                    break;
                }
                if (adisk.abortTransaction(transid) == FAILURE)
                    std::cout << "Transaction Abortion Failed.\n\n";
                break;
            }
            case 6:
            {
                std::cout << "Enter a transaction ID: ";
                std::cin >> transid;
                if (std::cin.rdstate() & (ios_base::badbit || ios_base::failbit)) {
                    std::cout << "Parameter error (TransID).\n\n";
                    break;
                }
                if (adisk.commitTransaction(transid) == FAILURE)
                    std::cout << "Transaction Commit Failed.\n\n";
                break;
            }
            case 7:
            {
                for (int i = 0; i < SECTOR_SIZE; i++)
                    std::cout << read_buffer[i];
                std::cout << std::endl;
                break;
            }
            case 8:
            {
                std::cout << "Bye.\n\n";
                return EXIT_SUCCESS;
            }
            default:
            {
                break;
            }
        }
    }
}