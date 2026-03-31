/* Compile with: gcc -O2 -fno-omit-frame-pointer -fdump-rtl-early_remat -S -o test.s test.c */
/* For even higher pressure: add -m32 or -fno-schedule-insns */

#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

/* Prevent inlining to keep RTL complex */
static __attribute__((noinline, noipa))
uint64_t high_pressure_calculation(uint64_t seed) {
    /* Declare many local variables to create register pressure */
    register uint64_t v0  = seed + 1;
    register uint64_t v1  = seed * 2;
    register uint64_t v2  = seed ^ 0x12345678;
    register uint64_t v3  = seed - 0xABCDEF;
    register uint64_t v4  = seed << 3;
    register uint64_t v5  = seed >> 2;
    register uint64_t v6  = seed | 0xF0F0F0F0;
    register uint64_t v7  = seed & 0x0F0F0F0F;
    register uint64_t v8  = seed * 3 + 1;
    register uint64_t v9  = seed * 5 - 2;
    register uint64_t v10 = seed ^ 0xDEADBEEF;
    register uint64_t v11 = seed + 0xCAFEBABE;
    register uint64_t v12 = seed * 7;
    register uint64_t v13 = seed / 3;
    register uint64_t v14 = seed % 1001;
    register uint64_t v15 = ~seed;
    
    /* Create rematerialization candidates - pure functions of inputs */
    uint64_t cand1 = v0 + v1;      /* Cheap: addition */
    uint64_t cand2 = v2 & 0xFF;    /* Cheap: bitwise AND with constant */
    uint64_t cand3 = v3 << 2;      /* Cheap: shift */
    uint64_t cand4 = v4 ^ v5;      /* Cheap: XOR */
    uint64_t cand5 = v6 + 0x1234;  /* Cheap: add constant */
    uint64_t cand6 = v7 * 2;       /* Cheap: multiply by power of 2 */
    uint64_t cand7 = v8 & v9;      /* Cheap: bitwise AND */
    uint64_t cand8 = v10 | 0xAA;   /* Cheap: bitwise OR with constant */
    
    /* Force these candidates to be live across many operations */
    /* by using them later in the function */
    
    /* Complex control flow to create challenging liveness patterns */
    uint64_t sum = 0;
    
    /* Outer loop - candidates defined outside, used inside */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use candidates inside loop - they need to stay live or be rematerialized */
        uint64_t t1 = cand1 + i;
        uint64_t t2 = cand2 - i;
        uint64_t t3 = cand3 ^ i;
        uint64_t t4 = cand4 | i;
        
        /* Inner loop with more computations */
        for (int j = 0; j < 5; j++) {
            /* More computations creating register pressure */
            uint64_t a = v0 + v1 + v2 + i + j;
            uint64_t b = v3 * v4 - i * j;
            uint64_t c = v5 & v6 | (i << j);
            uint64_t d = v7 ^ v8 ^ (j * 3);
            uint64_t e = v9 + v10 + (i % 17);
            uint64_t f = v11 * v12 + j;
            uint64_t g = v13 | v14 | (i & 0xFF);
            uint64_t h = v15 - v0 - j;
            
            /* Conditional inside inner loop using different variable sets */
            if ((i + j) % 3 == 0) {
                /* Use candidates in one branch */
                sum += t1 + cand5 + a + b;
                sum ^= cand6 + c + d;
            } else if ((i + j) % 3 == 1) {
                /* Use different candidates in another branch */
                sum += t2 + cand7 + e + f;
                sum ^= cand8 + g + h;
            } else {
                /* Use all candidates in third branch */
                sum += t3 + t4 + cand1 + cand2 + cand3 + cand4;
                sum ^= cand5 + cand6 + cand7 + cand8;
            }
            
            /* More independent computations to increase pressure */
            v0 = v0 * 6364136223846793005ULL + 1;
            v1 = v1 * 2862933555777941757ULL + 1;
            v2 = (v2 >> 1) ^ (-(v2 & 1) & 0xD0000001);
            v3 = v3 * 1664525 + 1013904223;
            v4 = v4 ^ (v4 << 13);
            v5 = v5 ^ (v5 >> 17);
            v6 = v6 ^ (v6 << 5);
            v7 = v7 * 1103515245 + 12345;
            v8 = (v8 & 0x55555555) << 1 | (v8 & 0xAAAAAAAA) >> 1;
            v9 = v9 + (v9 << 2) + (v9 << 4);
            v10 = v10 ^ (v10 << 7);
            v11 = v11 * 0x5DEECE66D + 0xB;
            v12 = v12 + (v12 << 3) + (v12 << 7);
            v13 = v13 ^ 0x9E3779B9;
            v14 = v14 * 0x85EBCA6B;
            v15 = v15 + 0xC0FFEE;
        }
        
        /* Use candidates again after inner loop */
        sum = (sum >> 1) | (sum << 63);  /* Rotate right */
        sum += cand1 * cand2;
        sum ^= cand3 + cand4;
    }
    
    /* Final use of all candidates to ensure they're needed */
    uint64_t final = cand1 + cand2 + cand3 + cand4 + 
                    cand5 + cand6 + cand7 + cand8;
    
    return sum + final;
}

/* Another noinline function to create more compilation context */
static __attribute__((noinline, noipa))
uint64_t secondary_calculation(uint64_t x, uint64_t y) {
    /* Different computation patterns */
    uint64_t a = x * y + (x >> 3);
    uint64_t b = (x ^ y) & 0xF0F0F0F0;
    uint64_t c = (x << 4) | (y >> 4);
    uint64_t d = x % 100 + y % 100;
    
    /* Force many values live across a conditional */
    uint64_t result = 0;
    for (int i = 0; i < 50; i++) {
        uint64_t t1 = a + i;
        uint64_t t2 = b - i;
        uint64_t t3 = c * i;
        uint64_t t4 = d ^ i;
        
        if (i % 4 == 0) {
            result += t1 * t2;
        } else if (i % 4 == 1) {
            result += t3 | t4;
        } else if (i % 4 == 2) {
            result += (t1 << 2) + (t2 >> 2);
        } else {
            result += t3 ^ t4;
        }
        
        /* Modify all variables to prevent CSE */
        a = a * 3 + 1;
        b = b ^ 0x12345678;
        c = c + 0xABCDEF;
        d = d * 5 - 7;
    }
    
    return result;
}

int main(void) {
    uint64_t seed = 0x123456789ABCDEF0ULL;
    
    /* Call high-pressure function multiple times */
    uint64_t result1 = high_pressure_calculation(seed);
    uint64_t result2 = high_pressure_calculation(seed + 1);
    uint64_t result3 = secondary_calculation(seed, seed + 2);
    
    /* Combine results to ensure all computations matter */
    uint64_t final_result = result1 ^ result2 + result3;
    
    printf("Result: 0x%016llX\n", (unsigned long long)final_result);
    
    /* Also print intermediate to prevent dead code elimination */
    printf("Intermediate: %llu %llu %llu\n", 
           (unsigned long long)result1,
           (unsigned long long)result2,
           (unsigned long long)result3);
    
    return 0;
}
