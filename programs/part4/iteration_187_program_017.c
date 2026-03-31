/* test_sel_sched.c - Test case for GCC selective scheduling debug output */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and mixed operations */
unsigned long complex_loop(unsigned int seed, int iterations) {
    volatile unsigned long result = 0;  /* Prevent dead code elimination */
    unsigned long a = seed * 1103515245UL + 12345;
    unsigned long b = seed * 1664525UL + 1013904223UL;
    int i, j;
    
    /* Outer loop with loop-carried dependency */
    for (i = 0; i < iterations; i++) {
        /* Inner loop with data-dependent operations */
        for (j = 0; j < 8; j++) {
            /* Complex data-dependent branching */
            if ((a & 0xFF) % 3 == 0) {
                result += (a * b) & 0xFFFF;
                a = (a << 3) | (a >> 29);  /* Rotate left */
            } else if ((a & 0xFF) % 5 == 0) {
                result ^= (b % 1024) * (i + 1);
                b = (b * 1103515245UL + 12345) ^ result;
            } else {
                result |= (a + b) % 256;
                a = a ^ b;
                b = b + 1;
            }
            
            /* Mixed arithmetic operations creating dependencies */
            unsigned long temp = result;
            temp = temp * 6364136223846793005UL;
            temp = temp + (temp >> 32);
            temp = temp ^ (temp << 21);
            temp = temp ^ (temp >> 35);
            temp = temp ^ (temp << 4);
            
            /* More operations with different data types */
            int small_temp = (int)(temp & 0x7FFFFFFF);
            if (small_temp % 7 == 0) {
                result += small_temp / 7;
            } else if (small_temp % 11 == 0) {
                result -= small_temp / 11;
            } else {
                result ^= small_temp;
            }
            
            /* Memory barrier via inline asm to prevent reordering */
            asm volatile("" ::: "memory");
            
            /* Bitwise operations with shifting */
            b = (b << 1) | (b >> 31);
            a = (a >> 1) ^ (a << 2);
        }
        
        /* Conditional break based on computed value */
        if ((result & 0xFFF) == 0xABC) {
            break;
        }
        
        /* Switch statement for additional control flow complexity */
        switch (i % 4) {
            case 0:
                result = result * 3 + 1;
                break;
            case 1:
                result = result ^ 0x5A5A5A5A5A5A5A5AUL;
                break;
            case 2:
                result = result - (result >> 2);
                break;
            case 3:
                result = result | (result << 16);
                break;
        }
    }
    
    /* Final mixing */
    result = result ^ (result >> 33);
    result = result * 0xff51afd7ed558ccdUL;
    result = result ^ (result >> 33);
    result = result * 0xc4ceb9fe1a85ec53UL;
    result = result ^ (result >> 33);
    
    return result;
}

/* Another function with different pattern to ensure scheduler activity */
int hash_computation(char *data, int len) {
    volatile int hash = 5381;  /* djb2 hash seed */
    int i;
    
    for (i = 0; i < len; i++) {
        /* Data-dependent operation */
        hash = ((hash << 5) + hash) + data[i];
        
        /* Additional arithmetic to create more instructions */
        if (hash % 2 == 0) {
            hash = hash ^ 0xDEADBEEF;
            hash = hash * 16777619;  /* FNV prime */
        } else {
            hash = hash & 0x7FFFFFFF;
            hash = hash | (i << 24);
        }
        
        /* Division/modulo operations are expensive and create dependencies */
        if (i % 3 == 0) {
            hash = hash / 3;
        } else if (i % 7 == 0) {
            hash = hash % 1023;
        }
        
        /* More bitwise operations */
        hash = (hash << 1) | (hash >> 31);
        hash = hash ^ (hash >> 16);
    }
    
    return hash;
}

/* Main function that calls the complex functions */
int main(int argc, char **argv) {
    unsigned long final_result = 0;
    int i;
    
    /* Use command line argument or default for variability */
    unsigned int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call complex function multiple times with different parameters */
    for (i = 0; i < 3; i++) {
        unsigned long r = complex_loop(seed + i, 50 + i * 10);
        final_result ^= r;
        printf("Iteration %d: %lu\n", i, r);
    }
    
    /* Process some data */
    char test_data[] = "This is test data for the hash computation function";
    int hash = hash_computation(test_data, sizeof(test_data) - 1);
    final_result += hash;
    
    /* Use volatile to ensure result is used */
    volatile unsigned long sink = final_result;
    
    printf("Final result: %lu\n", (unsigned long)sink);
    return (int)(final_result & 0x7FFFFFFF);
}
