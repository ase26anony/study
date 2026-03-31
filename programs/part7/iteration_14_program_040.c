/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * Target: ARMv8-A with hardware loop support
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Create register pressure */
#define PRESSURE(x) do { \
    int a = (x), b = (x)*2, c = b - a; \
    asm volatile ("" : : "r"(a), "r"(b), "r"(c)); \
} while(0)

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    volatile int keep = N; /* Prevent optimization */
    
    /* Outer loop (will be 'loop' in bitmap logic) */
    for (int i = 0; i < keep; ++i) {
        /* No code here ensures loop blocks are superset of other */
        
        /* Inner loop (will be 'other' in bitmap logic) */
        for (int j = 0; j < i % 10 + 1; ++j) {
            result ^= i * j;
            PRESSURE(j);
            /* Create side effect */
            asm volatile ("" : "+r"(result));
        }
        
        /* No code here either - ensures perfect nesting */
    }
    
    return result;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    volatile int keep = N;
    
    /* Outer loop (will be 'other' in bitmap logic) */
    for (int i = 0; i < keep; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            result += i * j;
            PRESSURE(j);
        }
        
        /* Second inner loop (will be 'loop' in bitmap logic) */
        /* This loop's blocks are subset of outer loop's blocks */
        for (int k = 0; k < 5; ++k) {
            result ^= i * k;
            PRESSURE(k);
        }
    }
    
    return result;
}

/* Function 3: Partially overlapping loops with goto (Condition 1) */
NOINLINE int overlapping_loops(int N) {
    int result = 0;
    volatile int keep = N;
    int i, j;
    
    /* First loop (will be 'loop') */
    for (i = 0; i < keep; ++i) {
        result += i;
        PRESSURE(i);
        
    shared_label:
        /* This label creates shared basic block */
        result ^= i * 2;
        
        /* Second loop (will be 'other') */
        for (j = 0; j < 5; ++j) {
            result += j;
            PRESSURE(j);
            
            /* Jump into first loop's body */
            if (j == 2 && i < keep/2) {
                goto shared_label;
            }
        }
    }
    
    return result;
}

/* Function 4: Mixed loop types with complex CFG */
NOINLINE int mixed_loops(int N) {
    int result = 0;
    volatile int keep = N;
    int i = 0;
    
    /* do-while inside for */
    for (i = 0; i < keep; ++i) {
        int j = 0;
        
        /* do-while loop */
        do {
            result ^= i * j;
            PRESSURE(j);
            j++;
        } while (j < 3);
        
        /* while loop after do-while */
        int k = 0;
        while (k < 2) {
            result += i + k;
            PRESSURE(k);
            k++;
        }
    }
    
    /* Additional while loop that shares some blocks */
    i = 0;
    while (i < 5) {
        result ^= i;
        PRESSURE(i);
        i++;
    }
    
    return result;
}

/* Function 5: Sibling loops with break to shared block */
NOINLINE int sibling_loops(int N) {
    int result = 0;
    volatile int keep = N;
    
    /* First loop */
    for (int i = 0; i < keep; ++i) {
        if (i % 3 == 0) {
            /* Shared block via break target */
            result += i * 2;
            goto shared_exit;
        }
        PRESSURE(i);
    }
    
    /* Second loop that can break to same label */
    for (int j = 0; j < keep; ++j) {
        if (j % 5 == 0) {
            result += j * 3;
            goto shared_exit;
        }
        PRESSURE(j);
    }
    
shared_exit:
    /* Shared exit block */
    result ^= 0x55;
    return result;
}

/* Main driver with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to create runtime values */
    volatile int seed = argc;
    int N1 = (seed % 100) + 10;
    int N2 = (seed % 50) + 5;
    int N3 = (seed % 30) + 3;
    int N4 = (seed % 20) + 2;
    int N5 = (seed % 10) + 1;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= reverse_nesting(N2);
    total ^= overlapping_loops(N3);
    total ^= mixed_loops(N4);
    total ^= sibling_loops(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
