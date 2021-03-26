//
// NIST-developed software is provided by NIST as a public service.
// You may use, copy and distribute copies of the software in any medium,
// provided that you keep intact this entire notice. You may improve, 
// modify and create derivative works of the software or any portion of
// the software, and you may copy and distribute such modifications or
// works. Modified works should carry a notice stating that you changed
// the software and should note the date and nature of any such change.
// Please explicitly acknowledge the National Institute of Standards and 
// Technology as the source of the software.
//
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO 
// WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION
// OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST
// NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE 
// UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST 
// DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE
// OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY,
// RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
//
// You are solely responsible for determining the appropriateness of using and 
// distributing the software and you assume all risks associated with its use, 
// including but not limited to the risks and costs of program errors, compliance 
// with applicable laws, damage to or loss of data, programs or equipment, and 
// the unavailability or interruption of operation. This software is not intended
// to be used in any situation where a failure could cause risk of injury or 
// damage to property. The software developed by NIST employees is not subject to
// copyright protection within the United States.
//

// disable deprecation for sprintf and fopen
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>

#include "crypto_aead.h"
#include "api.h"

#define KAT_SUCCESS          0
#define KAT_FILE_OPEN_ERROR -1
#define KAT_DATA_ERROR      -3
#define KAT_CRYPTO_FAILURE  -4

#define MAX_MESSAGE_LENGTH			(unsigned long long)pow(2,18)
#define MAX_ASSOCIATED_DATA_LENGTH	(unsigned long long)pow(2,18)

void init_buffer(unsigned char *buffer, unsigned long long numbytes);

struct Benchmark
{
	int ret;
	double time_encrypting, time_decrypting, min_throughput_encrypting, max_throughput_encrypting, average_throughput_encrypting,
        min_throughput_decrypting, max_throughput_decrypting, average_throughput_decrypting;
	double encryption_times[16], decryption_times[16];
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
	unsigned char*      msg2 = (unsigned char*) malloc(MAX_MESSAGE_LENGTH *sizeof(unsigned char));
	unsigned char*		ad = (unsigned char*) malloc(MAX_ASSOCIATED_DATA_LENGTH *sizeof(unsigned char));
	unsigned char*		ct = (unsigned char*) malloc((MAX_MESSAGE_LENGTH + CRYPTO_ABYTES) *sizeof(unsigned char));
	unsigned long long  clen, mlen2;
	int                 func_ret, ret_val = KAT_SUCCESS;

	init_buffer(key, sizeof(key));
	init_buffer(nonce, sizeof(nonce));
	init_buffer(msg, sizeof(msg));
	init_buffer(ad, sizeof(ad));

	double time_encrypting = 0;
    double min_throughput_encrypting = DBL_MAX;
    double max_throughput_encrypting = 0.0;
    unsigned long long total_m_encrypted = 0;

	double time_decrypting = 0;
    double min_throughput_decrypting = DBL_MAX;
    double max_throughput_decrypting = 0.0;
    unsigned long long total_c_decrypted = 0;
	int point = 0;

	struct Benchmark benchmark;
    double time_encrypt, time_decrypt, throughput;

    // 1 KiloByte to 1 MegaByte
	for (unsigned long long mlen = 1024; (mlen <= MAX_MESSAGE_LENGTH) && (ret_val == KAT_SUCCESS); mlen *= 2) {
		unsigned long long adlen = mlen;
		struct timespec begin_encrypt, end_encrypt, end_decrypt;

		clock_gettime(CLOCK_REALTIME, &begin_encrypt);

		if ((func_ret = crypto_aead_encrypt(ct, &clen, msg, mlen, ad, adlen, NULL, nonce, key)) != 0) {
			ret_val = KAT_CRYPTO_FAILURE;
			break;
		}

		clock_gettime(CLOCK_REALTIME, &end_encrypt);
		if ((func_ret = crypto_aead_decrypt(msg2, &mlen2, NULL, ct, clen, ad, adlen, nonce, key)) != 0) {
			ret_val = KAT_CRYPTO_FAILURE;
			break;
		}
		clock_gettime(CLOCK_REALTIME, &end_decrypt);
		if (mlen != mlen2) {
			ret_val = KAT_CRYPTO_FAILURE;
			break;
		}
		time_encrypt = end_encrypt.tv_sec - begin_encrypt.tv_sec + (end_encrypt.tv_nsec- begin_encrypt.tv_nsec) / 1000000000.0;
		time_decrypt = end_decrypt.tv_sec - end_encrypt.tv_sec + (end_decrypt.tv_nsec  -   end_encrypt.tv_nsec) / 1000000000.0;
		
		time_encrypting += time_encrypt;
		time_decrypting += time_decrypt;

		throughput = (sizeof(unsigned char) * mlen) / time_encrypt;
		if(throughput < min_throughput_encrypting) {
			min_throughput_encrypting = throughput;
		} 
		if(throughput > max_throughput_encrypting) {
			max_throughput_encrypting = throughput;
		}

		throughput = (sizeof(unsigned char) * clen) / time_decrypt;
		if(throughput < min_throughput_decrypting) {
			min_throughput_decrypting = throughput;
		} 
		if(throughput > max_throughput_decrypting) {
			max_throughput_decrypting = throughput;
		}

		total_m_encrypted += mlen;
		total_c_decrypted += clen;

		benchmark.encryption_times[point] = time_encrypt;
		benchmark.decryption_times[point] = time_decrypt;
		point += 1;

		if (memcmp(msg, msg2, mlen)) {
			ret_val = KAT_CRYPTO_FAILURE;
			break;
		}
		
	}

	free(msg);
	free(msg2);
	free(ad);
	free(ct);

    double average_throughput_encrypting = (sizeof(unsigned char) * total_m_encrypted) / time_encrypting;
    double average_throughput_decrypting = (sizeof(unsigned char) * total_c_decrypted) / time_decrypting;

	benchmark.ret = ret_val;
	benchmark.time_encrypting = time_encrypting;
	benchmark.time_decrypting = time_decrypting;

    benchmark.min_throughput_encrypting = min_throughput_encrypting;
    benchmark.max_throughput_encrypting = max_throughput_encrypting;
    benchmark.average_throughput_encrypting = average_throughput_encrypting;

    benchmark.min_throughput_decrypting = min_throughput_decrypting;
    benchmark.max_throughput_decrypting = max_throughput_decrypting;
    benchmark.average_throughput_decrypting = average_throughput_decrypting;

	return benchmark;
}

void init_buffer(unsigned char *buffer, unsigned long long numbytes)
{
	for (unsigned long long i = 0; i < numbytes; i++)
		buffer[i] = (unsigned char)i;
}
