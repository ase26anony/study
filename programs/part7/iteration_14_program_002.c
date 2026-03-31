/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * For generic testing: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Create register pressure with complex operations */
#define CREATE_PRESSURE(i) \
    do { \
        int a = (i) * 3; \
        int b = (i) << 2; \
        int c = a ^ b; \
        int d = c * 7; \
        int e = d - a + b; \
        asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e)); \
    } while(0)

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure other is subset */
        
        /* Inner loop - this will be 'other' (subset of loop) */
        for (int j = 0; j < (N - i); ++j) {
            CREATE_PRESSURE(j);
            result ^= (i * j) & 0xFF;
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result;
}

/* Function 2: Loop is subset of other (Condition 3) */
NOINLINE int loop_subset_of_other(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in other not in loop */
        for (int j = 0; j < 3; ++j) {
            CREATE_PRESSURE(j);
            result += j * 2;
        }
        
        /* Second inner loop - this will be 'loop' (subset of other) */
        for (int k = 0; k < (N / 2); ++k) {
            CREATE_PRESSURE(k);
            result ^= (i * k) & 0xFF;
        }
    }
    
    return result;
}

/* Function 3: Partially overlapping loops with goto (Condition 1) */
NOINLINE int overlapping_with_goto(int N) {
    int result = 0;
    
    /* Loop A - will be 'loop' */
    for (int i = 0; i < N; ++i) {
        CREATE_PRESSURE(i);
        
    shared_block:
        result += i * 3;
        
        /* Loop B - will be 'other' */
        for (int j = 0; j < 5; ++j) {
            CREATE_PRESSURE(j);
            
            if (j == 3 && i < N/2) {
                /* Jump into loop A's body, creating intersection */
                goto shared_block;
            }
            
            result ^= j;
        }
    }
    
    return result;
}

/* Function 4: Mixed loop types with complex relationships */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    int count = N;
    
    /* do-while inside for */
    for (int i = 0; i < N; ++i) {
        int k = 0;
        
        /* do-while loop */
        do {
            CREATE_PRESSURE(k);
            result += (i * k) % 256;
            k++;
        } while (k < 4);
        
        /* while loop that follows */
        int m = 0;
        while (m < 3) {
            CREATE_PRESSURE(m);
            result ^= m;
            m++;
        }
    }
    
    /* Another for loop that shares some blocks via break */
    for (int i = 0; i < count; ++i) {
        CREATE_PRESSURE(i);
        
        for (int j = 0; j < 10; ++j) {
            if (j == 5) {
                /* Break to outer loop's continuation */
                result += 100;
                break;
            }
            CREATE_PRESSURE(j);
            result += j;
        }
    }
    
    return result;
}

/* Function 5: Sibling loops with partial overlap via shared condition */
NOINLINE int sibling_loops(int N) {
    int result = 0;
    
    /* Two sequential loops that might share some CFG blocks */
    
    /* First loop */
    for (int i = 0; i < N; ++i) {
        if (i % 2 == 0) {
            CREATE_PRESSURE(i);
            result += i * 2;
        } else {
            CREATE_PRESSURE(i);
            result += i * 3;
        }
    }
    
    /* Second loop with similar structure */
    for (int j = 0; j < N/2; ++j) {
        if (j % 3 == 0) {
            CREATE_PRESSURE(j);
            result ^= j * 5;
        } else {
            CREATE_PRESSURE(j);
            result ^= j * 7;
        }
    }
    
    return result;
}

/* Main function with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent optimization */
    volatile int seed = argc;
    int N1 = (seed % 50) + 20;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 10;
    int N4 = (seed % 25) + 8;
    int N5 = (seed % 20) + 5;
    
    int total = 0;
    
    /* Call all functions to ensure they're compiled and executed */
    total += perfect_nesting(N1);
    total += loop_subset_of_other(N2);
    total += overlapping_with_goto(N3);
    total += mixed_loop_types(N4);
    total += sibling_loops(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
