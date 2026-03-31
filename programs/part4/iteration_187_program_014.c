/* test_sel_sched.c - Test case for GCC selective scheduling debug output */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and multiple operations */
unsigned long complex_loop(unsigned int seed, int iterations) {
    volatile unsigned int v_seed = seed; /* Prevent optimization */
    unsigned long result = 0;
    int i, j;
    
    /* Outer loop with loop-carried dependency */
    for (i = 0; i < iterations; i++) {
        unsigned int val = v_seed;
        
        /* Inner loop with mixed operations */
        for (j = 0; j < 8; j++) {
            /* Data-dependent branching */
            if (val & 1) {
                /* Multiple arithmetic operations */
                val = (val * 1103515245U + 12345U) & 0x7fffffffU;
                result += val;
                val = val ^ (val >> 16);
                val = val * 1664525U + 1013904223U;
            } else {
                /* Bitwise operations */
                val = (val << 13) | (val >> 19);
                val = val ^ (val * 16807U);
                result -= val & 0xff;
            }
            
            /* More operations to increase instruction count */
            val = (val + i) * (j + 1);
            val = val % 1000007U;
            
            /* Conditional operation based on loop index */
            switch (j % 4) {
                case 0: val = val | 0x55555555U; break;
                case 1: val = val & 0x33333333U; break;
                case 2: val = val ^ 0x0f0f0f0fU; break;
                case 3: val = ~val; break;
            }
            
            /* Artificial memory barrier */
            asm volatile("" : : : "memory");
        }
        
        /* Break condition based on computed value */
        if (val > 0x70000000U) {
            v_seed = val >> 1;
        } else if (val < 0x10000000U) {
            v_seed = val << 1;
        } else {
            v_seed = val;
        }
        
        /* Early exit condition */
        if (result > 0x1000000000000ULL) {
            result = result >> 4;
            break;
        }
    }
    
    return result;
}

/* Another function with different pattern */
int hash_computation(const char* data, int len) {
    volatile int hash = 5381;
    int i;
    
    for (i = 0; i < len; i++) {
        /* Complex hash computation */
        hash = ((hash << 5) + hash) + data[i];
        
        /* Conditional operations */
        if (i % 3 == 0) {
            hash = hash ^ (hash >> 3);
            hash = hash * 16777619U;
        } else if (i % 3 == 1) {
            hash = hash + (hash << 10);
            hash = hash ^ (hash >> 6);
        } else {
            hash = hash - (hash << 7);
            hash = hash ^ (hash << 17);
        }
        
        /* Division/modulo operations are expensive to schedule */
        if (hash % 7 == 0) {
            hash = hash / 3;
        } else if (hash % 5 == 0) {
            hash = hash % 100003;
        }
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r" (hash) : : "memory");
    }
    
    return hash;
}

/* Main function that calls both computational functions */
int main(int argc, char** argv) {
    unsigned int seed;
    int iterations;
    
    /* Get input to prevent compile-time computation */
    if (argc > 1) {
        seed = atoi(argv[1]);
        iterations = atoi(argv[2]);
    } else {
        printf("Usage: %s <seed> <iterations>\n", argv[0]);
        seed = 12345;
        iterations = 1000;
    }
    
    /* Call complex loop function */
    unsigned long result1 = complex_loop(seed, iterations);
    printf("Result1: %lu\n", result1);
    
    /* Call hash function */
    const char test_data[] = "This is test data for hash computation with varying length";
    int result2 = hash_computation(test_data, sizeof(test_data));
    printf("Result2: %d\n", result2);
    
    /* Mix results to prevent dead code elimination */
    volatile unsigned long final_result = result1 + result2;
    printf("Final: %lu\n", final_result);
    
    return (int)(final_result % 256);
}
