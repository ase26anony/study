/* 
 * Test program for GCC hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * For generic targets: gcc -O2 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Create register pressure and prevent optimization */
#define KEEP(i) asm volatile("" : : "r"(i))
#define BUSY_WORK(i) \
    do { \
        int a = (i) * 3; \
        int b = (i) << 2; \
        int c = a ^ b; \
        int d = c * 7; \
        KEEP(a); KEEP(b); KEEP(c); KEEP(d); \
    } while(0)

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int total = 0;
    volatile int seed = N; /* Prevent constant propagation */
    int limit = (seed % 50) + 10;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < limit; ++i) {
        /* No code here - ensures other is subset */
        
        /* Inner loop - this will be 'other' */
        for (int j = 0; j < i + 1; ++j) {
            BUSY_WORK(j);
            total += (i * j) & 0xFF;
        }
        
        /* No code here either - maintains subset relationship */
    }
    
    return total & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE int reverse_nesting(int N) {
    int total = 0;
    volatile int seed = N;
    int limit = (seed % 40) + 15;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < limit; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            BUSY_WORK(j);
            total ^= (i + j) * 7;
        }
        
        /* Second inner loop - this will be 'loop' (subset of other) */
        for (int k = 0; k < i % 5 + 2; ++k) {
            BUSY_WORK(k);
            total += (i * k) & 0xFF;
        }
    }
    
    return total & 0xFF;
}

/* Function 3: Partial overlap with goto - triggers first condition */
NOINLINE int partial_overlap_goto(int N) {
    int total = 0;
    volatile int seed = N;
    int limit = (seed % 30) + 20;
    
    /* Loop A - will be 'loop' */
    for (int i = 0; i < limit; ++i) {
        BUSY_WORK(i);
        
    shared_label:
        total += i * 3;
        
        /* Loop B - will be 'other' */
        int j = 0;
        while (j < 5) {
            BUSY_WORK(j);
            total ^= j;
            
            if (j == 3 && (i & 1)) {
                /* Jump into loop A's body - creates intersection */
                goto shared_label;
            }
            j++;
        }
    }
    
    return total & 0xFF;
}

/* Function 4: Mixed loop types with complex CFG */
NOINLINE int mixed_loop_types(int N) {
    int total = 0;
    volatile int seed = N;
    int limit = (seed % 25) + 25;
    
    /* do-while inside for */
    for (int i = 0; i < limit; ++i) {
        int count = 0;
        
        do {
            BUSY_WORK(count);
            total += (i * count) & 0xFF;
            count++;
            
            /* Nested while loop */
            int inner = 0;
            while (inner < 2) {
                BUSY_WORK(inner);
                total ^= inner;
                inner++;
            }
        } while (count < (i % 4 + 1));
    }
    
    /* Additional while loop after for */
    int extra = 0;
    while (extra < 10) {
        BUSY_WORK(extra);
        total += extra * 11;
        extra++;
    }
    
    return total & 0xFF;
}

/* Function 5: Sibling loops with break to shared label */
NOINLINE int sibling_loops_break(int N) {
    int total = 0;
    volatile int seed = N;
    int limit = (seed % 35) + 15;
    
    /* First loop - will be 'loop' */
    for (int i = 0; i < limit; ++i) {
        BUSY_WORK(i);
        
    shared_block:
        total += i * 5;
        
        /* Second loop - will be 'other' */
        for (int j = 0; j < 4; ++j) {
            BUSY_WORK(j);
            total ^= j;
            
            if (j == 2 && (total & 1)) {
                /* Break to label in first loop */
                goto shared_block;
            }
        }
        
        /* Third loop to create more blocks */
        int k = 0;
        while (k < 3) {
            BUSY_WORK(k);
            total += k * 7;
            k++;
        }
    }
    
    return total & 0xFF;
}

/* Function 6: Complex nested structure with multiple levels */
NOINLINE int multi_level_nesting(int N) {
    int total = 0;
    volatile int seed = N;
    int limit = (seed % 20) + 30;
    
    /* Level 1 */
    for (int a = 0; a < limit; ++a) {
        BUSY_WORK(a);
        
        /* Level 2 - first */
        for (int b = 0; b < a % 3 + 2; ++b) {
            BUSY_WORK(b);
            
            /* Level 3 - inner */
            int c = 0;
            do {
                BUSY_WORK(c);
                total += (a * b * c) & 0xFF;
                c++;
            } while (c < 2);
        }
        
        /* Level 2 - second (creates sibling relationship) */
        int d = 0;
        while (d < 3) {
            BUSY_WORK(d);
            total ^= (a * d) & 0xFF;
            d++;
        }
    }
    
    return total & 0xFF;
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int base = argc;
    
    /* Call each function with different arguments */
    result ^= perfect_nesting(base + 1);
    result ^= reverse_nesting(base + 2);
    result ^= partial_overlap_goto(base + 3);
    result ^= mixed_loop_types(base + 4);
    result ^= sibling_loops_break(base + 5);
    result ^= multi_level_nesting(base + 6);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result & 0xFF);
    
    return 0;
}
