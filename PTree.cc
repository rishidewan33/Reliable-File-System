#include "PTree.h"

PTree::PTree(ADisk adisk, int doFormat)
{
    this->adisk = adisk(doFormat);
}