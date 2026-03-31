/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * For generic targets: gcc -O2 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Create register pressure and prevent optimization */
#define KEEP(i) asm volatile("" : : "r"(i))
#define COMPUTE(a, b, c) do { \
    int _t1 = (a) * (b); \
    int _t2 = (b) - (a); \
    int _t3 = _t1 >> (_t2 & 7); \
    KEEP(_t3); \
    (c) ^= _t3; \
} while(0)

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure other is subset */
        
        /* Inner loop - this will be 'other' in the analysis */
        for (int j = 0; j < (N - i); ++j) {
            int a = i + j;
            int b = j * 2;
            int c = a - b;
            COMPUTE(a, b, result);
            KEEP(c);
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result;
}

/* Function 2: Loop is subset of other (Condition 3) */
NOINLINE int loop_subset_of_other(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'other' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            int a = i * j;
            int b = a + 1;
            COMPUTE(a, b, result);
        }
        
        /* Second inner loop - this will be 'loop' in the analysis */
        for (int k = 0; k < (N - i); ++k) {
            int a = i + k;
            int b = k * 3;
            COMPUTE(a, b, result);
        }
    }
    
    return result;
}

/* Function 3: Partially overlapping loops with goto (Condition 1) */
NOINLINE int overlapping_with_goto(int N) {
    int result = 0;
    
    /* First loop - will be 'loop' */
    for (int i = 0; i < N; ++i) {
        int a = i * 2;
        
    loop_body:
        if (a > N/2) {
            /* Second loop - will be 'other' */
            for (int j = 0; j < 5; ++j) {
                int b = a + j;
                COMPUTE(a, b, result);
                
                /* Jump into first loop's body */
                if (j == 2 && i < N-1) {
                    a++;
                    goto loop_body;
                }
            }
        }
        
        int c = a * 3;
        KEEP(c);
    }
    
    return result;
}

/* Function 4: Mixed loop types for varied CFG */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < N) {
        int a = i * 4;
        
        /* do-while nested inside while */
        int j = 0;
        do {
            int b = a + j;
            COMPUTE(a, b, result);
            j++;
        } while (j < 3);
        
        /* for loop after do-while */
        for (int k = 0; k < 2; ++k) {
            int c = a - k;
            KEEP(c);
        }
        
        i++;
    }
    
    return result;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int complex_sibling_loops(int N) {
    int result = 0;
    
    /* Outer loop */
    for (int i = 0; i < N; ++i) {
        /* Sibling loop A */
        for (int j = 0; j < (i % 5 + 1); ++j) {
            int a = i * j;
            KEEP(a);
        }
        
        /* Some intermediate code to create separate blocks */
        int temp = i * 7;
        KEEP(temp);
        
        /* Sibling loop B - shares some blocks via control flow */
        for (int k = 0; k < (N - i); ++k) {
            int b = temp + k;
            
            /* Conditional break that could create shared blocks */
            if (b > N * 2) {
                /* This creates a shared exit block */
                result += b;
                break;
            }
            
            COMPUTE(temp, b, result);
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
    int N4 = (seed % 25) + 5;
    int N5 = (seed % 20) + 3;
    
    /* Call all functions to ensure they're compiled and executed */
    int r1 = perfect_nesting(N1);
    int r2 = loop_subset_of_other(N2);
    int r3 = overlapping_with_goto(N3);
    int r4 = mixed_loop_types(N4);
    int r5 = complex_sibling_loops(N5);
    
    /* Use results to prevent dead code elimination */
    int total = r1 + r2 + r3 + r4 + r5;
    
    /* Print something to ensure execution */
    printf("Result: %d\n", total & 255);
    
    return 0;
}
