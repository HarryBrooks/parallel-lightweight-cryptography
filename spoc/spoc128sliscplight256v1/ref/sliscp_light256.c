/* Reference implementation of the sliscp_light256 permutation
   Written by:
   Kalikinkar Mandal <kmandal@uwaterloo.ca>
*/
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<stdint.h>
#include "sliscp_light256.h"

/*
   *SC1_256: step constants, applied on S_0
   *SC2_256: step constants, applied on S_2
*/
const uint8_t SC1_256[18]={0x8, 0x86, 0xe2, 0x89, 0xe6, 0xca, 0x17, 0x8e, 0x64, 0x6b, 0x6f, 0x2c, 0xdd, 0x99, 0xea, 0x0f, 0x04, 0x43};// Step constants (SC_{2i})
const uint8_t SC2_256[18]={0x64, 0x6b, 0x6f, 0x2c, 0xdd, 0x99, 0xea, 0xf, 0x4, 0x43, 0xf1, 0x44, 0x73, 0xe5, 0x0b, 0x47, 0xb2, 0xb5};// Step constants (SC_{2i+1})
/*
   *RC1_256: round constants of simeck box applied on S_1
   *RC2_256: round constants of simeck box applied on S_3
*/
const uint8_t RC1_256[18]={0xf, 0x4, 0x43, 0xf1, 0x44, 0x73, 0xe5, 0xb, 0x47, 0xb2, 0xb5, 0x37, 0x96, 0xee, 0x4c, 0xf5, 0x7, 0x82};// Round constants (RC_{2i})
const uint8_t RC2_256[18]={0x47, 0xb2, 0xb5, 0x37, 0x96, 0xee, 0x4c, 0xf5, 0x7, 0x82, 0xa1, 0x78, 0xa2, 0xb9, 0xf2, 0x85, 0x23, 0xd9};// Round constants (RC_{2i+1})

uint8_t rotl8 ( const uint8_t x, const uint8_t y, const uint8_t shift )
{
	return ((x<<shift)|(y>>(8-shift)));
}

/***********************************************************
  *******sLiSCP-LIGHT256 permutation implementation*********
  *********************************************************/

void sliscp_print_state256( const uint8_t *state )
{
	uint8_t i;
	for ( i = 0; i < STATEBYTES; i++ )
		printf("%.2x ", state[i]);
	printf("\n");
}

/*
   *simeck64_box: 64-bit simeck sbox
   *rc: 8-bit round constant
   *input: 64-bit input
   *output: 64-bit output
*/
void simeck64_box( uint8_t *output, const uint8_t *input, const uint8_t rc )
{
	uint8_t i, j, t;

	uint8_t tmp_shift_1[4*sizeof(uint8_t)] __attribute__((aligned(16)));
	uint8_t tmp_shift_5[4*sizeof(uint8_t)] __attribute__((aligned(16)));;
	uint8_t tmp_pt[SIMECKBYTES*sizeof(uint8_t)] __attribute__((aligned(16)));;

        // 10/1/20 - Completely unrolled and vectorised, both are unaligned
	for ( i = 0; i < SIMECKBYTES; i++ )
		tmp_pt[i] = input[i];

        // 10/1/20 - If we separate out the variables, this is a prime vectorisation candidate expect notice the slight out
        // of sync accesses 3 goes with 0. This does 8 iterations so it may be worth it.
        // Dependency prevents vectorisation
	for ( i = 0; i < SIMECKROUND; i++ )
	{
		tmp_shift_1[0] = rotl8(tmp_pt[0], tmp_pt[1],1);
		tmp_shift_1[1] = rotl8(tmp_pt[1], tmp_pt[2],1);
		tmp_shift_1[2] = rotl8(tmp_pt[2], tmp_pt[3],1);
		tmp_shift_1[3] = rotl8(tmp_pt[3], tmp_pt[0],1);

		tmp_shift_5[0] = rotl8(tmp_pt[0], tmp_pt[1],5);
		tmp_shift_5[1] = rotl8(tmp_pt[1], tmp_pt[2],5);
		tmp_shift_5[2] = rotl8(tmp_pt[2], tmp_pt[3],5);
		tmp_shift_5[3] = rotl8(tmp_pt[3], tmp_pt[0],5);
                /*
		tmp_shift_5[0] = tmp_shift_5[0]&tmp_pt[0];
		tmp_shift_5[1] = tmp_shift_5[1]&tmp_pt[1];
		tmp_shift_5[2] = tmp_shift_5[2]&tmp_pt[2];
		tmp_shift_5[3] = tmp_shift_5[3]&tmp_pt[3];

		tmp_shift_1[0] = tmp_shift_1[0]^tmp_shift_5[0];
		tmp_shift_1[1] = tmp_shift_1[1]^tmp_shift_5[1];
		tmp_shift_1[2] = tmp_shift_1[2]^tmp_shift_5[2];
		tmp_shift_1[3] = tmp_shift_1[3]^tmp_shift_5[3];
		*/
                #pragma omp simd
                for(j=0; j < 4; j++){
                        tmp_shift_5[j] = tmp_shift_5[j]&tmp_pt[j];
                        tmp_shift_1[j] = tmp_shift_1[j]^tmp_shift_5[j];
                }
                
		tmp_shift_1[0] = tmp_shift_1[0]^tmp_pt[4]^(0xff);
		tmp_shift_1[1] = tmp_shift_1[1]^tmp_pt[5]^(0xff);
		tmp_shift_1[2] = tmp_shift_1[2]^tmp_pt[6]^(0xff);
		tmp_shift_1[3] = tmp_shift_1[3]^tmp_pt[7]^(0xfe);

		t = (rc >> i)&1;
		tmp_shift_1[3] = tmp_shift_1[3]^t;
		//printf("%d ", t);
		tmp_pt[4] = tmp_pt[0];
		tmp_pt[5] = tmp_pt[1];
		tmp_pt[6] = tmp_pt[2];
		tmp_pt[7] = tmp_pt[3];

		tmp_pt[0] = tmp_shift_1[0];
		tmp_pt[1] = tmp_shift_1[1];
		tmp_pt[2] = tmp_shift_1[2];
		tmp_pt[3] = tmp_shift_1[3];
                
		//simeck_print_data(tmp_pt, 8);
	}

        // 10/1/20 - Completely unrolled and vectorised, both are unaligned
	for ( i = 0; i < SIMECKBYTES; i++ )
		output[i] = tmp_pt[i];

return;
}

/*
   *sliscp_permutation256r18: 18-round sliscp-light permutation of width 256 bits
   *input: 256-bit input, and output is stored in input (inplace).  
*/
void sliscp_permutation256r18 ( uint8_t *input )
{
        uint8_t i, j;
        
        uint8_t tmp_pt[STATEBYTES*sizeof(uint8_t)];
        uint8_t tmp_block[SIMECKBYTES*sizeof(uint8_t)];
        uint8_t simeck_inp[SIMECKBYTES*sizeof(uint8_t)];
        
        // 10/1/20 - Vectorised and completely unrolled but both are unaligned
        // 12/1/20 - Enforced vectorisation to be sure
        #pragma omp simd
        for ( i = 0; i < STATEBYTES; i++ )
                tmp_pt[i] = input[i];
        
        for ( i = 0; i < NUMSTEPSFULL; i++ )
        {


                for (j = 0; j < SIMECKBYTES; j++) {
                        tmp_block[j] = tmp_pt[j];
                        simeck_inp[j] = tmp_pt[SIMECKBYTES+j];
                }
                
                simeck64_box( simeck_inp, simeck_inp, RC1_256[i] );

                for ( j = 0; j < SIMECKBYTES-1; j++) {
                        tmp_block[j] ^= simeck_inp[j] ^ 0xff;
                        tmp_pt[j] = simeck_inp[j]; // x0' = F(x1)
                        simeck_inp[j] = tmp_pt[3*SIMECKBYTES + j];
                }
                tmp_block[SIMECKBYTES-1] ^= simeck_inp[SIMECKBYTES-1] ^ SC1_256[i];
                tmp_pt[SIMECKBYTES-1] = simeck_inp[SIMECKBYTES-1];
                simeck_inp[SIMECKBYTES-1] = tmp_pt[4*SIMECKBYTES -1];
                
                simeck64_box ( simeck_inp, simeck_inp, RC2_256[i] );
                
                for ( j = 0; j < SIMECKBYTES-1; j++ ) {
                        tmp_pt[SIMECKBYTES+j] = tmp_pt[2*SIMECKBYTES+j]^simeck_inp[j]^0xff; //x2^F(x3)
                        tmp_pt[2*SIMECKBYTES + j ] = simeck_inp[j]; // x2' = F(x3)//
                        tmp_pt[3*SIMECKBYTES + j ] = tmp_block[j]; // x3' = RC[0]^x0^F(x1)//
                }
                tmp_pt[2*SIMECKBYTES-1] = tmp_pt[3*SIMECKBYTES-1]^simeck_inp[SIMECKBYTES-1]^SC2_256[i];
                tmp_pt[3*SIMECKBYTES -1 ] = simeck_inp[SIMECKBYTES-1]; // x2' = F(x3)//
                tmp_pt[4*SIMECKBYTES -1 ] = tmp_block[SIMECKBYTES-1]; // x3' = RC[0]^x0^F(x1)//
                //sliscp_print_state256(tmp_pt); // Printing intermediate state
        }
        for ( i = 0; i < STATEBYTES; i++ )
                input[i] = tmp_pt[i];
        
        return;
}
/*
  *sliscp_permutation256_ALLZERO: print output on input all-zero (0^256)
  *state: output
*/
void sliscp_permutation256r18_ALLZERO ( uint8_t *state )
{
	uint8_t i;
	
        // 10/1/20 - Loop was vectorised with completely unrolled and large speed up
	for ( i = 0; i < STATEBYTES; i++ )
		state[i] = 0x0;
	sliscp_print_state256( state );
	sliscp_permutation256r18(state );
return;
}

/*
  *sliscp_permutation256_ALLONE: print output on input all-one (1^256)
  *state: output
*/
void sliscp_permutation256r18_ALLONE ( uint8_t *state )
{
	uint8_t i;
	
        // Completely unrolled and vectorised, unaligned accesses
	for ( i = 0; i < STATEBYTES; i++ )
		state[i] = 0xff;
	sliscp_print_state256( state );
	sliscp_permutation256r18(state);
return;
}

