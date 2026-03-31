/* test_sel_sched_dump.c
 * 
 * This test is designed to trigger GCC's selective scheduling debug output
 * machinery, specifically the code in sel-sched-dump.cc that prints scheduled
 * instructions. The goal is to cover the lines that switch dump output,
 * print the RTL representation of an instruction, and restore the dump state.
 *
 * Compile this with an instrumented GCC build using flags like:
 *   gcc -O3 -fselective-scheduling2 -fsel-sched-dump -c test_sel_sched_dump.c -o test.o
 *   gcc -O3 -fselective-scheduling2 -fdump-rtl-sched1 -fdump-rtl-sched2 -c test_sel_sched_dump.c
 *
 * The actual execution of the resulting binary is less important than the
 * compilation process itself, which should cause the instrumented GCC to
 * execute the debug dumping code paths.
 */

#include <stdio.h>
#include <stdlib.h>

/* A non-trivial function with complex control flow and data dependencies
 * to give the selective scheduler plenty of work and reasons to log decisions.
 */
unsigned int complex_loop(unsigned int seed, int iterations) {
    volatile unsigned int sink; /* Prevent dead code elimination */
    unsigned int a = seed;
    unsigned int b = 0x9e3779b9; /* Golden ratio constant */
    unsigned int c = 0;
    unsigned int d = 1;
    int i;
    
    /* Loop with data-dependent branching and mixed operations */
    for (i = 0; i < iterations; ++i) {
        /* Multiple arithmetic operations creating dependencies */
        a = a * 1664525 + 1013904223;
        b = (b >> 13) | (b << 19); /* Rotate right */
        c = a ^ b;
        
        /* Conditional operations based on computed values */
        if (c % 3 == 0) {
            d = d + (c & 0xFF);
            /* Bitwise operations */
            d = d ^ (a << 3);
        } else if (c % 5 == 0) {
            d = d - (b % 256);
            d = d | 0xAAAAAAAA;
        } else {
            d = d * 3;
            d = d & 0x55555555;
        }
        
        /* More operations to increase instruction count */
        if (i % 7 == 0) {
            /* Use division/modulo (expensive operations) */
            d = d / 2;
            c = c % 1000;
        }
        
        /* Artificial memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
        
        /* Nested conditional with bit manipulation */
        if (d > 0x80000000) {
            d = d >> 4;
            b = b + d;
        } else {
            d = d << 2;
            a = a - d;
        }
        
        /* Loop-carried dependency */
        sink = d; /* volatile write ensures side effect */
    }
    
    /* Mix all values to produce a result */
    return a ^ b ^ c ^ d ^ sink;
}

/* Another function with different patterns to increase scheduling complexity */
int branching_pattern(int x, int y) {
    int result = 0;
    int i, j;
    
    /* Nested loops with breaks */
    for (i = 0; i < x; ++i) {
        for (j = 0; j < y; ++j) {
            result += i * j;
            
            /* Conditional break */
            if (result > 1000000) {
                result = result / 2;
                break;
            }
            
            /* Switch statement for control flow variety */
            switch ((i + j) % 4) {
                case 0: result = result << 1; break;
                case 1: result = result >> 1; break;
                case 2: result = result ^ 0x12345678; break;
                case 3: result = result | 0x87654321; break;
            }
        }
        
        /* Early exit based on computation */
        if (result < 0) {
            result = -result;
            if (i > x/2) break;
        }
    }
    
    return result;
}

/* Main driver that uses input to prevent compile-time computation */
int main(int argc, char **argv) {
    unsigned int seed;
    int iterations, x, y;
    
    /* Use command line arguments to make values unknown at compile time */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    if (argc > 2) {
        iterations = atoi(argv[2]);
    } else {
        iterations = 1000;
    }
    
    if (argc > 3) {
        x = atoi(argv[3]);
        y = atoi(argv[4]);
    } else {
        x = 50;
        y = 50;
    }
    
    /* Call the complex functions and combine results */
    unsigned int r1 = complex_loop(seed, iterations);
    int r2 = branching_pattern(x, y);
    
    /* Final result printed to ensure all code is live */
    printf("Result: %u %d\n", r1, r2);
    
    return 0;
}
