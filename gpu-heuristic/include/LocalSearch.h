#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include <string>
#include <iostream>
#include <algorithm>
#include "Problem.h"

using namespace std;

class LocalSearch{
    public:
        virtual Problem * search();
};

#endif