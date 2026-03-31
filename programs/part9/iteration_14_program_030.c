/* test_hwloop_coverage.c
 * 
 * This test is designed to trigger specific bitmap intersection logic
 * in GCC's hardware loop optimization pass (hw-doloop.cc).
 * 
 * Compilation for coverage analysis:
 *   gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a \
 *       -fdump-rtl-loop2 -fdump-rtl-doloop \
 *       test_hwloop_coverage.c -o test_hwloop_executable
 * 
 * Run the executable to generate profile data:
 *   ./test_hwloop_executable
 * 
 * Target requirements: ARMv8-A or other architecture with hardware loop support.
 * The test creates specific loop nesting patterns to exercise:
 *   1. Loops with intersecting but not subset block bitmaps
 *   2. Perfectly nested loops (inner is subset of outer)
 *   3. Sibling loops where one is subset of the other's container
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - inner loop is subset of outer loop
 * This should trigger: !bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap)
 * where 'other' (inner) is subset of 'loop' (outer)
 */
NOINLINE
int perfect_nesting(int N) {
    int result = 0;
    int i, j;
    
    /* Outer loop - will be 'loop' in the analysis */
    for (i = 0; i < N; ++i) {
        /* No code here to ensure inner loop is perfect subset */
        
        /* Inner loop - will be 'other' in the analysis */
        for (j = 0; j < (i % 5) + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a + j;
            int c = b - i;
            int d = c * 3;
            int e = d >> 2;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            
            result ^= (a * b) + (c ^ d) - e;
        }
        
        /* No code here either - inner loop is perfect subset */
    }
    
    return result & 0xFF;
}

/* Function 2: Sibling loops where second is subset of container
 * This should trigger: !bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap)
 * where 'loop' (second inner) is subset of 'other' (outer)
 */
NOINLINE
int sibling_subset(int N) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop - will be 'other' in the analysis */
    for (i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 2; ++j) {
            int a = i + j;
            int b = a * 2;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Some intermediate code - ensures blocks in 'other' not in 'loop' */
        int temp = result * 3;
        asm volatile("" : : "r"(temp));
        
        /* Second inner loop - will be 'loop' in the analysis */
        for (k = 0; k < 3; ++k) {
            int x = i * k;
            int y = x + temp;
            result ^= y;
            asm volatile("" : : "r"(x), "r"(y));
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops via goto
 * This should trigger the first condition: bitmap_intersect_p returns true
 * but neither is subset of the other
 */
NOINLINE
int overlapping_loops(int N) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - will be 'loop' in the analysis */
    for (i = 0; i < N; ++i) {
        if (i % 7 == 0) {
            /* Jump into second loop's body */
            goto overlap_point;
        }
        
        result += i * 2;
    }
    
    /* Second loop - will be 'other' in the analysis */
    while (j < N) {
        overlap_point:
        result ^= j * 3;
        j++;
        
        /* Complex body for register pressure */
        int a = result;
        int b = a << 2;
        int c = b - j;
        int d = c / 2;
        asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
        
        if (j > N/2) break;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with do-while inside for */
NOINLINE
int mixed_loops(int N) {
    int result = 0;
    
    /* Outer for loop */
    for (int i = 0; i < N; ++i) {
        /* do-while loop inside */
        int j = 0;
        do {
            int a = i * j;
            int b = a + result;
            int c = b ^ 0x55;
            result = c;
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            j++;
        } while (j < (i % 4) + 1);
        
        /* while loop after do-while */
        int k = 0;
        while (k < 2) {
            result += k * i;
            asm volatile("" : : "r"(k));
            k++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with multiple exits */
NOINLINE
int complex_nesting(int N) {
    int result = 0;
    
    /* Three-level nesting */
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < (i % 3) + 2; ++j) {
            /* Innermost loop with early exit */
            for (int k = 0; k < 5; ++k) {
                if (k == j) break;
                
                /* Heavy computation for register pressure */
                int a = i * j * k;
                int b = a + result;
                int c = b ^ (i << 3);
                int d = c * 7;
                int e = d >> 1;
                int f = e - k;
                
                asm volatile("" : : "r"(a), "r"(b), "r"(c), 
                                     "r"(d), "r"(e), "r"(f));
                
                result = f;
            }
            
            /* Additional code in middle loop */
            result ^= (i << j);
        }
        
        /* Code after inner loops but still in outer loop */
        result += i * 11;
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile/argc to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 20) + 25;
    int N5 = (seed % 10) + 30;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= sibling_subset(N2);
    total ^= overlapping_loops(N3);
    total ^= mixed_loops(N4);
    total ^= complex_nesting(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
