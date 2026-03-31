/* test_hwloop_coverage.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates specific loop structures to exercise the bitmap
 * intersection logic in hw-doloop.cc's discover_loop_hierarchy function.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in the analysis) */
    for (i = 0; i < n; ++i) {
        /* No code here ensures loop's blocks are superset of other's */
        
        /* Inner loop (will be 'other' in the analysis) */
        for (j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a ^ j;
            int c = b - i;
            int d = c * a;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= (a + b + c + d) & 0xFF;
        }
        
        /* No code here either - ensures perfect subset relationship */
    }
    
    return result;
}

/* Function 2: Reverse nesting - loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE int reverse_nesting(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in the analysis) */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i * j;
            result += a;
            asm volatile("" : : "r"(a));
        }
        
        /* Second inner loop (will be 'loop' in the analysis) */
        for (k = 0; k < i; ++k) {
            /* Create more complex operations for register pressure */
            int x = i ^ k;
            int y = x * k;
            int z = y - i;
            int w = z >> 2;
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z), "r"(w));
            result ^= (x * y * z) & 0xFF;
        }
        
        /* More code in outer loop after 'loop' */
        result += i * 7;
    }
    
    return result;
}

/* Function 3: Partially overlapping loops via goto
 * This should trigger the first condition (bitmap_intersect_p returns true)
 * but not the subset conditions
 */
NOINLINE int overlapping_loops(int n) {
    int result = 0;
    int i, j;
    
    /* First loop */
    for (i = 0; i < n; ++i) {
        int a = i * 3;
        result += a;
        
        if (i == n/2) {
            /* Jump into second loop's body */
            goto overlap_point;
        }
    }
    
    /* Second loop with label inside */
    for (j = 0; j < n; ++j) {
        int b = j * 5;
overlap_point:
        result ^= b;
        
        /* Complex body for register pressure */
        int x = j * 11;
        int y = x ^ result;
        int z = y - j;
        
        asm volatile("" : : "r"(x), "r"(y), "r"(z));
        
        if (j < n-1) {
            b = (b * 13) & 0xFF;
        }
    }
    
    return result;
}

/* Function 4: Mixed loop types (do-while inside for) */
NOINLINE int mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* for loop */
    for (i = 0; i < n; ++i) {
        int counter = i;
        
        /* do-while loop inside */
        do {
            int a = counter * 17;
            int b = a ^ counter;
            int c = b - i;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            result += (a + b + c) & 0xFF;
            
            counter--;
        } while (counter > 0 && counter < 10);
        
        /* while loop after do-while */
        int j = 0;
        while (j < 3) {
            result ^= (i * j * 19) & 0xFF;
            j++;
            asm volatile("" : : "r"(j));
        }
    }
    
    return result;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int complex_nesting(int n) {
    int result = 0;
    
    /* Level 1 */
    for (int a = 0; a < n; a++) {
        /* Level 2 - first sibling */
        for (int b = 0; b < a; b++) {
            int x = a * b;
            result += x;
            asm volatile("" : : "r"(x));
        }
        
        /* Level 2 - second sibling (adjacent, not nested) */
        for (int c = 0; c < 5; c++) {
            int y = a ^ c;
            int z = y * 23;
            result ^= z;
            asm volatile("" : : "r"(y), "r"(z));
        }
        
        /* Level 2 - third with internal nesting */
        for (int d = 0; d < 3; d++) {
            /* Level 3 */
            for (int e = 0; e < 2; e++) {
                int val = (a * d * e * 29) & 0xFF;
                result += val;
                asm volatile("" : : "r"(val));
            }
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    /* Call all functions with different parameters to create
     * various loop structures in the CFG */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N + 5);
    total ^= overlapping_loops(N + 3);
    total ^= mixed_loops(N + 7);
    total ^= complex_nesting(N + 2);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    /* Generate profile data by running multiple times */
    for (int i = 0; i < 10; i++) {
        perfect_nesting(i + 1);
        reverse_nesting(i + 2);
    }
    
    return (total & 0xFF) == 0 ? 0 : 1;
}
