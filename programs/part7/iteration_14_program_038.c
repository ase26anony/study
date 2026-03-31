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

/* Use volatile assembly to prevent optimization */
#define KEEP(var) asm volatile("" : : "r"(var))

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    volatile int v = N;  /* Prevent constant propagation */
    int limit = v;
    
    /* Outer loop - this will be 'loop' in the analysis */
    for (int i = 0; i < limit; ++i) {
        /* Inner loop - this will be 'other' in the analysis */
        /* No code between outer header and inner loop */
        for (int j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a + j;
            int c = b - i;
            int d = c * a;
            int e = d >> 2;
            result ^= e;
            KEEP(result);
        }
        /* No code after inner loop but before outer loop ends */
    }
    return result & 0xFF;
}

/* Function 2: Loop is subset of other (Condition 3) */
NOINLINE int loop_subset_of_other(int N) {
    int result = 0;
    volatile int v = N;
    int limit = v;
    
    /* Outer loop - this will be 'other' in the analysis */
    for (int i = 0; i < limit; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            int a = i + j;
            int b = a * 2;
            result += b;
            KEEP(result);
        }
        
        /* Second inner loop - this will be 'loop' in the analysis */
        for (int k = 0; k < i + 2; ++k) {
            /* Create different register pressure pattern */
            int x = k * 3;
            int y = x - i;
            int z = y >> 1;
            result ^= z;
            KEEP(result);
        }
    }
    return result & 0xFF;
}

/* Function 3: Loops with partial overlap via goto (Condition 1) */
NOINLINE int partial_overlap_goto(int N) {
    int result = 0;
    volatile int v = N;
    int limit = v;
    
    /* Loop A - will be 'loop' in some analysis */
    for (int i = 0; i < limit; ++i) {
        int a = i * 2;
        
        /* Loop B - will be 'other' in some analysis */
        for (int j = 0; j < 5; ++j) {
            int b = j + 1;
            
            /* Create partial overlap via conditional goto into loop A's body */
            if (j == 3 && i > limit/2) {
                goto shared_block;  /* Jump into loop A's body */
            }
            
            result += a * b;
            KEEP(result);
            
            /* Continue with loop B */
            continue;
            
        shared_block:
            /* This block is shared between loop A and loop B */
            int shared = a + b;
            result ^= shared;
            KEEP(result);
            break;  /* Exit loop B, continue in loop A */
        }
        
        /* More computation in loop A */
        result += i;
        KEEP(result);
    }
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex relationships */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    volatile int v = N;
    int limit = v;
    
    /* do-while inside for */
    for (int i = 0; i < limit; ++i) {
        int counter = 0;
        
        /* do-while loop */
        do {
            int a = counter * i;
            int b = a + 7;
            result += b;
            KEEP(result);
            counter++;
        } while (counter < 3);
        
        /* while loop after for */
        int w = 0;
        while (w < 2) {
            int x = i - w;
            int y = x * x;
            result ^= y;
            KEEP(result);
            w++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with break to shared label */
NOINLINE int sibling_loops_shared(int N) {
    int result = 0;
    volatile int v = N;
    int limit = v;
    
    /* First loop */
    for (int i = 0; i < limit; ++i) {
        if (i % 2 == 0) {
            /* Second loop - sibling with potential overlap */
            for (int j = 0; j < 4; ++j) {
                if (j == 2) {
                    goto shared_computation;
                }
                result += i * j;
                KEEP(result);
            }
        }
        
        /* Shared computation block */
        shared_computation:
        int comp = i * 3 + 1;
        result ^= comp;
        KEEP(result);
    }
    
    return result & 0xFF;
}

/* Main function to drive all test cases */
int main(int argc, char *argv[]) {
    /* Use volatile and command-line args to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 50) + 20;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 10;
    int N4 = (seed % 25) + 5;
    int N5 = (seed % 20) + 3;
    
    /* Call all functions to ensure they're compiled and executed */
    int r1 = perfect_nesting(N1);
    int r2 = loop_subset_of_other(N2);
    int r3 = partial_overlap_goto(N3);
    int r4 = mixed_loop_types(N4);
    int r5 = sibling_loops_shared(N5);
    
    /* Combine results to prevent dead code elimination */
    int total = r1 + r2 + r3 + r4 + r5;
    
    /* Print result to create side effect */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
