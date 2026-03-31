/* early-remat-test.c
 * Designed to trigger early rematerialization in GCC's RTL optimizer
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fdump-rtl-early_remat early-remat-test.c -o early_remat_test
 */

#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent optimization */
static __attribute__((noinline)) uint64_t
high_pressure_computation(const uint32_t* input) {
    /* Declare many distinct variables to increase register pressure */
    register uint32_t v0  = input[0]  ^ 0x5A5A5A5A;
    register uint32_t v1  = input[1]  + 0x12345678;
    register uint32_t v2  = input[2]  | 0xF0F0F0F0;
    register uint32_t v3  = input[3]  & 0x0F0F0F0F;
    register uint32_t v4  = input[4]  * 3;
    register uint32_t v5  = input[5]  - 0x11111111;
    register uint32_t v6  = input[6]  ^ 0x33333333;
    register uint32_t v7  = input[7]  + 0x77777777;
    register uint32_t v8  = input[8]  | 0xAAAAAAAA;
    register uint32_t v9  = input[9]  & 0x55555555;
    register uint32_t v10 = input[10] * 5;
    register uint32_t v11 = input[11] - 0x22222222;
    register uint32_t v12 = input[12] ^ 0x44444444;
    register uint32_t v13 = input[13] + 0x88888888;
    register uint32_t v14 = input[14] | 0xCCCCCCCC;
    register uint32_t v15 = input[15] & 0x33333333;
    
    /* Create rematerialization candidates - pure functions of inputs */
    uint32_t cand1 = v0 + 0x1000;      /* Simple addition - cheap to recompute */
    uint32_t cand2 = v1 & 0x00FFFFFF;  /* Mask operation - cheap */
    uint32_t cand3 = v2 << 3;          /* Shift - cheap */
    uint32_t cand4 = v3 ^ 0x80000000;  /* XOR - cheap */
    uint32_t cand5 = v4 + v5;          /* Addition of two vars - still cheap */
    uint32_t cand6 = v6 | 0x0000FFFF;  /* OR with mask */
    uint32_t cand7 = v7 - 0x100;       /* Subtraction */
    uint32_t cand8 = v8 >> 2;          /* Right shift */
    
    /* Use candidates once initially, but they'll need to stay live */
    uint32_t temp1 = cand1 * 2;
    uint32_t temp2 = cand2 / 2;
    uint32_t temp3 = cand3 | 0xFF;
    uint32_t temp4 = cand4 ^ 0x55;
    
    /* Complex control flow to create challenging liveness patterns */
    uint64_t accumulator = 0;
    
    /* Outer loop - uses candidates defined outside */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Inner loop with conditional branches */
        for (int j = 0; j < 4; j++) {
            /* Use rematerialization candidates inside loops */
            if (j % 2 == 0) {
                /* Even iteration path - uses some candidates */
                accumulator += cand1;
                accumulator += cand3;
                accumulator += cand5;
                accumulator += cand7;
                
                /* More computations to increase pressure */
                v0 = v0 * 2 + 1;
                v2 = v2 ^ (v3 << j);
                v4 = v4 + (v5 >> j);
            } else {
                /* Odd iteration path - uses different candidates */
                accumulator += cand2;
                accumulator += cand4;
                accumulator += cand6;
                accumulator += cand8;
                
                /* Different computations */
                v1 = v1 / 2 + v0;
                v3 = v3 | (v2 & 0xFF);
                v5 = v5 - (v4 ^ 0xAA);
            }
            
            /* Use all variables to keep them live */
            accumulator += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
            accumulator += v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
            
            /* More independent computations */
            uint32_t t1 = v0 * v1 + v2;
            uint32_t t2 = v3 / (v4 + 1) + v5;
            uint32_t t3 = v6 & v7 | v8;
            uint32_t t4 = v9 ^ v10 ^ v11;
            uint32_t t5 = v12 << (v13 & 3);
            uint32_t t6 = v14 >> (v15 & 3);
            uint32_t t7 = t1 + t2 + t3;
            uint32_t t8 = t4 * t5 - t6;
            
            accumulator += t7 + t8;
        }
        
        /* Use the temps that depend on candidates */
        accumulator += temp1 + temp2 + temp3 + temp4;
        
        /* Modify some variables to prevent CSE */
        v0 ^= i;
        v1 += i * 2;
        v2 |= i;
        v3 &= ~i;
        v4 *= (i % 5) + 1;
        v5 -= i * 3;
        v6 ^= 0x55AA55AA + i;
        v7 += 0xAA55AA55 - i;
    }
    
    /* Final computation using all candidates and variables */
    uint64_t final = accumulator;
    final += cand1 * cand2;
    final += cand3 * cand4;
    final += cand5 * cand6;
    final += cand7 * cand8;
    
    final += v0 * 1000000007ULL;
    final += v1 * 1000000009ULL;
    final += v2 * 1000000021ULL;
    final += v3 * 1000000033ULL;
    
    return final;
}

/* Another function to create more complex call graph */
static __attribute__((noinline)) uint64_t
nested_high_pressure(const uint32_t* input, int selector) {
    uint64_t result = 0;
    
    if (selector & 1) {
        /* Path with different computation patterns */
        for (int i = 0; i < 10; i++) {
            uint32_t a = input[i] + i;
            uint32_t b = input[i + 10] - i;
            uint32_t c = a * b;
            uint32_t d = (a + b) & 0xFFFF;
            uint32_t e = (a - b) | 0xFF00;
            
            /* Keep many values live */
            result += a + b + c + d + e;
            
            /* More computations */
            for (int j = 0; j < 3; j++) {
                uint32_t f = a << j;
                uint32_t g = b >> j;
                uint32_t h = c & (0xFF << j);
                result += f + g + h;
            }
        }
    } else {
        result = high_pressure_computation(input);
    }
    
    return result;
}

int main() {
    /* Initialize input data */
    uint32_t input_data[NUM_VARS];
    
    /* Fill with pseudo-random but deterministic values */
    for (int i = 0; i < NUM_VARS; i++) {
        input_data[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Perform computation with high register pressure */
    uint64_t result1 = high_pressure_computation(input_data);
    
    /* Another call with different selector */
    uint64_t result2 = nested_high_pressure(input_data, 0);
    uint64_t result3 = nested_high_pressure(input_data, 1);
    
    /* Combine results to ensure all computations are used */
    uint64_t final_result = result1 + result2 + result3;
    
    printf("Result: %llu\n", (unsigned long long)final_result);
    
    /* Also print intermediate results to prevent dead code elimination */
    printf("Intermediate: %llu %llu %llu\n", 
           (unsigned long long)result1,
           (unsigned long long)result2,
           (unsigned long long)result3);
    
    return 0;
}
