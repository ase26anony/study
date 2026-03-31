/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * For best results, also try: -O3 -funroll-loops -fpeel-loops
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Create register pressure */
#define PRESSURE_OP(x) \
    do { \
        int _a = (x); \
        int _b = _a * 2; \
        int _c = _b - _a; \
        int _d = (_a * _b) >> (_c & 3); \
        asm volatile("" : : "r"(_d)); \
    } while(0)

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE int test_perfect_nesting(int N) {
    volatile int result = 0;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (int i = 0; i < N; ++i) {
        /* No code here ensures loop has no blocks outside other */
        
        /* Inner loop (will be 'other' in hierarchy) */
        for (int j = 0; j < i + 1; ++j) {
            PRESSURE_OP(j);
            result ^= (i * j) & 0xFF;
        }
        
        /* No code here either */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop is subset of other */
NOINLINE int test_loop_subset_of_other(int N) {
    volatile int result = 0;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            PRESSURE_OP(j);
            result += j * 7;
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (int k = 0; k < i + 2; ++k) {
            PRESSURE_OP(k);
            result ^= (i * k) & 0xFF;
        }
        
        /* More code in outer loop after 'loop' */
        PRESSURE_OP(i);
        result += i & 0xF;
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops using goto */
NOINLINE int test_partial_overlap_goto(int N) {
    volatile int result = 0;
    int i = 0, j = 0;
    
    /* Loop A (will be 'loop' in hierarchy) */
    for (i = 0; i < N; ++i) {
        PRESSURE_OP(i);
        result += i * 3;
        
    loop_a_body:
        /* This label creates a shared basic block */
        if (i & 1) {
            result ^= 0xAA;
        }
    }
    
    /* Loop B (will be 'other' in hierarchy) - shares block via goto */
    for (j = 0; j < N * 2; ++j) {
        PRESSURE_OP(j);
        result += j * 5;
        
        if (j == N) {
            /* Jump into loop A's body, creating intersection */
            goto loop_a_body;
        }
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex nesting */
NOINLINE int test_mixed_loops(int N) {
    volatile int result = 0;
    int i = 0;
    
    /* Outer for loop */
    for (i = 0; i < N; ++i) {
        /* Inner do-while loop */
        int dw = 0;
        do {
            PRESSURE_OP(dw);
            result += (i * dw) & 0xFF;
            dw++;
        } while (dw < 4);
        
        /* Another inner while loop */
        int w = 0;
        while (w < 3) {
            PRESSURE_OP(w);
            result ^= w;
            w++;
        }
        
        /* Sibling loop that shares some blocks via break */
        for (int k = 0; k < 5; ++k) {
            PRESSURE_OP(k);
            if (k == 3 && (i & 1)) {
                break;
            }
            result += k * 11;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Adjacent loops with shared condition check */
NOINLINE int test_adjacent_loops(int N) {
    volatile int result = 0;
    
    /* First loop */
    for (int i = 0; i < N; ++i) {
        PRESSURE_OP(i);
        result += i * 13;
        
        /* Shared condition block */
        if (i > N/2) {
            result ^= 0x55;
        }
    }
    
    /* Second loop that also uses the shared condition logic */
    for (int j = N; j > 0; --j) {
        PRESSURE_OP(j);
        result += j * 17;
        
        /* Same condition check - might create shared block */
        if (j < N/2) {
            result ^= 0x55;
        }
    }
    
    return result & 0xFF;
}

/* Function 6: Complex three-level nesting */
NOINLINE int test_three_level_nesting(int N) {
    volatile int result = 0;
    
    /* Level 1: outermost */
    for (int i = 0; i < N; ++i) {
        /* Level 2: middle */
        for (int j = 0; j < i + 1; ++j) {
            /* Level 3: innermost */
            for (int k = 0; k < j + 1; ++k) {
                PRESSURE_OP(k);
                result += (i * j * k) & 0xFF;
            }
            
            /* Code between inner loops at level 2 */
            PRESSURE_OP(j);
            result ^= j;
        }
        
        /* Code at level 1 between loops */
        if (i & 1) {
            PRESSURE_OP(i);
            result += 0x33;
        }
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;  /* Prevent trivial loops */
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 25) + 25;
    int N5 = (seed % 20) + 30;
    int N6 = (seed % 15) + 35;
    
    int result = 0;
    
    /* Call all test functions to ensure they're compiled and executed */
    result ^= test_perfect_nesting(N1);
    result ^= test_loop_subset_of_other(N2);
    result ^= test_partial_overlap_goto(N3);
    result ^= test_mixed_loops(N4);
    result ^= test_adjacent_loops(N5);
    result ^= test_three_level_nesting(N6);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result & 0xFF);
    
    return 0;
}
