/* test_sel_sched_dump.c
 * 
 * This test is designed to trigger GCC's selective scheduling debug output
 * machinery, specifically targeting the uncovered lines in sel-sched-dump.cc
 * that dump scheduled instruction details.
 *
 * Compile with instrumented GCC using flags like:
 *   gcc -O3 -fselective-scheduling2 -fsel-sched-dump -c test_sel_sched_dump.c -o test.o
 *   gcc -O3 -fselective-scheduling2 -fdump-rtl-sched1 -fdump-rtl-sched2 -c test_sel_sched_dump.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile sink to prevent dead code elimination */
volatile int global_sink = 0;

/* Function with complex control flow and data dependencies
 * to engage the selective scheduler */
unsigned int complex_loop(unsigned int seed, int iterations) {
    unsigned int a = seed;
    unsigned int b = 0x9e3779b9; /* Golden ratio */
    unsigned int c = 0;
    unsigned int d = 1;
    int i;
    
    /* Loop with multiple operations and control flow */
    for (i = 0; i < iterations; i++) {
        /* Data-dependent branching */
        if (i % 3 == 0) {
            a = (a ^ (a >> 13)) * 0x85ebca6b;
            b = (b + (b << 5)) ^ 0xabcdef12;
        } else if (i % 3 == 1) {
            a = (a + (a << 7)) | 0x12345678;
            b = (b ^ (b >> 11)) * 0x5f356495;
        } else {
            a = (a * 0x9e3779b9) ^ 0xdeadbeef;
            b = (b - (b << 3)) + 0x87654321;
        }
        
        /* More arithmetic operations to increase instruction count */
        c = (c + a) * 0xabcdef01;
        d = (d ^ b) + (c >> 3);
        
        /* Nested conditional with bitwise operations */
        if ((i & 7) == 0) {
            d = d << 1;
        } else if ((i & 7) == 4) {
            d = d >> 1;
        }
        
        /* Loop-carried dependency */
        c = c + d;
        
        /* Artificial memory barrier using inline asm */
        asm volatile("" ::: "memory");
        
        /* Early exit condition based on computed values */
        if ((a & 0xfff) == 0xabc) {
            break;
        }
    }
    
    /* Mix all values */
    return a ^ b ^ c ^ d;
}

/* Another function with different pattern to increase scheduling complexity */
int hash_array(const int* arr, int size) {
    int hash = 0x5a5a5a5a;
    int i, j;
    
    for (i = 0; i < size; i++) {
        /* Multiple operations per iteration */
        hash = hash ^ arr[i];
        hash = hash * 16777619;
        
        /* Small inner loop */
        for (j = 0; j < 3; j++) {
            hash = (hash << (j + 1)) | (hash >> (32 - (j + 1)));
            hash = hash + (arr[i] * j);
        }
        
        /* Switch statement for control flow variety */
        switch (arr[i] % 4) {
            case 0:
                hash = hash & 0x00ffffff;
                break;
            case 1:
                hash = hash | 0xff000000;
                break;
            case 2:
                hash = hash ^ 0x12345678;
                break;
            case 3:
                hash = ~hash;
                break;
        }
        
        /* Division/modulo operations (expensive, scheduler relevant) */
        if (hash != 0) {
            hash = hash / ((arr[i] & 0x1f) + 1);
        }
        hash = hash % 0x7fffffff;
    }
    
    return hash;
}

/* Main driver that creates sufficient work for the scheduler */
int main(int argc, char** argv) {
    int i;
    unsigned int result = 0;
    int data[50];
    
    /* Initialize with pseudo-random but compiler-unpredictable values */
    for (i = 0; i < 50; i++) {
        data[i] = (i * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Call complex functions multiple times with different parameters */
    for (i = 0; i < 100; i++) {
        result ^= complex_loop(result + i, 50 + (i % 10));
        
        /* Feed result into array hash */
        data[i % 50] ^= result;
        result += hash_array(data, 20 + (i % 30));
        
        /* Use volatile write to prevent optimization */
        global_sink = result & 1;
    }
    
    /* Use input to prevent compile-time computation */
    if (argc > 1) {
        result += atoi(argv[1]);
    }
    
    printf("Result: %u\n", result);
    return (int)(result & 0x7fffffff);
}
