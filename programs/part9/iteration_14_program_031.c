/* test_hwloop_coverage.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates specific loop structures to exercise the bitmap
 * intersection logic in discover_loop_hierarchy:
 * 1. Perfectly nested loops (other ⊆ loop)
 * 2. Outer loop containing multiple inner loops (loop ⊆ other)
 * 3. Partially overlapping loops via goto
 * 4. Disjoint loops for comparison
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfectly nested loops - other ⊆ loop
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure inner loop is subset */
        
        /* Inner loop (will be 'other' in hierarchy) */
        for (int j = 0; j < 5; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = a * 2;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= d;
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Outer loop with multiple inner loops - loop ⊆ other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE int outer_with_multiple_inner(int N) {
    int result = 0;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            int a = i * j;
            int b = a + 1;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Some intermediate code in outer loop */
        int temp = i * 2;
        asm volatile("" : : "r"(temp));
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (int k = 0; k < 4; ++k) {
            /* This loop is subset of outer loop */
            int x = i + k;
            int y = x * 3;
            result ^= y;
            asm volatile("" : : "r"(x), "r"(y));
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops via goto
 * This should trigger the first condition: bitmap_intersect_p = true
 * but not the subset conditions
 */
NOINLINE int overlapping_via_goto(int N) {
    int result = 0;
    
    /* Loop A */
    for (int i = 0; i < N; ++i) {
        int a = i * 3;
        
        /* Loop B - partially overlaps with A via goto */
        for (int j = 0; j < 5; ++j) {
            if (j == 2 && i % 3 == 0) {
                /* Jump into Loop A's body */
                goto shared_block;
            }
            
            int b = j * 2;
            result += b;
            asm volatile("" : : "r"(b));
        }
        
        /* This block is shared via goto */
        shared_block:
        int shared = a + i;
        result ^= shared;
        asm volatile("" : : "r"(shared));
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with do-while and while */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    
    /* for loop */
    for (int i = 0; i < N; ++i) {
        /* do-while loop inside for */
        int j = 0;
        do {
            int a = i + j;
            int b = a * a;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
            j++;
        } while (j < 3);
        
        /* while loop after do-while */
        int k = 0;
        while (k < 2) {
            int c = i - k;
            result ^= c;
            asm volatile("" : : "r"(c));
            k++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int complex_sibling_loops(int N) {
    int result = 0;
    
    /* Level 1: Outer loop */
    for (int i = 0; i < N; ++i) {
        /* Level 2: First middle loop */
        for (int j = 0; j < 3; ++j) {
            /* Level 3: Innermost loop A */
            for (int k = 0; k < 2; ++k) {
                int val = i * j * k;
                result += val;
                asm volatile("" : : "r"(val));
            }
        }
        
        /* Level 2: Second middle loop (sibling of first) */
        for (int m = 0; m < 4; ++m) {
            /* Level 3: Innermost loop B */
            for (int n = 0; n < 2; ++n) {
                int val = i * m * n;
                result ^= val;
                asm volatile("" : : "r"(val));
            }
        }
    }
    
    return result & 0xFF;
}

/* Function 6: Disjoint loops for comparison */
NOINLINE int disjoint_loops(int N) {
    int result = 0;
    
    /* First independent loop */
    for (int i = 0; i < N/2; ++i) {
        int a = i * 7;
        result += a;
        asm volatile("" : : "r"(a));
    }
    
    /* Some unrelated code */
    int temp = N * 2;
    
    /* Second independent loop */
    for (int j = 0; j < N/2; ++j) {
        int b = j * 11;
        result ^= b;
        asm volatile("" : : "r"(b));
    }
    
    return result & 0xFF;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= outer_with_multiple_inner(N + 1);
    total ^= overlapping_via_goto(N + 2);
    total ^= mixed_loop_types(N + 3);
    total ^= complex_sibling_loops(N + 4);
    total ^= disjoint_loops(N + 5);
    
    /* Generate side effect to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
