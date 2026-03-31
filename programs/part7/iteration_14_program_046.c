/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
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

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure inner loop is perfect subset */
        
        /* Inner loop - this will be 'other' (subset of loop) */
        for (int j = 0; j < 5; ++j) {
            PRESSURE(i + j);
            result ^= (i * j) & 0xFF;
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            PRESSURE(i);
            result += j;
        }
        
        /* Second inner loop - this will be 'loop' (subset of other) */
        for (int k = 0; k < 4; ++k) {
            PRESSURE(k);
            result ^= (i * k) & 0xFF;
        }
    }
    
    return result;
}

/* Function 3: Partially overlapping loops with goto (Condition 1) */
NOINLINE int overlapping_loops(int N) {
    int result = 0;
    
    /* First loop - this will be 'loop' */
    for (int i = 0; i < N; ++i) {
        PRESSURE(i);
        
        /* Target label inside loop body */
        inner_loop_start:
        
        /* Second loop - this will be 'other' */
        for (int j = 0; j < 3; ++j) {
            PRESSURE(j);
            result += i * j;
            
            /* Jump into first loop's body to create intersection */
            if (j == 2 && i < N/2) {
                goto skip_to_label;
            }
        }
        
        continue;
        
        skip_to_label:
        /* This creates shared basic blocks between the loops */
        goto inner_loop_start;
    }
    
    return result;
}

/* Function 4: Mixed loop types with complex CFG */
NOINLINE int mixed_loops(int N) {
    int result = 0;
    int i = 0;
    
    /* do-while inside for */
    for (i = 0; i < N; ++i) {
        int j = 0;
        
        /* do-while loop */
        do {
            PRESSURE(i + j);
            result ^= (i << j) & 0xFF;
            j++;
        } while (j < 3);
        
        /* while loop after do-while */
        int k = 0;
        while (k < 2) {
            PRESSURE(k);
            result += k;
            k++;
        }
    }
    
    /* Additional sibling loop */
    int m = 0;
    while (m < N/2) {
        PRESSURE(m);
        result -= m;
        m++;
    }
    
    return result;
}

/* Function 5: Disjoint loops (should not trigger intersection) */
NOINLINE int disjoint_loops(int N) {
    int result = 0;
    
    /* First completely separate loop */
    for (int i = 0; i < N; ++i) {
        PRESSURE(i);
        result += i * 2;
    }
    
    /* Some unrelated code between loops */
    int temp = result;
    asm volatile("" : : "r"(temp));
    
    /* Second completely separate loop */
    for (int j = 0; j < N/2; ++j) {
        PRESSURE(j);
        result -= j;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    int total = 0;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N);
    total ^= overlapping_loops(N);
    total ^= mixed_loops(N);
    total ^= disjoint_loops(N);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
