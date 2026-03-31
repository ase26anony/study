/* test_hwloop_coverage.c
 * 
 * This test is designed to trigger specific bitmap intersection logic
 * in GCC's hardware loop optimization pass (hw-doloop.cc).
 * 
 * Compilation for coverage analysis:
 *   gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop_coverage.c -o test_hwloop.o
 *   gcc -fprofile-arcs -ftest-coverage test_hwloop.o -o test_hwloop_executable
 *   ./test_hwloop_executable
 *   gcov -b test_hwloop_coverage.c
 *
 * For aggressive loop optimization:
 *   gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage -march=armv8-a test_hwloop_coverage.c -o test_hwloop_executable
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units for coverage */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* ======================================================================
 * Function 1: Perfect nesting (other is subset of loop)
 * This should trigger: !bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap)
 * Result: loop->loops.safe_push(other)
 * ====================================================================== */
NOINLINE
int perfect_nesting(int N) {
    int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' in the hierarchy */
    for (i = 0; i < N; ++i) {
        /* Inner loop - this will be 'other' in the hierarchy */
        for (j = 0; j < (N - i); ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = i * 2;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            
            /* Prevent optimization */
            asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= d;
        }
        /* No code here - ensures inner loop blocks are subset of outer loop blocks */
    }
    
    return result & 0xFF;
}

/* ======================================================================
 * Function 2: Reverse nesting (loop is subset of other)
 * This should trigger: !bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap)
 * Result: other->loops.safe_push(loop)
 * ====================================================================== */
NOINLINE
int reverse_nesting(int N) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop - this will be 'other' in the hierarchy */
    for (i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' that are not in 'loop' */
        for (j = 0; j < (i & 3); ++j) {
            int a = i * j;
            int b = a ^ result;
            asm volatile ("" : : "r"(a), "r"(b));
            result += b;
        }
        
        /* Second inner loop - this will be 'loop' in the hierarchy */
        for (k = 0; k < (N - i); ++k) {
            int a = i + k;
            int b = k * 3;
            int c = (a * b) & 0xFF;
            asm volatile ("" : : "r"(a), "r"(b), "r"(c));
            result ^= c;
        }
    }
    
    return result & 0xFF;
}

/* ======================================================================
 * Function 3: Partially overlapping loops with goto
 * This should trigger: bitmap_intersect_p(other->block_bitmap, loop->block_bitmap) = true
 * But not the subset conditions
 * ====================================================================== */
NOINLINE
int overlapping_loops(int N) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - will be 'loop' */
    for (i = 0; i < N; ++i) {
        int a = i * 2;
        asm volatile ("" : : "r"(a));
        result += a;
        
        if (i == N/2) {
            /* Jump into the middle of the second loop */
            goto overlap_point;
        }
    }
    
    /* Second loop - will be 'other' */
    for (j = 0; j < N; ++j) {
        overlap_point:
        int b = j * 3;
        int c = b ^ result;
        asm volatile ("" : : "r"(b), "r"(c));
        result ^= c;
        
        if (j > N/2) {
            /* Jump back out */
            break;
        }
    }
    
    return result & 0xFF;
}

/* ======================================================================
 * Function 4: Mixed loop types (do-while inside for)
 * Creates varied CFG structures
 * ====================================================================== */
NOINLINE
int mixed_loops(int N) {
    int result = 0;
    int i = 0;
    
    /* Outer for loop */
    for (i = 0; i < N; ++i) {
        int counter = i;
        
        /* Inner do-while loop */
        do {
            int a = counter * 5;
            int b = a ^ (result << 2);
            int c = b - counter;
            asm volatile ("" : : "r"(a), "r"(b), "r"(c));
            result += c;
            counter--;
        } while (counter > 0 && counter < 10);
        
        /* While loop after do-while */
        int j = 0;
        while (j < (i & 7)) {
            int d = result * j;
            asm volatile ("" : : "r"(d));
            result ^= d;
            j++;
        }
    }
    
    return result & 0xFF;
}

/* ======================================================================
 * Function 5: Complex nested structure with sibling loops
 * Creates multiple loop relationships in one function
 * ====================================================================== */
NOINLINE
int complex_nesting(int N) {
    int result = 0;
    int i, j, k;
    
    /* Level 1: Outer loop */
    for (i = 0; i < N; ++i) {
        /* Level 2: First middle loop */
        for (j = 0; j < (i + 1); ++j) {
            /* Level 3: Innermost loop A */
            for (k = 0; k < 3; ++k) {
                int a = i * j * k;
                int b = a ^ result;
                asm volatile ("" : : "r"(a), "r"(b));
                result += b;
            }
        }
        
        /* Level 2: Second middle loop (sibling of first) */
        for (j = 0; j < (N - i); ++j) {
            int c = i * j;
            int d = c << (j & 3);
            asm volatile ("" : : "r"(c), "r"(d));
            result ^= d;
            
            /* Small while loop inside */
            int m = 0;
            while (m < 2) {
                result += (c >> m);
                m++;
            }
        }
    }
    
    return result & 0xFF;
}

/* ======================================================================
 * Main function: Calls all test functions with varying parameters
 * ====================================================================== */
int main(int argc, char *argv[]) {
    int total_result = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 50) + 20;
    int N2 = (seed % 40) + 30;
    int N3 = (seed % 30) + 10;
    int N4 = (seed % 60) + 5;
    int N5 = (seed % 25) + 15;
    
    /* Call all test functions */
    total_result ^= perfect_nesting(N1);
    total_result ^= reverse_nesting(N2);
    total_result ^= overlapping_loops(N3);
    total_result ^= mixed_loops(N4);
    total_result ^= complex_nesting(N5);
    
    /* Generate side effect to prevent dead code elimination */
    printf("Result: %d\n", total_result & 0xFF);
    
    return (total_result & 0xFF) == 0 ? 0 : 1;
}
