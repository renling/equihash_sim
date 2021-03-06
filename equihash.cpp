#include "equihash.h"

void PrintTable(Table &X, string header="", bool list=false)
{ 
	cout << header << ": len = " << X.size() << endl;
	if (!list)	return;
	for (uint64_t i = 0; i < X.size(); i++)
	{
		if (X[i].size > 0)
			cout << "  " << HexDigest(X[i].value, X[i].size);
		cout << "  (" << (int) X[i].indices->at(0);
		for (int j = 1; j < X[i].indices->size(); j++)
			cout << ", " << (int) X[i].indices->at(j);
		cout << ")" << endl;
	}
	cout << endl;
}

class SortBySubBytes
{
public:
	SortBySubBytes(char step) {  this->step = step;	}
	
	bool operator() (TableEntry x, TableEntry y) { return bytecmp(x.value, y.value) < 0; }	// cannot use <=, need strict ordering

	int bytecmp(byte* x, byte* y) { return memcmp(x, y, step); }

private:
	char step;
};

bool Duplicated(Indices &y, Indices &x1, Indices &x2)
{	
	merge(x1.begin(), x1.end(), x2.begin(), x2.end(), back_inserter(y));
//	return false;	// no deduplication at all, does not work with large n or k
	
//	Indices::iterator indexIt = unique(y.begin(), y.end());
//	return distance(y.begin(), indexIt) != y.size(); // standard dup check

	int i = 0, j = 0;
	while (i < y.size())
	{
		if (i+1 < y.size() && y[i] == y[i+1])
		{
			i += 2;	// skip this pair of elements
			continue;
		}
		else	// keep this element
			y[j++] = y[i++];
	}
	y.resize(j);	
	return j == 0; // check for full dup
}

TableEntry MergeEntry(TableEntry &x1, TableEntry &x2, Indices indy, char step)
{
	TableEntry y;
	if ((y.size = x1.size-step) > 0)
	{
		y.value = new byte [x1.size];			
		CryptoPP::xorbuf(y.value, x1.value+step, x2.value+step, y.size);			
	}
	else
		y.value = NULL;
	y.indices = new Indices(indy);	
	return y;
}

void FindCollision(Table &Y, Table &X, char step)
{
	Y.clear();
	Y.reserve(X.size());
	SortBySubBytes ByteRange(step);
	sort(X.begin(), X.end(), ByteRange);
	PrintTable(X, "sorted", false);

	uint64_t i = 0, j = 0;
	while (i < X.size())
	{
		j = i+1;
		while ( j < X.size() && !ByteRange.bytecmp(X[i].value, X[j].value) )
			j++;
		
		// collision from [i, j)
		for (uint64_t u = i; u < j; u++)
		{
			for (uint64_t v = u+1; v < j; v++)	// add (X[k], X[l])
			{	
				Indices indy;
				if (!Duplicated( indy, *(X[u].indices), *(X[v].indices) ))				
					Y.push_back( MergeEntry(X[u], X[v], indy, step) );				
			}
			delete X[u].value;	// we can free Table X up to row j now
			delete X[u].indices;
		}
		
		i = j;
	}
	// PrintTable(Y, "collide");
}

bool SortSolution(Indices x1, Indices x2)
{
	for (int j = 0; j < x1.size(); j++)
		if (x1[j] != x2[j])
			return x1[j] < x2[j];
	return false;			
}

bool SameSolution(Indices x1, Indices x2)
{
	for (int j = 0; j < x1.size(); j++)
		if (x1[j] != x2[j])
			return false;
	return true;	
}

int SingleListWagner(char nBytes, char kStep, int N, int indexSize, int64_t seed=0)
{
	RandomOracle H(nBytes, seed);	

	byte** input = new byte* [1];
	input[0] = new byte [nBytes];

	Table X, Y;
	X.resize(N);
	for (int j = 0; j < X.size(); j++)
	{
		X[j].size = nBytes;
		X[j].value = new byte [nBytes];
		int2bytes(input[0], j, nBytes);
		H.Digest(X[j].value, input, 1);
		X[j].indices = new Indices;
		X[j].indices->push_back( j % (1 << indexSize) );	// index trimming
	}
//	PrintTable(X, "initial");
	
	for (char k = 0; k < nBytes-2*kStep && X.size() > 0; k += kStep)
	{
		FindCollision(Y, X, kStep);
		swap(X, Y);
	}
	FindCollision(Y, X, 2*kStep);

//	PrintTable(Y, "final", true);
//	return Y.size();

	int real_num_sol = Y.size();
	for (int i = 0; i < Y.size(); i++)
	{
		for (int j = 0; j < i; j++)
		{
			if ( Y[i].indices->size() != Y[j].indices->size() )
				continue;
			int l = 0;
			for (l = 0; l < Y[i].indices->size(); l++)
				if ( Y[i].indices->at(l) != Y[j].indices->at(l) )
					break;

			if ( l == Y[i].indices->size() )
			{
				real_num_sol--;
				break;
			}
		}
	}
	cout << "final: len = " << real_num_sol << endl;
	return real_num_sol;
}


int main(int argc, char* argv[])
{
	char nBytes = 7;
	char kStep = 1;	
	if (argc >= 3)
	{
		nBytes = atoi(argv[1]);
		kStep = atoi(argv[2]);
		if (nBytes % kStep)
		{
			cerr << "Error: kStep does not divide nBytes!" << endl;
			return 0;
		}
	}
	int N = 2 << (kStep * 8);
	int indexSize = kStep * 8 + 1;
	
	if (argc >= 4)
	{
		indexSize = atoi(argv[3]);
		if (indexSize <= 0 || indexSize > kStep * 8 + 1)
		{
			cerr << "Error: invalid index size!" << endl;
			return 0;
		}
	}
	
	int nTest = 50, nSol = 0;
	clock_t start = clock(), diff;
	for (int i = 0; i < nTest; i++)
	{
		nSol += SingleListWagner(nBytes, kStep, N, indexSize, i);
	}	
	diff = clock() - start;
	cout << "Avg # of sol = " << nSol * 1.0 / nTest << ", total time = " << diff / 1000000.0 << endl;
	return 0;
}


