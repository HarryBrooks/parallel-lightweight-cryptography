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

#include "crypto_aead.h"
#include "api.h"

#define KAT_SUCCESS          0
#define KAT_FILE_OPEN_ERROR -1
#define KAT_DATA_ERROR      -3
#define KAT_CRYPTO_FAILURE  -4

#define MAX_MESSAGE_LENGTH			16
#define MAX_ASSOCIATED_DATA_LENGTH	16

void init_buffer(unsigned char *buffer, unsigned long long numbytes);

struct Benchmark
{
	int ret;
	double time_encrypting, time_decrypting;
};

typedef struct Benchmark Struct;

Struct generate_test_vectors();

int main()
{

	Struct benchmark;
	benchmark = generate_test_vectors();

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
	unsigned char       msg[MAX_MESSAGE_LENGTH];
	unsigned char       msg2[MAX_MESSAGE_LENGTH];
	unsigned char 		ad[MAX_ASSOCIATED_DATA_LENGTH];
	unsigned char 		ct[MAX_MESSAGE_LENGTH + CRYPTO_ABYTES];
	unsigned long long  clen, mlen2;
	int                 func_ret, ret_val = KAT_SUCCESS;

	init_buffer(key, sizeof(key));
	init_buffer(nonce, sizeof(nonce));
	init_buffer(msg, sizeof(msg));
	init_buffer(ad, sizeof(ad));


    unsigned long long mlen = 0, adlen = 0;

    struct timespec begin_encrypt, end_encrypt, end_decrypt;

    clock_gettime(CLOCK_REALTIME, &begin_encrypt);
    if ((func_ret = crypto_aead_encrypt(ct, &clen, msg, mlen, ad, adlen, NULL, nonce, key)) != 0) {
        ret_val = KAT_CRYPTO_FAILURE;
    }
    clock_gettime(CLOCK_REALTIME, &end_encrypt);
    if ((func_ret = crypto_aead_decrypt(msg2, &mlen2, NULL, ct, clen, ad, adlen, nonce, key)) != 0) {
        ret_val = KAT_CRYPTO_FAILURE;
    }
    clock_gettime(CLOCK_REALTIME, &end_decrypt);
    if (mlen != mlen2) {
        ret_val = KAT_CRYPTO_FAILURE;

    }
    double time_encrypt = end_encrypt.tv_sec - begin_encrypt.tv_sec + (end_encrypt.tv_nsec- begin_encrypt.tv_nsec) / 1000000000.0;
    double time_decrypt = end_decrypt.tv_sec - end_encrypt.tv_sec + (end_decrypt.tv_nsec  -   end_encrypt.tv_nsec) / 1000000000.0;

    if (memcmp(msg, msg2, mlen)) {
        ret_val = KAT_CRYPTO_FAILURE;
    }

	struct Benchmark benchmark;
	benchmark.ret = ret_val;
	benchmark.time_encrypting = time_encrypt;
	benchmark.time_decrypting = time_decrypt;

	return benchmark;
}

void init_buffer(unsigned char *buffer, unsigned long long numbytes)
{
	for (unsigned long long i = 0; i < numbytes; i++)
		buffer[i] = (unsigned char)i;
}
