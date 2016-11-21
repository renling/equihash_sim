#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <memory.h>
#include <ctime>
#include "util.h"

using namespace std;

typedef vector<int> Indices; 

struct TableEntry
{
	byte* value;
	Indices* indices;
	char size;
};

typedef vector<TableEntry> Table;
 

