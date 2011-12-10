#include "Disk.h"
#include "sthread.h"
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cerrno>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char **argv) {
    string op;
    char buf[SECTOR_SIZE];
    Disk disk;
    for (int i = 0; i < SECTOR_SIZE; i++)
        buf[i] = '0';
    disk.writeDisk(NUM_OF_SECTORS - 1, buf);
    int sect_num = -1;
    string sect_value = "";
    float crashprob = 0.0;

    cout << "write    sector_number          sector_value(only one char)\n";
    cout << "read     sector_number\n";
    cout << "setcrash crashprobability(>=0,<=1)\n";
    cout << "quit\n";

    while (1) {
        cout << "Operation(write|read|quit|set crash):";
        cin >> op;

        if (op == "write") {
            cin >> sect_num >> sect_value;
            if (cin.rdstate() & ios_base::failbit || cin.rdstate() & ios_base::badbit) {
                cin.clear();
                cout << "parameter error\n";
            } else {
                char* write_buffer = new char[SECTOR_SIZE]();
                for (int i = 0; i < SECTOR_SIZE; i++)
                    write_buffer[i] = sect_value[0];
                if (disk.writeDisk(sect_num, write_buffer) == SUCCESS)
                    cout << "write succeed\n";
                else
                    cout << "write failure\n";

            }
        } else
            if (op == "read") {
            cin >> sect_num;
            //cout << "op is " << op << sect_num << sect_value << "\n";
            if (cin.rdstate() & ios_base::failbit || cin.rdstate() & ios_base::badbit) {
                cin.clear();
                cout << "parameter error\n";
            } else {
                if (disk.readDisk(sect_num, buf) == SUCCESS) {
                    cout << "read succeed\n";
                    buf[20] = '\0';
                    cout << "buf value is:" << buf << "\n";
                } else
                    cout << "read failure\n";

            }
        } else
            if (op == "quit") {
            exit(0);
        } else
            if (op == "setcrash") {
            cin >> crashprob;
            if (cin.rdstate() & ios_base::failbit || cin.rdstate() & ios_base::badbit) {
                cin.clear();
                cout << "parameter error\n";
            } else {
                disk.setCrashProb(crashprob);
            }
        } else {
            cout << "write    sector_number          sector_value(only one char)\n";
            cout << "read     sector_number\n";
            cout << "setcrash crashprobability(>=0,<=1)\n";
            cout << "quit\n";
        }
        char temp[20];
        cin.get(temp, 20);
        if (cin.rdstate() & ios_base::failbit || cin.rdstate() & ios_base::badbit) {
            cin.clear();
        }
    }
    return 1;
}

