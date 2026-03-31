/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * or: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage -march=native
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Create register pressure */
#define PRESSURE(x) \
    do { \
        int a = (x), b = (x)*2, c = b - a; \
        asm volatile("" : : "r"(a), "r"(b), "r"(c)); \
    } while(0)

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE int test_perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < N; ++i) {
        /* Inner loop - this will be 'other' (subset of outer) */
        for (int j = 0; j < (N - i); ++j) {
            result ^= i * j;
            PRESSURE(j);
        }
        /* No code here ensures inner loop is perfect subset */
    }
    
    return result;
}

/* Function 2: Loop is subset of other */
NOINLINE int test_loop_subset_of_other(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            result += j * i;
            PRESSURE(j);
        }
        
        /* Second inner loop - this will be 'loop' (subset of outer) */
        for (int k = 0; k < (N - i); ++k) {
            result ^= k * i;
            PRESSURE(k);
        }
    }
    
    return result;
}

/* Function 3: Partially overlapping loops with goto */
NOINLINE int test_partial_overlap(int N) {
    int result = 0;
    
    /* First loop - will be 'loop' */
    for (int i = 0; i < N; ++i) {
    loop_body:
        result += i * 2;
        PRESSURE(i);
        
        /* Second loop - will be 'other' */
        for (int j = 0; j < 5; ++j) {
            result ^= i * j;
            PRESSURE(j);
            
            if (j == 3 && i < N/2) {
                /* Jump into first loop's body, creating intersection */
                goto loop_body;
            }
        }
    }
    
    return result;
}

/* Function 4: Mixed loop types with complex nesting */
NOINLINE int test_mixed_loops(int N) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < N) {
        /* do-while nested inside while */
        int j = 0;
        do {
            result += i * j;
            PRESSURE(j);
            j++;
        } while (j < 5);
        
        /* for loop after do-while */
        for (int k = 0; k < (N - i); ++k) {
            result ^= k * i;
            PRESSURE(k);
            
            /* Another nested loop creating subset relationship */
            for (int m = 0; m < 2; ++m) {
                result += m * k;
                PRESSURE(m);
            }
        }
        
        i++;
    }
    
    return result;
}

/* Function 5: Sibling loops with break to shared label */
NOINLINE int test_sibling_loops(int N) {
    int result = 0;
    
    /* First loop */
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 10; ++j) {
            result += i * j;
            PRESSURE(j);
            
            if (j == 5) {
                /* Break to label shared with second loop */
                goto shared_block;
            }
        }
        
        /* Code here creates blocks in first loop not in second */
        result -= i;
        PRESSURE(i);
        
    shared_block:
        /* This block will be in both loops' bitmaps */
        result |= 1;
        
        /* Second loop that shares the label */
        for (int k = 0; k < 8; ++k) {
            result ^= k * i;
            PRESSURE(k);
        }
    }
    
    return result;
}

/* Function 6: Complex with switch inside loop */
NOINLINE int test_switch_in_loop(int N) {
    int result = 0;
    
    for (int i = 0; i < N; ++i) {
        switch (i % 4) {
            case 0:
                for (int j = 0; j < 3; ++j) {
                    result += j;
                    PRESSURE(j);
                }
                break;
            case 1:
                /* Empty case creates different block structure */
                break;
            case 2:
                for (int k = 0; k < 2; ++k) {
                    result ^= k * i;
                    PRESSURE(k);
                }
                /* Fall through */
            case 3:
                result += i;
                PRESSURE(i);
                break;
        }
        
        /* Another loop creating subset relationship */
        for (int m = 0; m < (i % 3 + 1); ++m) {
            result -= m * 2;
            PRESSURE(m);
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    int total = 0;
    
    /* Call all test functions with varying parameters */
    total ^= test_perfect_nesting(N);
    total ^= test_loop_subset_of_other(N + 5);
    total ^= test_partial_overlap(N / 2 + 1);
    total ^= test_mixed_loops(N % 20 + 5);
    total ^= test_sibling_loops(N % 15 + 8);
    total ^= test_switch_in_loop(N % 12 + 6);
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", total & 255);
    
    return 0;
}
