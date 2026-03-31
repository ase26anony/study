/* test_sel_sched_dump.c
 * 
 * This test is designed to trigger selective scheduling debug dumps
 * in GCC's sel-sched-dump.cc when compiled with appropriate flags.
 * The uncovered lines are in a debug printing function that dumps
 * scheduled instructions. To hit them, we need code that causes the
 * selective scheduler to make non-trivial decisions and produce debug output.
 *
 * Compile with (using an instrumented GCC build):
 *   gcc -O3 -fselective-scheduling2 -fsel-sched-dump -c test_sel_sched_dump.c -o test.o
 * or:
 *   gcc -O2 -fselective-scheduling -fsel-sched-dump -fdump-rtl-sched1 -c test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

/* A volatile sink to prevent dead code elimination */
volatile int global_sink;

/* Function with complex loop to engage the instruction scheduler */
unsigned int complex_loop(unsigned int seed, int iterations) {
    unsigned int state = seed;
    int i, j;
    
    /* Outer loop with data-dependent trip count */
    for (i = 0; i < iterations; i++) {
        unsigned int temp = state;
        
        /* Multiple arithmetic operations creating dependencies */
        state = state * 1103515245 + 12345;
        state = (state >> 16) & 0x7FFF;
        
        /* Bitwise operations mixed with arithmetic */
        state ^= (temp << 7) & 0x9D2C5680;
        state ^= (temp >> 15) & 0xEFC60000;
        
        /* Conditional operations based on state */
        if (state % 3 == 0) {
            state = state + (temp & 0xFF);
        } else if (state % 5 == 0) {
            state = state - (temp | 0x7F);
        } else {
            state = state ^ 0xDEADBEEF;
        }
        
        /* Nested loop with small, variable iteration count */
        int inner_loop = (state & 0x3) + 1; /* 1-4 iterations */
        for (j = 0; j < inner_loop; j++) {
            /* Mixed operations including division (expensive) */
            state = (state + j) * 1664525;
            state = state % 0xFFFFFFFF;
            
            /* Inline asm to create memory barrier and prevent reordering */
            asm volatile("" ::: "memory");
        }
        
        /* More operations with different data types */
        long long big_val = (long long)state * state;
        state = (unsigned int)(big_val >> 16);
        
        /* Another conditional with early exit possibility */
        if (state > 0x7FFFFFFF) {
            state = state >> 1;
            if (i > iterations / 2) {
                /* Sometimes break early */
                break;
            }
        }
    }
    
    return state;
}

/* Another function with switch statement for control flow variety */
int control_flow_test(int x, int y) {
    int result = 0;
    int i;
    
    for (i = 0; i < 100; i++) {
        /* Data-dependent switch */
        switch ((x + i) % 7) {
            case 0:
                result += x * y;
                break;
            case 1:
                result -= x | y;
                break;
            case 2:
                result ^= x & y;
                break;
            case 3:
                result = (result << 3) | (x >> 2);
                break;
            case 4:
                result = (result % 17) + y;
                break;
            case 5:
                /* Nested loop inside switch case */
                for (int k = 0; k < 3; k++) {
                    result += k * x;
                }
                break;
            default:
                result = ~result;
                break;
        }
        
        /* Update variables to create loop-carried dependencies */
        x = (x * 13 + 7) & 0xFF;
        y = (y - 5) | 0x1;
        
        /* Volatile write to prevent optimization */
        global_sink = result;
    }
    
    return result;
}

/* Main function that drives everything */
int main(int argc, char **argv) {
    unsigned int seed = 42;
    int iterations = 1000;
    
    /* Use command line argument to vary input and prevent constant folding */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 10) iterations = 10;
        if (iterations > 10000) iterations = 10000;
    }
    
    printf("Starting with seed=%u, iterations=%d\n", seed, iterations);
    
    /* Call the complex loop function multiple times */
    unsigned int result1 = complex_loop(seed, iterations);
    printf("Result1: %u\n", result1);
    
    unsigned int result2 = complex_loop(result1, iterations / 2);
    printf("Result2: %u\n", result2);
    
    /* Call the control flow function */
    int cf_result = control_flow_test((int)result1, (int)result2);
    printf("Control flow result: %d\n", cf_result);
    
    /* Mix results to produce final output */
    unsigned int final = result1 ^ result2 ^ (unsigned int)cf_result;
    printf("Final result: %u\n", final);
    
    return (int)(final % 256);
}
