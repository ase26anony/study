/* hwloop_coverage_test.c
 * 
 * This test is designed to trigger specific bitmap intersection logic
 * in GCC's hardware loop optimization pass (hw-doloop.cc).
 * 
 * Target requirements: Architecture with hardware loop support
 * Compile with: -O2 -doloop -fprofile-arcs -ftest-coverage
 * For ARM: -march=armv8-a
 * For RISC-V: -march=rv64gc_ziloop
 * 
 * The test creates complex loop structures with specific block relationships
 * to exercise the uncovered lines in discover_loop_hierarchy:
 * - Loops with intersecting block bitmaps
 * - Perfectly nested loops (subset relationship)
 * - Partially overlapping loops
 * - Sibling loops with one being subset of another
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfectly nested loops - other is subset of loop
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE int test_perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure inner loop is perfect subset */
        
        /* Inner loop - this will be 'other' (subset of outer) */
        for (j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = (i << 3) | (j & 7);
            int c = a ^ b;
            int d = c * (i - j);
            
            /* Prevent optimization */
            asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d));
            
            result += d;
        }
        
        /* No code here either - inner loop is perfect subset */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop with subset relationship reversed
 * Outer loop contains two inner loops, second inner is subset
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE int test_reverse_subset(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop - this will be 'other' */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int temp = i * j * 7;
            result ^= temp;
            asm volatile("" : "+r"(temp));
        }
        
        /* Second inner loop - this will be 'loop' (subset of outer) */
        for (k = 0; k < i % 5 + 1; ++k) {
            /* Complex body for register pressure */
            int a = i * k * 11;
            int b = (i << 2) + (k << 1);
            int c = a | b;
            int d = c ^ result;
            int e = d * 3;
            
            asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), "+r"(e));
            result += e;
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops via goto
 * Creates loops that intersect but neither is subset
 * Should trigger first if but not the subset conditions
 */
NOINLINE int test_partial_overlap(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - will be 'loop' */
    for (i = 0; i < n; ++i) {
        int a = i * 3;
        result += a;
        
        if (i == n/2) {
            /* Jump into second loop's body */
            goto overlap_point;
        }
        
        continue;
        
overlap_point:
        /* This label is inside both loops' block sets */
        int b = i * 7;
        result ^= b;
        
        /* Second loop - will be 'other' */
        for (j = 0; j < 2; ++j) {
            int c = (i + j) * 5;
            int d = c ^ result;
            asm volatile("" : "+r"(c), "+r"(d));
            result = d;
        }
        
        /* Break to exit both loops */
        if (i > n/2) break;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex nesting
 * Combines for, while, and do-while loops
 */
NOINLINE int test_mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* Outer for loop */
    for (i = 0; i < n; i += 2) {
        int j = 0;
        
        /* Inner while loop */
        while (j < 3) {
            int a = i * j * 13;
            result += a;
            
            /* Innermost do-while */
            int k = 0;
            do {
                int b = (i + j + k) * 17;
                int c = b ^ result;
                int d = c << (k & 3);
                asm volatile("" : "+r"(b), "+r"(c), "+r"(d));
                result = d;
                k++;
            } while (k < 2);
            
            j++;
        }
        
        /* Another loop at same nesting level */
        for (int m = 0; m < i % 4; m++) {
            int e = result * m * 19;
            asm volatile("" : "+r"(e));
            result ^= e;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Adjacent loops with shared basic block via switch */
NOINLINE int test_adjacent_loops(int n) {
    int result = 0;
    int state = 0;
    
    /* First loop */
    for (int i = 0; i < n; i++) {
        switch (state) {
            case 0:
                result += i * 2;
                state = 1;
                break;
            case 1:
                result ^= i * 3;
                state = (i % 3 == 0) ? 2 : 0;
                break;
            shared_block:  /* Label shared between loops */
                result |= i * 5;
                asm volatile("" : "+r"(result));
                break;
        }
    }
    
    /* Second loop that can jump to shared_block */
    int j = n - 1;
    while (j >= 0) {
        if (j % 4 == 0) {
            goto shared_block;
        }
        
        int a = j * 7;
        int b = a ^ result;
        asm volatile("" : "+r"(a), "+r"(b));
        result = b;
        j--;
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to create varying loop bounds */
    volatile int seed = argc;
    int N1 = (seed % 50) + 20;
    int N2 = (seed % 40) + 30;
    int N3 = (seed % 30) + 10;
    int N4 = (seed % 60) + 5;
    int N5 = (seed % 25) + 15;
    
    /* Call all test functions */
    total ^= test_perfect_nesting(N1);
    total ^= test_reverse_subset(N2);
    total ^= test_partial_overlap(N3);
    total ^= test_mixed_loops(N4);
    total ^= test_adjacent_loops(N5);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    /* Additional runs with different parameters for coverage */
    if (argc > 1) {
        total ^= test_perfect_nesting(atoi(argv[1]) % 100);
        total ^= test_reverse_subset(atoi(argv[1]) % 80);
        printf("Additional result: %d\n", total & 0xFF);
    }
    
    return (total & 0xFF) == 0 ? 0 : 1;
}
