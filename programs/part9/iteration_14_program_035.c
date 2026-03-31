/* test_hwloop.c
 * 
 * This test is designed to trigger specific bitmap intersection logic
 * in GCC's hardware loop optimization pass (hw-doloop.cc).
 * 
 * Compilation for hardware loop targets (e.g., ARMv8):
 *   gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop.c -o test_hwloop.o
 *   gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a test_hwloop.c -o test_hwloop_executable
 * 
 * For aggressive loop optimization:
 *   gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage test_hwloop.c -o test_hwloop_executable -march=native
 * 
 * The test creates multiple functions with carefully structured loops
 * to exercise the uncovered lines in discover_loop_hierarchy:
 *   if (!bitmap_intersect_p(...)) continue;
 *   if (!bitmap_intersect_compl_p(...)) loop->loops.safe_push(other);
 *   else if (!bitmap_intersect_compl_p(...)) other->loops.safe_push(loop);
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* ============================================
 * Function 1: Perfect nesting (other ⊆ loop)
 * Creates condition: !bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap)
 * ============================================ */
NOINLINE
int perfect_nesting(int N) {
    int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (i = 0; i < N; ++i) {
        /* No code here - ensures inner loop blocks are subset of outer */
        
        /* Inner loop (will be 'other' in hierarchy) */
        for (j = 0; j < (N - i); ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = i * 2;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= d;
        }
        
        /* No code here either - maintains subset relationship */
    }
    
    return result & 0xFF;
}

/* ============================================
 * Function 2: Reverse subset (loop ⊆ other)
 * Creates condition: !bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap)
 * ============================================ */
NOINLINE
int reverse_subset(int N) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i * j;
            result += a;
            asm volatile("" : : "r"(a));
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (k = 0; k < (N - i); ++k) {
            /* This loop's blocks are a subset of outer loop's blocks */
            int b = i + k;
            int c = b * 2;
            int d = c - b;
            
            asm volatile("" : : "r"(b), "r"(c), "r"(d));
            result ^= (b * c) >> (d & 3);
        }
        
        /* More code in outer loop after inner loops */
        result += i * 7;
    }
    
    return result & 0xFF;
}

/* ============================================
 * Function 3: Partial overlap with goto
 * Creates condition: bitmap_intersect_p is true
 * but neither is subset of the other
 * ============================================ */
NOINLINE
int partial_overlap(int N) {
    int result = 0;
    int i = 0, j = 0;
    
    /* Loop A */
    for (i = 0; i < N; ++i) {
        int a = i * 3;
        
    shared_label:
        /* This block will be shared between both loops via goto */
        result += a;
        asm volatile("" : : "r"(a));
        
        /* Loop B - starts here but jumps into Loop A */
        while (j < N) {
            int b = j * 5;
            
            /* Jump into Loop A's body */
            if ((j & 1) && (i < N/2)) {
                a = b + i;
                goto shared_label;
            }
            
            result ^= b;
            asm volatile("" : : "r"(b));
            j++;
            
            /* Break back to Loop A sometimes */
            if (j > N/2) break;
        }
        
        /* More computations in Loop A */
        result -= i * 2;
    }
    
    return result & 0xFF;
}

/* ============================================
 * Function 4: Mixed loop types
 * Creates varied CFG structures
 * ============================================ */
NOINLINE
int mixed_loops(int N) {
    int result = 0;
    int i = 0;
    
    /* for loop */
    for (i = 0; i < N; ++i) {
        int a = i;
        
        /* do-while inside for */
        int j = 0;
        do {
            int b = a + j;
            int c = b * 3;
            result += c;
            asm volatile("" : : "r"(b), "r"(c));
            j++;
        } while (j < 5);
        
        /* while loop after do-while */
        int k = 0;
        while (k < 3) {
            int d = (a << k);
            result ^= d;
            asm volatile("" : : "r"(d));
            k++;
        }
    }
    
    /* Another loop at same level */
    int m = N;
    while (m > 0) {
        result += m * 11;
        asm volatile("" : : "r"(m));
        m -= 2;
    }
    
    return result & 0xFF;
}

/* ============================================
 * Function 5: Complex sibling loops
 * Multiple loops at same nesting level
 * ============================================ */
NOINLINE
int sibling_loops(int N) {
    int result = 0;
    
    /* First sibling loop */
    for (int i = 0; i < N; i += 2) {
        int a = i * i;
        int b = a - i;
        result += b;
        asm volatile("" : : "r"(a), "r"(b));
    }
    
    /* Code between loops - creates separate blocks */
    int temp = result * 3;
    asm volatile("" : : "r"(temp));
    
    /* Second sibling loop - partially overlaps in block usage */
    for (int j = 1; j < N; j += 2) {
        int c = j * 7;
        int d = c + temp;
        result ^= d;
        asm volatile("" : : "r"(c), "r"(d));
        
        /* Small inner loop */
        for (int k = 0; k < 2; ++k) {
            int e = d << k;
            result += e;
            asm volatile("" : : "r"(e));
        }
    }
    
    return result & 0xFF;
}

/* ============================================
 * Main function
 * Calls all test functions with varying parameters
 * ============================================ */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile/argc to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 50) + 20;
    int N2 = (seed % 40) + 30;
    int N3 = (seed % 30) + 10;
    int N4 = (seed % 60) + 5;
    int N5 = (seed % 25) + 15;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= reverse_subset(N2);
    total ^= partial_overlap(N3);
    total ^= mixed_loops(N4);
    total ^= sibling_loops(N5);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return (total & 0xFF) == 0 ? 0 : 1;
}
