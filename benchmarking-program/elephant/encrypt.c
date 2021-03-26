#include "api.h"
#include "crypto_aead.h"
#include <string.h> 
#include "elephant_160.h"
#include <stdio.h>
#include <stdlib.h>

BYTE rotl3(BYTE b)
{
    return (b << 3) | (b >> 5);
}

int constcmp(const BYTE* a, const BYTE* b, SIZE length)
{
    BYTE r = 0;

    for (SIZE i = 0; i < length; ++i)
        r |= a[i] ^ b[i];
    return r; 
}


// State should be BLOCK_SIZE bytes long
// Note: input may be equal to output
void lfsr_step(BYTE* output, BYTE* input)
{
    BYTE temp = rotl3(input[0]) ^ (input[3] << 7) ^ (input[13] >> 7);
    for(SIZE i = 0; i < BLOCK_SIZE - 1; ++i)
        output[i] = input[i + 1];
    output[BLOCK_SIZE - 1] = temp;
}

void xor_block(BYTE* state, const BYTE* block, SIZE size)
{
    for(SIZE i = 0; i < size; ++i)
        state[i] ^= block[i];
}

// Write the ith assocated data block to "output".
// The nonce is prepended and padding is added as required.
// adlen is the length of the associated data in bytes
void get_ad_block(BYTE* output, const BYTE* ad, SIZE adlen, const BYTE* npub, SIZE i)
{
    SIZE len = 0;
    // First block contains nonce
    // Remark: nonce may not be longer then BLOCK_SIZE
    if(i == 0) {
        memcpy(output, npub, CRYPTO_NPUBBYTES);
        len += CRYPTO_NPUBBYTES;
    }

    const SIZE block_offset = i * BLOCK_SIZE - (i != 0) * CRYPTO_NPUBBYTES;
    // If adlen is divisible by BLOCK_SIZE, add an additional padding block
    if(i != 0 && block_offset == adlen) {
        memset(output, 0x00, BLOCK_SIZE);
        output[0] = 0x01;
        return;
    }
    const SIZE r_outlen = BLOCK_SIZE - len;
    const SIZE r_adlen  = adlen - block_offset;
    // Fill with associated data if available
    if(r_outlen <= r_adlen) { // enough AD
        memcpy(output + len, ad + block_offset, r_outlen);
    } else { // not enough AD, need to pad
        if(r_adlen > 0) // ad might be nullptr
            memcpy(output + len, ad + block_offset, r_adlen);
        memset(output + len + r_adlen, 0x00, r_outlen - r_adlen);
        output[len + r_adlen] = 0x01;
    }
}

// Return the ith assocated data block.
// clen is the length of the ciphertext in bytes 
void get_c_block(BYTE* output, const BYTE* c, SIZE clen, SIZE i)
{
    const SIZE block_offset = i * BLOCK_SIZE;
    // If clen is divisible by BLOCK_SIZE, add an additional padding block
    if(block_offset == clen) {
        memset(output, 0x00, BLOCK_SIZE);
        output[0] = 0x01;
        return;
    }
    const SIZE r_clen  = clen - block_offset;
    // Fill with associated data if available
    if(BLOCK_SIZE <= r_clen) { // enough ciphertext
        memcpy(output, c + block_offset, BLOCK_SIZE);
    } else { // not enough ciphertext, need to pad
        if(r_clen > 0) // c might be nullptr
            memcpy(output, c + block_offset, r_clen);
        memset(output + r_clen, 0x00, BLOCK_SIZE - r_clen);
        output[r_clen] = 0x01;
    }
}

// It is assumed that c is sufficiently long
// Also, tag and c should not overlap
void crypto_aead_impl(
    BYTE* c, BYTE* tag, const BYTE* m, SIZE mlen, const BYTE* ad, SIZE adlen,
    const BYTE* npub, const BYTE* k, int encrypt)
{ 
    // Compute number of blocks
    const SIZE nblocks_c  = mlen ? 1 + mlen / BLOCK_SIZE : 0;
    const SIZE nblocks_m  = (!mlen || mlen % BLOCK_SIZE) ? nblocks_c : nblocks_c - 1;
    const SIZE nblocks_ad = 1 + (CRYPTO_NPUBBYTES + adlen) / BLOCK_SIZE;
    const SIZE nb_it = (nblocks_m > nblocks_ad) ? nblocks_m : nblocks_ad + 1;

    // Storage for the expanded key L
    BYTE expanded_key[BLOCK_SIZE] = {0};
    memcpy(expanded_key, k, CRYPTO_KEYBYTES);
    permutation(expanded_key);

    // Buffers for storing previous, current and next mask
    BYTE mask_buffer_1[BLOCK_SIZE] = {0};
    BYTE mask_buffer_2[BLOCK_SIZE] = {0};
    BYTE mask_buffer_3[BLOCK_SIZE] = {0};
    memcpy(mask_buffer_2, expanded_key, CRYPTO_KEYBYTES);

    BYTE* previous_mask = mask_buffer_1;
    BYTE* current_mask = mask_buffer_2;
    BYTE* next_mask = mask_buffer_3;

    //SIZE offset = 0;

    // Byte pointer arrays to hold the values for these variables at points 
    // in the execution of the loop
    BYTE** previous_masks = (BYTE**)malloc(sizeof(BYTE*)*nb_it);
    BYTE** current_masks =(BYTE**)malloc(sizeof(BYTE*)*nb_it);
    BYTE** next_masks = (BYTE**)malloc(sizeof(BYTE*)*nb_it);

    for(SIZE i = 0; i < nb_it; ++i) {
        lfsr_step(next_mask, current_mask);
        
        BYTE* to_add_previous_mask = malloc(BLOCK_SIZE);
        BYTE* to_add_current_mask = malloc(BLOCK_SIZE);
        BYTE* to_add_next_mask = malloc(BLOCK_SIZE);

        memcpy(to_add_previous_mask, previous_mask, BLOCK_SIZE);
        memcpy(to_add_current_mask, current_mask, BLOCK_SIZE);
        memcpy(to_add_next_mask, next_mask, BLOCK_SIZE);

        previous_masks[i] = to_add_previous_mask;
        current_masks[i] = to_add_current_mask;
        next_masks[i] = to_add_next_mask;
        
        BYTE* const temp = previous_mask;
        previous_mask = current_mask;
        current_mask = next_mask;
        next_mask = temp;
    }

    BYTE** tags = malloc((nblocks_c + nblocks_ad)*sizeof(BYTE*));
    BYTE* hasBeenSet = malloc(nblocks_c + nblocks_ad);
    memset(hasBeenSet, 0, nblocks_c + nblocks_ad);
    memset(tag, 0, CRYPTO_ABYTES);

    BYTE finalTag[CRYPTO_ABYTES] = { 0 };
    #pragma omp parallel 
    {
        #pragma omp for schedule(dynamic, 4)
        for(SIZE i = 0; i < nb_it; ++i) {
            if( i < nblocks_m ) {
                // Buffer to store current ciphertext block
                BYTE* c_buffer = calloc(1,BLOCK_SIZE);
                SIZE offset = i * BLOCK_SIZE;

                // Compute ciphertext block
                memcpy(c_buffer, npub, CRYPTO_NPUBBYTES);
                memset(c_buffer + CRYPTO_NPUBBYTES, 0, BLOCK_SIZE - CRYPTO_NPUBBYTES);
                xor_block(c_buffer, current_masks[i], BLOCK_SIZE);
                permutation(c_buffer);
                xor_block(c_buffer, current_masks[i], BLOCK_SIZE);
                const SIZE r_size = (i == nblocks_m - 1) ? mlen - offset : BLOCK_SIZE;
                xor_block(c_buffer, m + offset, r_size);
                memcpy(c + offset, c_buffer, r_size);     
                free(c_buffer);
            }
            if( i < nblocks_c ) {
                // Compute tag for ciphertext block
                // This dependency is fine as the offset at this point is considered
                BYTE* tag_buffer_c = calloc(1,BLOCK_SIZE);
                get_c_block(tag_buffer_c, encrypt ? c : m, mlen, i);
                xor_block(tag_buffer_c, current_masks[i], BLOCK_SIZE);
                xor_block(tag_buffer_c, next_masks[i], BLOCK_SIZE);
                permutation(tag_buffer_c);
                xor_block(tag_buffer_c, current_masks[i], BLOCK_SIZE);
                xor_block(tag_buffer_c, next_masks[i], BLOCK_SIZE);
                tags[i] = tag_buffer_c;
                hasBeenSet[i] = 1;
            }
            if( i > 0 && i <= nblocks_ad ) { 
                BYTE* tag_buffer_ad = calloc(1,BLOCK_SIZE);
                get_ad_block(tag_buffer_ad, ad, adlen, npub, i - 1);
                xor_block(tag_buffer_ad, previous_masks[i], BLOCK_SIZE);
                xor_block(tag_buffer_ad, next_masks[i], BLOCK_SIZE);
                permutation(tag_buffer_ad);
                xor_block(tag_buffer_ad, previous_masks[i], BLOCK_SIZE);
                xor_block(tag_buffer_ad, next_masks[i], BLOCK_SIZE);
                //xor_block(tag, tag_buffer, CRYPTO_ABYTES);
                //fprintf(stdout,"%llu\n",nblocks_c+i-1);
                tags[nblocks_c+i-1] = tag_buffer_ad;
                hasBeenSet[nblocks_c+i-1] = 1;
            }
        }
        #pragma omp for reduction(^:finalTag)
        for(SIZE i = 0; i < nblocks_c + nblocks_ad; ++i) {
            BYTE* buffer_tag = tags[i];
            if(hasBeenSet[i]){
                for(SIZE j = 0; j < CRYPTO_ABYTES; ++j) {
                    //fprintf(stdout,"%llu", j);
                    finalTag[j] ^= buffer_tag[j];
                }
                free(tags[i]);
            }
        }

        #pragma omp for
        for(SIZE j = 0; j < CRYPTO_ABYTES; ++j) {
            tag[j] = finalTag[j];
        }
    }

    for(SIZE i = 0; i < nb_it; i++){
        free(previous_masks[i]);
        free(current_masks[i]);
        free(next_masks[i]);
    }

    free(previous_masks);
    free(current_masks);
    free(next_masks);
    free(tags);
    free(hasBeenSet);
}

// Remark: c must be at least mlen + CRYPTO_ABYTES long
int crypto_aead_encrypt(
  unsigned char *c, unsigned long long *clen,
  const unsigned char *m, unsigned long long mlen,
  const unsigned char *ad, unsigned long long adlen,
  const unsigned char *nsec,
  const unsigned char *npub,
  const unsigned char *k)
{ 
    (void)nsec;
    *clen = mlen + CRYPTO_ABYTES;
    BYTE tag[CRYPTO_ABYTES];
    crypto_aead_impl(c, tag, m, mlen, ad, adlen, npub, k, 1);
    memcpy(c + mlen, tag, CRYPTO_ABYTES); 
    return 0;
}

int crypto_aead_decrypt(
  unsigned char *m, unsigned long long *mlen,
  unsigned char *nsec,
  const unsigned char *c, unsigned long long clen,
  const unsigned char *ad, unsigned long long adlen,
  const unsigned char *npub,
  const unsigned char *k)
{
    (void)nsec;
    if(clen < CRYPTO_ABYTES)
        return -1;
    *mlen = clen - CRYPTO_ABYTES;
    BYTE tag[CRYPTO_ABYTES];
    crypto_aead_impl(m, tag, c, *mlen, ad, adlen, npub, k, 0);
    return (constcmp(c + *mlen, tag, CRYPTO_ABYTES) == 0) ? 0 : -1;
}
