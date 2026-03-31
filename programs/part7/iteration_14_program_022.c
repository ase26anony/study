/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Create register pressure */
#define CREATE_PRESSURE(i) \
    do { \
        int a = (i); \
        int b = (i) * 2; \
        int c = b - a; \
        int d = (a * b) >> (c & 3); \
        asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d)); \
    } while(0)

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE int test_perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < N; ++i) {
        /* No code here ensures loop's blocks are exactly outer header + inner loop */
        
        /* Inner loop - this will be 'other' (subset of loop's blocks) */
        for (int j = 0; j < 5; ++j) {
            CREATE_PRESSURE(j);
            result ^= (i * j) & 0xFF;
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result;
}

/* Function 2: loop is subset of other */
NOINLINE int test_loop_subset_of_other(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            CREATE_PRESSURE(j);
            result += j * 7;
        }
        
        /* Second inner loop - this will be 'loop' (subset of other's blocks) */
        for (int k = 0; k < 4; ++k) {
            CREATE_PRESSURE(k);
            result ^= (i * k) & 0xFF;
        }
    }
    
    return result;
}

/* Function 3: Partial overlap with goto */
NOINLINE int test_partial_overlap(int N) {
    int result = 0;
    
    /* First loop - will be 'loop' */
    for (int i = 0; i < N; ++i) {
        CREATE_PRESSURE(i);
        result += i * 3;
        
        if (i == N/2) {
            /* Jump target inside loop creates shared block */
shared_block:
            result |= 0x80;
        }
    }
    
    /* Second loop - will be 'other' */
    for (int j = 0; j < N; ++j) {
        CREATE_PRESSURE(j);
        result ^= j * 5;
        
        if (j == N/3) {
            /* goto into first loop creates intersection */
            goto shared_block;
        }
    }
    
    return result;
}

/* Function 4: Mixed loop types with complex nesting */
NOINLINE int test_mixed_loops(int N) {
    int result = 0;
    
    /* while loop */
    int w = 0;
    while (w < N) {
        /* do-while nested inside while */
        int d = 0;
        do {
            CREATE_PRESSURE(d);
            result += (w * d) & 0xFF;
            d++;
        } while (d < 3);
        
        /* for loop after do-while */
        for (int i = 0; i < 2; i++) {
            CREATE_PRESSURE(i);
            result ^= (w + i) * 11;
        }
        
        w++;
    }
    
    /* Another for loop that shares some blocks via break */
    for (int i = 0; i < N; i++) {
        CREATE_PRESSURE(i);
        if (result > 1000) {
            /* This break creates control flow to a block outside */
            break;
        }
        result += i * 13;
    }
    
    return result;
}

/* Function 5: Sibling loops with shared exit block */
NOINLINE int test_sibling_loops(int N) {
    int result = 0;
    
    /* Two sequential loops that might share exit blocks */
    for (int i = 0; i < N; ++i) {
        CREATE_PRESSURE(i);
        result += i * 17;
    }
    
    for (int j = 0; j < N; ++j) {
        CREATE_PRESSURE(j);
        result ^= j * 19;
    }
    
    /* A third loop that contains both via goto */
    int k = 0;
loop_start:
    if (k < N) {
        CREATE_PRESSURE(k);
        result += k * 23;
        k++;
        
        if (k == N/2) {
            goto loop_exit;
        }
        goto loop_start;
    }
loop_exit:
    
    return result;
}

/* Main function with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 20) + 25;
    int N5 = (seed % 10) + 30;
    
    int total = 0;
    
    total += test_perfect_nesting(N1);
    total += test_loop_subset_of_other(N2);
    total += test_partial_overlap(N3);
    total += test_mixed_loops(N4);
    total += test_sibling_loops(N5);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
