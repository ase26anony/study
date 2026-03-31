/* test_hwloop_coverage.c
 * 
 * This test is designed to trigger specific bitmap intersection logic
 * in GCC's hardware loop optimization pass (hw-doloop.cc).
 * 
 * Target requirements: Architecture with hardware loop support
 * (e.g., ARM, RISC-V with Ziloop extension, PowerPC).
 * 
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 *                -funroll-loops -fpeel-loops -fdump-rtl-loop2 -fdump-rtl-doloop
 * 
 * The test creates multiple functions with carefully structured loops
 * to exercise the uncovered lines in discover_loop_hierarchy:
 *   - bitmap_intersect_p (shared blocks)
 *   - bitmap_intersect_compl_p (subset relationships)
 *   - Both directions of subset relationships
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop
 * Creates: !bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap)
 * Triggers: loop->loops.safe_push(other)
 */
NOINLINE
int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (i = 0; i < n; ++i) {
        /* No code here - ensures inner loop blocks are subset of outer */
        
        /* Inner loop (will be 'other' in hierarchy) */
        for (j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = (i << 3) | (j & 7);
            int c = a ^ b;
            int d = c * (i - j);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result += d;
        }
        
        /* No code here either - maintains subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other
 * Creates: !bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap)
 * Triggers: other->loops.safe_push(loop)
 */
NOINLINE
int reverse_nesting(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int temp = i * j * 7;
            result ^= temp;
            asm volatile("" : : "r"(temp));
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (k = 0; k < i; ++k) {
            /* Complex body for register pressure */
            int a = i * k;
            int b = k * k - i;
            int c = (a << 2) | (b & 15);
            int d = c ^ result;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result += d;
        }
        
        /* More code in outer loop after inner loops */
        result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto
 * Creates: bitmap_intersect_p is true (shared block via goto)
 * But neither is subset of the other
 */
NOINLINE
int overlapping_loops(int n) {
    int result = 0;
    int i, j;
    
    /* First loop */
    for (i = 0; i < n; ++i) {
        if (i & 1) {
            /* Jump into second loop's body */
            goto shared_block;
        }
        
        result += i * 3;
        continue;
        
    shared_block:
        /* This block is shared between both loops */
        result ^= i;
        
        /* Second loop */
        for (j = 0; j < 5; ++j) {
            int a = i + j;
            int b = a * a - j;
            asm volatile("" : : "r"(a), "r"(b));
            result += b;
        }
        
        /* Break back to first loop */
        if (i < n - 1) {
            continue;
        }
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types (do-while inside for)
 * Creates varied CFG structures */
NOINLINE
int mixed_loop_types(int n) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < n) {
        int j = 0;
        
        /* do-while loop inside */
        do {
            int a = i * j;
            int b = (i << j) | (j << i);
            int c = a ^ b ^ result;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            
            result += c;
            j++;
        } while (j < 4);
        
        i++;
    }
    
    /* Another for loop after while */
    for (int k = 0; k < n; k += 2) {
        result ^= k * k;
        asm volatile("" : : "r"(k));
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with sibling loops
 * Creates multiple loop relationships in one function */
NOINLINE
int complex_nesting(int n) {
    int result = 0;
    
    /* Level 1: Outer loop */
    for (int i = 0; i < n; i++) {
        /* Level 2: First middle loop */
        for (int j = 0; j < i; j++) {
            /* Level 3: Innermost loop A */
            for (int k = 0; k < 2; k++) {
                int a = i * j * k;
                int b = (a << 3) ^ j;
                asm volatile("" : : "r"(a), "r"(b));
                result += b;
            }
        }
        
        /* Level 2: Second middle loop (sibling of first) */
        for (int j = n - 1; j > i; j--) {
            /* Level 3: Innermost loop B */
            int m = 0;
            while (m < 3) {
                int a = i * j * m;
                int b = a - result;
                int c = b * b;
                asm volatile("" : : "r"(a), "r"(b), "r"(c));
                result ^= c;
                m++;
            }
        }
        
        /* Additional computation in outer loop */
        result = (result * 6364136223846793005ULL) & 0xFFFFFF;
    }
    
    return result & 0xFF;
}

/* Main function to drive all test cases */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 50) + 20;  /* Ensure loops run enough iterations */
    
    /* Call all test functions */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N + 5);
    total ^= overlapping_loops(N + 3);
    total ^= mixed_loop_types(N + 7);
    total ^= complex_nesting(N + 2);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
