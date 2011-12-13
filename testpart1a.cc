/* 
 * File:   testpart1a.cc
 * Author: rishi
 *
 * Created on December 10, 2011, 8:02 AM
 */
#include <iostream>
#include "ADiskTests.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv)
{
    ADiskUnit adu(ADisk(Disk(),false));
    adu.testSizeOfDiskLists();
    adu.testTransactionFunctionality();
    adu.testTransactionClass();
    cout << "End Testing.\n";
    return 0;
}

