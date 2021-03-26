
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h> 
#include <pthread.h>

#include "memory.h"
#include "crypto_aead.h"
#include "api.h"

#define MAX_MESSAGE_LENGTH			(unsigned long long)pow(2,24)
#define MAX_ASSOCIATED_DATA_LENGTH	(unsigned long long)pow(2,24)

void init_buffer(unsigned char *buffer, unsigned long long numbytes);

void *threadproc(void *arg)
{
	long* memData = (long*) arg;
	long temp_vmrss, temp_vmsize;
	get_memory_usage_kb(&memData[1],&memData[2]);
    while(memData[0] != 1)
    {
		get_memory_usage_kb(&temp_vmrss,&temp_vmsize);
		if(temp_vmrss > memData[3])
			memData[3] = temp_vmrss;
		if(temp_vmsize > memData[4])
			memData[4] = temp_vmsize;
    }
	return 0;
}

struct Benchmark
{
	long start_vmrss[16], start_vmsize[16], end_vmrss[16], end_vmsize[16];
};

typedef struct Benchmark Struct;

Struct generate_test_vectors();

int main()
{
	generate_test_vectors();
	return 0;
}

Struct benchmark()
{
	Struct benchmark;
	benchmark = generate_test_vectors();
	return benchmark;
}

Struct generate_test_vectors()
{
	unsigned char       key[CRYPTO_KEYBYTES];
	unsigned char		nonce[CRYPTO_NPUBBYTES];
	unsigned char*      msg = (unsigned char*) malloc(MAX_MESSAGE_LENGTH *sizeof(unsigned char));
	unsigned char*		ad = (unsigned char*) malloc(MAX_ASSOCIATED_DATA_LENGTH *sizeof(unsigned char));
	unsigned char*		ct = (unsigned char*) malloc((MAX_MESSAGE_LENGTH + CRYPTO_ABYTES) *sizeof(unsigned char));
	unsigned long long  clen;
	
	init_buffer(key, sizeof(key));
	init_buffer(nonce, sizeof(nonce));
	init_buffer(msg, sizeof(msg));
	init_buffer(ad, sizeof(ad));

	struct Benchmark benchmark;
	int point = 0;
	
	// 1 KiloByte to 1 MegaByte
	for (unsigned long long mlen = 1024; (mlen <= MAX_MESSAGE_LENGTH); mlen *= 2) {
		unsigned long long adlen = mlen;
		// Where 0 is a flag for when the algorithm is to stop, 1 and 2 are the minimum values for vmrss and vmsize and 3 and 4 are the max values
		long  memData[5] = {0};
		pthread_t thread1;
		pthread_create( &thread1, NULL, threadproc, (void*) memData);
		sleep(0.1);
		crypto_aead_encrypt(ct, &clen, msg, mlen, ad, adlen, NULL, nonce, key);
		sleep(0.1);
		memData[0] = 1;
		sleep(0.1);
		benchmark.start_vmrss[point] = memData[1];
		benchmark.start_vmsize[point] = memData[2];
		benchmark.end_vmrss[point] = memData[3];
		benchmark.end_vmsize[point] = memData[4];
		point += 1;
	}

	free(msg);
	free(ad);
	free(ct);
	
	return benchmark;
}

void init_buffer(unsigned char *buffer, unsigned long long numbytes)
{
	for (unsigned long long i = 0; i < numbytes; i++)
		buffer[i] = (unsigned char)i;
}

