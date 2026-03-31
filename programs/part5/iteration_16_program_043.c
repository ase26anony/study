/* Compile with: gcc -O2 -fno-omit-frame-pointer -fdump-rtl-early_remat -m32 -o test test.c */
/* For 64-bit: gcc -O2 -fno-omit-frame-pointer -fdump-rtl-early_remat -fno-schedule-insns -o test test.c */

#include <stdio.h>
#include <stdint.h>

#define UNROLL_FACTOR 8
#define LIVE_RANGE_EXTENSION 20

/* Core function with high register pressure - marked noinline to prevent simplification */
static __attribute__((noinline)) 
uint32_t high_pressure_computation(const uint32_t* input, int iterations) {
    /* Declare many local variables to consume registers */
    register uint32_t a0, a1, a2, a3, a4, a5, a6, a7;
    register uint32_t b0, b1, b2, b3, b4, b5, b6, b7;
    register uint32_t c0, c1, c2, c3, c4, c5, c6, c7;
    register uint32_t d0, d1, d2, d3, d4, d5, d6, d7;
    register uint32_t e0, e1, e2, e3, e4, e5, e6, e7;
    
    /* Initialize from input array with different patterns */
    a0 = input[0] + 1;   /* Candidate for remat: input[0] + 1 */
    a1 = input[1] * 3;   /* Candidate: input[1] * 3 */
    a2 = input[2] ^ 0x55; /* Candidate: input[2] ^ 0x55 */
    a3 = input[3] << 2;  /* Candidate: input[3] << 2 */
    a4 = input[4] | 0xAA; /* Candidate: input[4] | 0xAA */
    a5 = input[5] - 7;   /* Candidate: input[5] - 7 */
    a6 = input[6] & 0xF0; /* Candidate: input[6] & 0xF0 */
    a7 = input[7] >> 1;  /* Candidate: input[7] >> 1 */
    
    /* More initializations creating long live ranges */
    b0 = a0 + input[8];   /* Uses a0, keeps it live */
    b1 = a1 - input[9];   /* Uses a1, keeps it live */
    b2 = a2 | input[10];  /* Uses a2, keeps it live */
    b3 = a3 ^ input[11];  /* Uses a3, keeps it live */
    b4 = a4 & input[12];  /* Uses a4, keeps it live */
    b5 = a5 + input[13];  /* Uses a5, keeps it live */
    b6 = a6 - input[14];  /* Uses a6, keeps it live */
    b7 = a7 | input[15];  /* Uses a7, keeps it live */
    
    /* Complex control flow to create merging points with many live values */
    uint32_t result = 0;
    for (int i = 0; i < iterations; i++) {
        /* Nested conditional blocks using different subsets of variables */
        if (i & 1) {
            /* Use first set of variables inside loop - their defs are outside */
            c0 = b0 * 2 + i;   /* b0 defined outside, used inside */
            c1 = b1 / 3 + i;   /* b1 defined outside, used inside */
            c2 = b2 << 1 + i;  /* b2 defined outside, used inside */
            c3 = b3 >> 2 + i;  /* b3 defined outside, used inside */
            
            /* Force many computations to keep values live */
            d0 = a0 + c0;  /* a0 defined outside, used inside */
            d1 = a1 + c1;  /* a1 defined outside, used inside */
            d2 = a2 + c2;  /* a2 defined outside, used inside */
            d3 = a3 + c3;  /* a3 defined outside, used inside */
            
            result += d0 + d1 + d2 + d3;
        } else {
            /* Use second set of variables - creating alternative live ranges */
            c4 = b4 ^ i;    /* b4 defined outside, used inside */
            c5 = b5 & i;    /* b5 defined outside, used inside */
            c6 = b6 | i;    /* b6 defined outside, used inside */
            c7 = b7 + i;    /* b7 defined outside, used inside */
            
            d4 = a4 + c4;   /* a4 defined outside, used inside */
            d5 = a5 + c5;   /* a5 defined outside, used inside */
            d6 = a6 + c6;   /* a6 defined outside, used inside */
            d7 = a7 + c7;   /* a7 defined outside, used inside */
            
            result += d4 + d5 + d6 + d7;
        }
        
        /* Additional computations to extend live ranges further */
        for (int j = 0; j < LIVE_RANGE_EXTENSION; j++) {
            /* Use all variables in a complex expression to keep them live */
            e0 = (a0 + j) * (b0 - j);
            e1 = (a1 + j) * (b1 - j);
            e2 = (a2 + j) * (b2 - j);
            e3 = (a3 + j) * (b3 - j);
            e4 = (a4 + j) * (b4 - j);
            e5 = (a5 + j) * (b5 - j);
            e6 = (a6 + j) * (b6 - j);
            e7 = (a7 + j) * (b7 - j);
            
            /* Force use of e variables to prevent dead code elimination */
            result ^= e0 + e1 + e2 + e3 + e4 + e5 + e6 + e7;
        }
        
        /* Unrolled computations to increase register pressure */
        #pragma GCC unroll UNROLL_FACTOR
        for (int k = 0; k < UNROLL_FACTOR; k++) {
            /* Independent computations that can't be CSE'd */
            uint32_t t0 = a0 * k + 0x1234;
            uint32_t t1 = a1 * k + 0x5678;
            uint32_t t2 = a2 * k + 0x9ABC;
            uint32_t t3 = a3 * k + 0xDEF0;
            uint32_t t4 = a4 * k + 0x2468;
            uint32_t t5 = a5 * k + 0x1357;
            uint32_t t6 = a6 * k + 0xFACE;
            uint32_t t7 = a7 * k + 0xBEEF;
            
            /* Use all temporaries */
            result += t0 ^ t1 ^ t2 ^ t3 ^ t4 ^ t5 ^ t6 ^ t7;
        }
    }
    
    /* Final combination using all variables to ensure they're live until the end */
    result += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7;
    result += b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7;
    
    return result;
}

/* Wrapper to add another layer of control flow */
static __attribute__((noinline))
uint32_t compute_with_branches(const uint32_t* data, int size) {
    uint32_t acc = 0;
    
    for (int i = 0; i < size; i += 16) {
        if (i + 16 <= size) {
            /* Process full block with high pressure */
            acc ^= high_pressure_computation(data + i, 4);
        } else {
            /* Process partial block - different execution path */
            uint32_t temp[16] = {0};
            for (int j = 0; j < size - i; j++) {
                temp[j] = data[i + j];
            }
            acc ^= high_pressure_computation(temp, 2);
        }
    }
    
    return acc;
}

int main(void) {
    /* Initialize test data */
    uint32_t test_data[64];
    for (int i = 0; i < 64; i++) {
        test_data[i] = i * 3 + 7;
    }
    
    /* Perform computation that should trigger high register pressure */
    uint32_t result = compute_with_branches(test_data, 64);
    
    /* Use result to prevent optimization */
    printf("Result: %u (0x%08x)\n", result, result);
    
    /* Verify with simple computation */
    uint32_t verify = 0;
    for (int i = 0; i < 64; i++) {
        verify ^= test_data[i] + i;
    }
    printf("Verify: %u (0x%08x)\n", verify, verify);
    
    return (result != 0) ? 0 : 1;
}
