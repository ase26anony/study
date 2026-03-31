/* Compile with: gcc -O2 -fno-omit-frame-pointer -fdump-rtl-early_remat -m32 -o early_remat_test early_remat_test.c */
/* For even higher pressure: add -O3 -funroll-loops -fno-schedule-insns */

#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

/* Force no inlining to keep RTL complex */
static __attribute__((noinline)) 
uint32_t high_pressure_calculation(const uint32_t* input) {
    /* Declare many local variables to create register pressure */
    register uint32_t a0, a1, a2, a3, a4, a5, a6, a7;
    register uint32_t b0, b1, b2, b3, b4, b5, b6, b7;
    register uint32_t c0, c1, c2, c3, c4, c5, c6, c7;
    register uint32_t d0, d1, d2, d3, d4, d5, d6, d7;
    
    /* Initialize from input array - creates many live ranges */
    a0 = input[0] + 1;   /* Candidate for remat: input[0] + 1 */
    a1 = input[1] * 2;   /* Candidate: input[1] * 2 */
    a2 = input[2] & 0xFF; /* Candidate: input[2] & 0xFF */
    a3 = input[3] | 0x80; /* Candidate: input[3] | 0x80 */
    a4 = input[4] ^ 0x55; /* Candidate: input[4] ^ 0x55 */
    a5 = input[5] << 3;   /* Candidate: input[5] << 3 */
    a6 = input[6] >> 2;   /* Candidate: input[6] >> 2 */
    a7 = input[7] + input[0]; /* Candidate: input[7] + input[0] */
    
    b0 = input[8] * 3 + 1;
    b1 = input[9] / 2 + 5;
    b2 = input[10] & 0xF0F0;
    b3 = input[11] | 0x0F0F;
    b4 = input[12] ^ 0xAAAA;
    b5 = input[13] << 1;
    b6 = input[14] >> 4;
    b7 = input[15] - input[1];
    
    c0 = input[16] + 0x100;
    c1 = input[17] * 5;
    c2 = input[18] & 0xCCCC;
    c3 = input[19] | 0x3333;
    c4 = input[20] ^ 0x9999;
    c5 = input[21] << 2;
    c6 = input[22] >> 1;
    c7 = input[23] + input[2];
    
    d0 = input[24] * 7;
    d1 = input[25] + 0x200;
    d2 = input[26] & 0xAAAA;
    d3 = input[27] | 0x5555;
    d4 = input[28] ^ 0xCCCC;
    d5 = input[29] << 4;
    d6 = input[30] >> 3;
    d7 = input[31] - input[3];
    
    /* Create complex control flow with many live values */
    uint32_t result = 0;
    
    /* Outer loop - many values defined outside, used inside */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use all the 'a' variables - they must stay live */
        uint32_t temp_a = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7;
        
        /* Inner loop with conditional - creates merging points */
        for (int j = 0; j < 5; j++) {
            /* Use 'b' variables conditionally */
            if (j & 1) {
                temp_a += b0 + b2 + b4 + b6;
            } else {
                temp_a += b1 + b3 + b5 + b7;
            }
            
            /* More computations that use 'c' and 'd' variables */
            uint32_t temp_b = c0 ^ c1 ^ c2 ^ c3;
            uint32_t temp_c = d0 & d1 & d2 & d3;
            
            /* Conditional that uses different sets */
            if (temp_b > temp_c) {
                result += temp_a + (c4 | c5 | c6 | c7);
            } else {
                result += temp_a + (d4 ^ d5 ^ d6 ^ d7);
            }
            
            /* Modify some values to prevent CSE */
            temp_a += j;
        }
        
        /* Use all variables in final computation to keep them live */
        result += (a0 * i) + (a1 / (i + 1)) + (a2 & i) + (a3 | i);
        result += (b0 - i) + (b1 * (i + 2)) + (b2 ^ i) + (b3 & (i + 3));
        result += (c0 + i) + (c1 - (i + 4)) + (c2 | i) + (c3 ^ (i + 5));
        result += (d0 * i) + (d1 / (i + 6)) + (d2 & i) + (d3 | (i + 7));
        
        /* Additional computations to increase pressure */
        uint32_t extra1 = (a4 << (i & 3)) + (a5 >> ((i + 1) & 3));
        uint32_t extra2 = (b4 ^ (i * 2)) | (b5 & (i * 3));
        uint32_t extra3 = (c4 + (i * 4)) - (c5 ^ (i * 5));
        uint32_t extra4 = (d4 * (i + 1)) & (d5 | (i * 6));
        
        result += extra1 + extra2 + extra3 + extra4;
    }
    
    /* Final use of all variables to ensure they're not dead */
    result += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7;
    result += b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7;
    result += c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7;
    result += d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7;
    
    return result;
}

/* Another function with different pattern to increase chances */
static __attribute__((noinline))
uint32_t nested_pressure_calc(uint32_t x, uint32_t y) {
    /* Create many intermediate values with long live ranges */
    uint32_t v1 = x + 1;      /* Remat candidate */
    uint32_t v2 = y * 2;      /* Remat candidate */
    uint32_t v3 = x & 0xFF;   /* Remat candidate */
    uint32_t v4 = y | 0x80;   /* Remat candidate */
    uint32_t v5 = x ^ y;      /* Remat candidate */
    uint32_t v6 = x << 3;     /* Remat candidate */
    uint32_t v7 = y >> 2;     /* Remat candidate */
    uint32_t v8 = x + y + 1;  /* Remat candidate */
    
    uint32_t sum = 0;
    
    /* Complex loop structure */
    for (uint32_t i = 0; i < 50; i++) {
        /* Conditional block that uses different subsets */
        if (i % 3 == 0) {
            sum += v1 + v3 + v5 + v7;
            /* More computations */
            for (uint32_t j = 0; j < 3; j++) {
                sum += (v2 * j) + (v4 & j) + (v6 ^ j) + (v8 | j);
            }
        } else if (i % 3 == 1) {
            sum += v2 + v4 + v6 + v8;
            /* Different computation pattern */
            uint32_t t1 = v1 * i;
            uint32_t t2 = v3 / (i + 1);
            uint32_t t3 = v5 & i;
            uint32_t t4 = v7 | i;
            sum += t1 + t2 + t3 + t4;
        } else {
            /* Use all variables */
            sum += v1 * v2 + v3 * v4 + v5 * v6 + v7 * v8;
            /* Chain computations */
            sum += ((v1 + i) & (v2 + i)) | ((v3 + i) ^ (v4 + i));
        }
        
        /* Keep all variables live across iteration */
        v1 += (i & 1);
        v2 += (i & 2) >> 1;
        v3 ^= i;
        v4 |= i;
        v5 = v5 * 3 + i;
        v6 = (v6 << 1) | (i & 1);
        v7 = (v7 >> 1) + i;
        v8 = v8 - (i % 4);
    }
    
    return sum;
}

int main() {
    /* Initialize with pattern to avoid constant propagation */
    uint32_t input[32];
    for (int i = 0; i < 32; i++) {
        input[i] = i * 3 + 7;
    }
    
    uint32_t result1 = high_pressure_calculation(input);
    uint32_t result2 = nested_pressure_calc(input[0], input[1]);
    
    printf("Result 1: %u\n", result1);
    printf("Result 2: %u\n", result2);
    printf("Combined: %u\n", result1 + result2);
    
    return 0;
}
