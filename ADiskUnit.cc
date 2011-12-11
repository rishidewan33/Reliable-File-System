#include "ADiskUnit.h"

ADiskUnit::ADiskUnit(ADisk adisk)
{
    this->adisk = adisk;
    for(int i = 0; i < SECTOR_SIZE; ++i)
    {
        ALL_ZEROES[i] = 0;
        ALL_ONES[i] = 1;
        ALL_TWOS[i] = 2;
        ALL_THREES[i] = 3;
        ALL_FOURS[i] = 4;
        ALL_FIVES[i] = 5;
        ALL_SIXES[i] = 6;
        ALL_SEVENS[i] = 7;
    }
}

ADiskUnit::~ADiskUnit()
{
    
}

