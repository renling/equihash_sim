#ifndef PEBBLE_UTIL
#define PEBBLE_UTIL

#include <iostream>
#include <stdint.h>
#include <sha.h>
#include <sha3.h>
#include <hex.h>
#include <vector>
#include <memory>
#include <omp.h>
using namespace std;

inline void int2bytes(byte* dst, uint64_t src, int nbytes)
{
	memset(dst, 0, sizeof(byte)*nbytes);
	*((uint64_t*) dst) = src;
}

class RandomOracle
{
public:
	RandomOracle(int digestSize = 32, int64_t seed = 0) 
	{ 
		this->digestSize = digestSize;
		nonce = new byte [digestSize];
		int2bytes(nonce, seed, digestSize);	
	}

	int GetDigestSize()	{ return digestSize; }

	void Digest(byte *output, const byte* const *input, int nInput)
	{
		//for (int i = 0; i < nInput; i++)
		//	memcpy(output, input[i], digestSize);
		//return;	
		hash.Update(nonce, digestSize);
		for (int i = 0; i < nInput; i++)
			hash.Update(input[i], digestSize);
		hash.TruncatedFinal(output, digestSize);
		return;
	}
		
private:	
	CryptoPP::SHA3_256 hash;
	int digestSize;
	byte *nonce;	
};

string HexDigest(const byte* digest, int digestSize)
{
	string encoded;	
	
	CryptoPP::HexEncoder encoder;
	encoder.Initialize();
	encoder.Put(digest, digestSize);
	encoder.MessageEnd();		
	int size = encoder.MaxRetrievable();
	if(size)
	{
		encoded.resize(size);		
		encoder.Get((byte*)encoded.data(), encoded.size());
	}		
	return encoded;
}

class SimplePerm
{
public:
	SimplePerm(uint64_t N) 
	{
		if (N >= (1 << 31))
		{
			std::cout << "[SimplePerm] Cannot handle more than 2^31 elements!\n";
			exit(1);
		}
		Pi.resize(N);
		for (uint64_t i = 0; i < N; i++)
			Pi[i] = i;
		srand(0);	
		for (uint64_t i = N-1; i > 0; i--)
		{
			uint64_t j = rand() % (i+1);
			swap(Pi[i], Pi[j]);
		}			
	}

	uint64_t perm(uint64_t x) { return Pi[x]; }
	vector<uint64_t>::iterator permIdx(uint64_t x) { return Pi.begin() + x; }	// so that the caller can get the next a few values, proabably unnecessary

private:
	vector<uint64_t> Pi;
};
#endif
