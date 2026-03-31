/* test_hwloop.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates complex loop structures with specific block relationships
 * to trigger bitmap intersection logic in hw-doloop.cc:
 * 1. Loops with intersecting block bitmaps
 * 2. Perfectly nested loops (subset relationship)
 * 3. Sibling loops with partial overlap
 * 4. Complex control flow with gotos
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Create register pressure */
#define CREATE_PRESSURE(i) \
    do { \
        int a = (i); \
        int b = (i) * 2; \
        int c = b - a; \
        int d = (a * b) >> (c & 3); \
        asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d)); \
    } while(0)

/* Function 1: Perfectly nested loops - other is subset of loop */
NOINLINE int test_perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' in the hierarchy */
    for (int i = 0; i < N; ++i) {
        /* Inner loop - this will be 'other' that is subset of loop */
        for (int j = 0; j < (N - i); ++j) {
            CREATE_PRESSURE(j);
            result ^= (i * j) & 0xFF;
        }
        /* No code here ensures loop has no blocks outside other */
    }
    
    return result;
}

/* Function 2: Loop with sibling inner loops - loop is subset of other */
NOINLINE int test_sibling_loops(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'other' in the hierarchy */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in other not in loop */
        for (int j = 0; j < 5; ++j) {
            CREATE_PRESSURE(j);
            result += j * 3;
        }
        
        /* Second inner loop - this will be 'loop' that is subset of other */
        for (int k = 0; k < (N - i); ++k) {
            CREATE_PRESSURE(k);
            result ^= (i * k) & 0xFF;
        }
    }
    
    return result;
}

/* Function 3: Loops with goto creating intersection but not subset */
NOINLINE int test_goto_intersection(int N) {
    int result = 0;
    int counter = 0;
    
    /* Loop A - will be 'loop' */
    for (int i = 0; i < N; ++i) {
        CREATE_PRESSURE(i);
        result += i;
        
        /* Loop B - will be 'other' */
        for (int j = 0; j < 3; ++j) {
            CREATE_PRESSURE(j);
            counter++;
            
            /* Goto into loop A's body creates intersection */
            if (counter == 2 && i < N/2) {
                goto shared_block;
            }
        }
        
        /* This label creates a shared basic block */
        shared_block:
        result ^= counter;
    }
    
    return result;
}

/* Function 4: Mixed loop types with complex control flow */
NOINLINE int test_mixed_loops(int N) {
    int result = 0;
    int i = 0;
    
    /* do-while inside for */
    for (i = 0; i < N; ++i) {
        int j = 0;
        do {
            CREATE_PRESSURE(j);
            result += (i * j) & 0xFF;
            j++;
        } while (j < 5);
        
        /* while loop after do-while */
        int k = 0;
        while (k < 3) {
            CREATE_PRESSURE(k);
            result ^= k;
            k++;
        }
    }
    
    /* Additional while loop that overlaps with previous loops via control flow */
    i = 0;
    while (i < N/2) {
        CREATE_PRESSURE(i);
        /* Break to create intersection with for loop */
        if (result > 1000) {
            break;
        }
        result += i * 2;
        i++;
    }
    
    return result;
}

/* Function 5: Complex nested structure with multiple exits */
NOINLINE int test_complex_nesting(int N) {
    int result = 0;
    
    /* Three-level nesting */
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < (N - i); ++j) {
            /* Innermost loop with early exit */
            for (int k = 0; k < 10; ++k) {
                CREATE_PRESSURE(k);
                result += (i + j + k) & 0xFF;
                
                /* Early exit creates different block relationships */
                if (result > 10000) {
                    goto outer_continue;
                }
            }
            
            /* Code between inner loops creates blocks in middle loop
               that aren't in innermost loop */
            if (j % 2 == 0) {
                result ^= 0x55;
            }
        }
        outer_continue:
        ;
    }
    
    return result;
}

/* Main function with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    int total = 0;
    
    /* Use volatile to prevent optimization */
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 25) + 25;
    int N5 = (seed % 20) + 30;
    
    /* Call all test functions to ensure they're compiled and executed */
    total ^= test_perfect_nesting(N1);
    total ^= test_sibling_loops(N2);
    total ^= test_goto_intersection(N3);
    total ^= test_mixed_loops(N4);
    total ^= test_complex_nesting(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
