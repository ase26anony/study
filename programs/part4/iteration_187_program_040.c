/* test_sel_sched_dump.c
 * 
 * This test is designed to trigger GCC's selective scheduling debug output
 * when compiled with flags like -fsel-sched-dump or -fdump-rtl-sched*.
 * The uncovered lines in sel-sched-dump.cc are in a debug printing function
 * that dumps scheduled instructions. To hit them, we need code that:
 * 1. Creates complex scheduling decisions for the selective scheduler
 * 2. Has loops with data dependencies and control flow
 * 3. Uses volatile/asm to prevent over-optimization
 * 4. Is compiled with selective scheduling enabled and debug dumps requested
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile sink to prevent dead code elimination */
volatile int global_sink;

/* Function with complex loop that should engage the selective scheduler */
unsigned int complex_hash(unsigned int seed, int iterations) {
    unsigned int x = seed;
    volatile int barrier; /* Prevent reordering */
    
    for (int i = 0; i < iterations; i++) {
        /* Create loop-carried dependency */
        x = x * 1103515245 + 12345;
        
        /* Mix of arithmetic and bitwise operations */
        x = (x ^ (x >> 16)) * 0x45d9f3b;
        x = (x ^ (x >> 16)) * 0x45d9f3b;
        x = x ^ (x >> 16);
        
        /* Control flow inside loop - creates scheduling challenges */
        if (x % 3 == 0) {
            x = x + (x << 3);  /* Multiply by 9 */
        } else if (x % 5 == 0) {
            x = x ^ 0xDEADBEEF;
        } else {
            x = x | 0x55555555;
        }
        
        /* More operations with different data types */
        unsigned long y = (unsigned long)x * 0xABCDEF;
        x = (unsigned int)(y ^ (y >> 32));
        
        /* Artificial memory barrier using inline asm */
        asm volatile("" ::: "memory");
        
        /* Volatile read to create memory dependency */
        barrier = global_sink;
        x += barrier;
        
        /* Nested conditional with break possibility */
        if (i > 100 && (x & 0xFF) == 0) {
            x >>= 8;
            if (x < 1000) break;
        }
    }
    
    return x;
}

/* Second function with different pattern to increase scheduler activity */
int compute_checksum(const char* data, int len) {
    int sum = 0;
    int prod = 1;
    
    for (int i = 0; i < len; i++) {
        /* Multiple interdependent operations */
        int val = data[i];
        sum = sum + val;
        prod = prod * (val | 1);  /* OR with 1 to avoid zero */
        
        /* Complex conditional with modulo */
        if (i % 4 == 0) {
            sum = sum ^ prod;
        } else if (i % 4 == 1) {
            prod = prod + (sum << 2);
        } else if (i % 4 == 2) {
            sum = sum - (prod >> 1);
        } else {
            prod = prod ^ sum;
        }
        
        /* Switch statement for additional control flow complexity */
        switch (val % 7) {
            case 0: sum += 1; break;
            case 1: sum -= 2; break;
            case 2: sum ^= 3; break;
            case 3: sum |= 4; break;
            case 4: sum &= 0xF; break;
            case 5: sum = ~sum; break;
            default: sum = sum * 2;
        }
    }
    
    /* Final mixing */
    return (sum << 16) | (prod & 0xFFFF);
}

/* Main driver that provides varying inputs */
int main(int argc, char** argv) {
    unsigned int hash_result = 0;
    int checksum_result = 0;
    
    /* Use argument or fixed value to prevent compile-time computation */
    int iterations = (argc > 1) ? atoi(argv[1]) : 1000;
    if (iterations < 100) iterations = 100;
    
    /* Call complex functions multiple times with different seeds */
    for (int j = 0; j < 5; j++) {
        hash_result ^= complex_hash(hash_result + j * 100, iterations + j * 50);
        
        /* Create some input data for checksum */
        char data[256];
        for (int k = 0; k < 256; k++) {
            data[k] = (hash_result + k) & 0xFF;
        }
        
        checksum_result += compute_checksum(data, 256);
        
        /* Update volatile to create cross-iteration dependency */
        global_sink = hash_result & 0xFF;
    }
    
    /* Use results to prevent elimination */
    printf("Final hash: %u, checksum: %d\n", hash_result, checksum_result);
    
    return (hash_result + checksum_result) & 0xFF;
}
