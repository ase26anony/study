/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * or for generic targets: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage
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
        asm volatile("" : : "r"(d)); \
    } while(0)

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure inner loop blocks are subset of outer */
        
        /* Inner loop - this will be 'other' in the analysis */
        for (int j = 0; j < N/2; ++j) {
            CREATE_PRESSURE(j);
            result ^= (i * j) & 0xFF;
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'other' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            CREATE_PRESSURE(j);
            result += j * 7;
        }
        
        /* Second inner loop - this will be 'loop' in the analysis */
        for (int k = 0; k < N/3; ++k) {
            CREATE_PRESSURE(k);
            result ^= (i * k) & 0xFF;
        }
    }
    
    return result;
}

/* Function 3: Partial overlap with goto (Condition 1) */
NOINLINE int partial_overlap_goto(int N) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - will be 'loop' */
    for (i = 0; i < N; ++i) {
        CREATE_PRESSURE(i);
        
        /* Target label inside loop body */
        inner_loop_start:
        result += i * 3;
        
        /* Second loop - will be 'other' */
        for (j = 0; j < N/2; ++j) {
            CREATE_PRESSURE(j);
            result ^= j * 5;
            
            /* Jump into first loop's body */
            if (j == N/4) {
                goto inner_loop_start;  /* Creates block intersection */
            }
        }
    }
    
    return result;
}

/* Function 4: Mixed loop types with break to shared block */
NOINLINE int mixed_loops_shared_block(int N) {
    int result = 0;
    int i = 0;
    
    /* do-while loop inside for loop */
    for (i = 0; i < N; ++i) {
        int k = 0;
        
        /* do-while - will be 'other' */
        do {
            CREATE_PRESSURE(k);
            result += k * 11;
            k++;
            
            /* Break to shared block in outer loop */
            if (k > N/3) {
                goto shared_block;
            }
        } while (k < N/2);
        
        /* while loop - will be 'loop' */
        int m = 0;
        shared_block:  /* Shared block between loops */
        CREATE_PRESSURE(m);
        while (m < N/4) {
            result ^= m * 13;
            m++;
        }
    }
    
    return result;
}

/* Function 5: Complex sibling loops with partial intersection */
NOINLINE int sibling_loops_partial(int N) {
    int result = 0;
    
    /* First loop - will be 'loop' */
    for (int i = 0; i < N; ++i) {
        CREATE_PRESSURE(i);
        
        /* Conditional that creates shared basic block */
        if (i % 2 == 0) {
            shared_label:
            result += i * 17;
        }
        
        /* Second loop - will be 'other' */
        for (int j = 0; j < N/3; ++j) {
            CREATE_PRESSURE(j);
            result ^= j * 19;
            
            /* Jump to shared block in first loop */
            if (j == N/6) {
                goto shared_label;
            }
        }
    }
    
    return result;
}

/* Function 6: Adjacent loops with no intersection (for contrast) */
NOINLINE int adjacent_disjoint(int N) {
    int result = 0;
    
    /* First loop - completely separate from second */
    for (int i = 0; i < N; ++i) {
        CREATE_PRESSURE(i);
        result += i * 23;
    }
    
    /* Second loop - no block sharing with first */
    for (int j = 0; j < N; ++j) {
        CREATE_PRESSURE(j);
        result ^= j * 29;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;  /* Ensure loops run */
    int total = 0;
    
    /* Call all functions to ensure compilation and execution */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N);
    total ^= partial_overlap_goto(N);
    total ^= mixed_loops_shared_block(N);
    total ^= sibling_loops_partial(N);
    total ^= adjacent_disjoint(N);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    /* Additional calls with different N values */
    for (int i = 1; i < 5; i++) {
        N = (seed * i) % 50 + 5;
        total ^= perfect_nesting(N);
        total ^= reverse_nesting(N);
    }
    
    printf("Final: %d\n", total & 0xFF);
    return 0;
}
