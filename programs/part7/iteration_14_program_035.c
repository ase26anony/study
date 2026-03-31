/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * or for generic testing: gcc -O2 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Create register pressure and prevent optimization */
#define KEEP(i) asm volatile("" : : "r"(i))
#define KEEP_VAR(v) asm volatile("" : : "r"(v))

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    volatile int seed = N; /* Prevent constant propagation */
    int limit = (seed % 50) + 10;
    
    /* Outer loop - this will be 'loop' in the analysis */
    for (int i = 0; i < limit; ++i) {
        /* No code here to ensure inner loop blocks are subset of outer */
        
        /* Inner loop - this will be 'other' in the analysis */
        for (int j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = (i << 3) ^ j;
            int c = b - a;
            int d = (a * b) >> (c & 7);
            result ^= d;
            KEEP(a); KEEP(b); KEEP(c); KEEP(d);
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 40) + 15;
    
    /* Outer loop - this will be 'other' in the analysis */
    for (int i = 0; i < limit; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            int a = i * j * 7;
            result += a;
            KEEP(a);
        }
        
        /* Second inner loop - this will be 'loop' in the analysis */
        /* This loop's blocks are subset of outer loop's blocks */
        for (int k = 0; k < i + 2; ++k) {
            int b = (i << 2) | k;
            int c = b * 3;
            int d = c ^ result;
            result = d;
            KEEP(b); KEEP(c); KEEP(d);
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto (Condition 1) */
NOINLINE int overlapping_loops(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 30) + 20;
    
    /* Loop A - will be 'loop' in some analysis */
    for (int i = 0; i < limit; ++i) {
        int a = i * 3;
        
    shared_block:
        /* This label creates shared basic block */
        int b = a ^ i;
        result += b;
        KEEP(a); KEEP(b);
        
        /* Loop B - will be 'other' in some analysis */
        for (int j = 0; j < 5; ++j) {
            int c = (i * j) + result;
            
            /* Jump to shared block in Loop A */
            if (j == 3 && (c & 1)) {
                goto shared_block;
            }
            
            int d = c << 2;
            result ^= d;
            KEEP(c); KEEP(d);
        }
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex control flow */
NOINLINE int mixed_loops(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 25) + 25;
    int i = 0;
    
    /* do-while inside for loop */
    for (i = 0; i < limit; ++i) {
        int j = 0;
        
        /* do-while loop */
        do {
            int a = (i * j) + result;
            int b = a ^ 0x55;
            result = b;
            KEEP(a); KEEP(b);
            j++;
        } while (j < 4);
        
        /* while loop after do-while */
        int k = 0;
        while (k < i) {
            int c = result * k;
            int d = c - i;
            result ^= d;
            KEEP(c); KEEP(d);
            k++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with partial overlap */
NOINLINE int sibling_loops(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 35) + 10;
    
    /* First loop */
    for (int i = 0; i < limit; ++i) {
        int a = i * 11;
        result += a;
        KEEP(a);
        
        if (i & 1) {
            /* Shared code block */
            int shared = result ^ 0xAA;
            result = shared;
            KEEP(shared);
        }
    }
    
    /* Second loop that shares the if-block above */
    for (int j = limit - 1; j >= 0; --j) {
        int b = j * 7;
        result ^= b;
        KEEP(b);
        
        if (j & 2) {
            /* Same shared code block */
            int shared = result ^ 0xAA;
            result = shared;
            KEEP(shared);
        }
    }
    
    return result & 0xFF;
}

/* Main function to drive execution */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int base = argc;
    int iterations = (base % 10) + 5;
    
    /* Call each function multiple times with different parameters */
    for (int i = 0; i < iterations; ++i) {
        total ^= perfect_nesting(i + 10);
        total ^= reverse_nesting(i + 15);
        total ^= overlapping_loops(i + 20);
        total ^= mixed_loops(i + 25);
        total ^= sibling_loops(i + 30);
        
        /* Create some variation in control flow */
        if (i & 1) {
            total += perfect_nesting(i * 2);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
