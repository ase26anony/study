/* test_hwloop.c
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a test_hwloop.c -o test_hwloop
 * Or for generic targets: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage test_hwloop.c -o test_hwloop
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
    volatile int seed = N;
    int limit = (seed % 50) + 10;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (int i = 0; i < limit; ++i) {
        /* No code here to ensure inner loop blocks are subset */
        
        /* Inner loop (will be 'other' in hierarchy) */
        for (int j = 0; j < (i % 5) + 1; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = i * j;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            result ^= d;
            
            KEEP(a); KEEP(b); KEEP(c); KEEP(d);
            KEEP_VAR(result);
        }
        
        /* No code here either - ensures inner loop is perfect subset */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop is subset of other (Condition 3) */
NOINLINE int loop_subset_of_other(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 40) + 15;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (int i = 0; i < limit; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 2; ++j) {
            int a = i * j;
            int b = a + 12345;
            result += b;
            KEEP(a); KEEP(b);
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (int k = 0; k < (i % 3) + 1; ++k) {
            /* This loop's blocks are subset of outer loop */
            int x = i * k;
            int y = x ^ result;
            int z = y >> 2;
            result ^= z;
            
            KEEP(x); KEEP(y); KEEP(z);
        }
        
        /* More code in outer loop after inner loops */
        result += i * 7;
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto (Condition 1) */
NOINLINE int overlapping_with_goto(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 30) + 20;
    
    /* Loop A */
    for (int i = 0; i < limit; ++i) {
        int a = i * 3;
        
    shared_block:
        /* This block will be shared between loops via goto */
        int b = a + result;
        result ^= b;
        KEEP(a); KEEP(b);
        
        /* Loop B - shares block via goto */
        for (int j = 0; j < 3; ++j) {
            if (j == 1 && (result & 1)) {
                /* Jump into Loop A's body */
                goto shared_block;
            }
            
            int c = i * j;
            result += c;
            KEEP(c);
        }
        
        /* Do-while loop nested inside for loop */
        int counter = 0;
        do {
            int d = result * counter;
            result ^= d;
            KEEP(d);
            counter++;
        } while (counter < 2);
    }
    
    return result & 0xFF;
}

/* Function 4: Complex mixed loop types */
NOINLINE int mixed_loop_types(int N) {
    int result = N;
    volatile int seed = N;
    int limit = (seed % 25) + 25;
    
    /* While loop */
    int w = 0;
    while (w < 5) {
        /* For loop inside while */
        for (int i = 0; i < limit; i += 2) {
            /* Multiple operations for register pressure */
            int a = i * w;
            int b = a ^ result;
            int c = b << (w & 3);
            int d = c - a;
            int e = d * w;
            
            result += e;
            
            KEEP(a); KEEP(b); KEEP(c); KEEP(d); KEEP(e);
            KEEP_VAR(result);
        }
        
        w++;
        
        /* Another for loop as sibling */
        for (int j = 0; j < 3; ++j) {
            result ^= (j * w);
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Adjacent loops with break to shared label */
NOINLINE int adjacent_loops_with_break(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 35) + 10;
    
    /* First loop */
    for (int i = 0; i < limit; ++i) {
        if (i == limit/2) {
            break;
        }
        
        int a = i * 7;
        result += a;
        KEEP(a);
    }
    
shared_label:
    /* Shared block label */
    int shared = result * 11;
    result ^= shared;
    KEEP(shared);
    
    /* Second loop that can break to shared label */
    for (int j = 0; j < limit; ++j) {
        if (result & (1 << (j & 3))) {
            goto shared_label;
        }
        
        int b = j * 13;
        result -= b;
        KEEP(b);
        
        /* Nested while loop */
        int k = 0;
        while (k < 2) {
            result ^= (k * j);
            k++;
        }
    }
    
    return result & 0xFF;
}

/* Main function to drive all test cases */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int base = argc;
    int N1 = (base % 100) + 50;
    int N2 = (base % 80) + 60;
    int N3 = (base % 70) + 40;
    int N4 = (base % 90) + 30;
    int N5 = (base % 60) + 70;
    
    /* Call all functions multiple times with different parameters */
    for (int iteration = 0; iteration < 3; ++iteration) {
        total ^= perfect_nesting(N1 + iteration);
        total += loop_subset_of_other(N2 + iteration);
        total ^= overlapping_with_goto(N3 + iteration);
        total -= mixed_loop_types(N4 + iteration);
        total ^= adjacent_loops_with_break(N5 + iteration);
        
        KEEP_VAR(total);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
