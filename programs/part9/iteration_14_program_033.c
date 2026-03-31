/* hwloop_coverage_test.c
 * 
 * This test is designed to exercise the loop hierarchy discovery logic in
 * GCC's hardware loop optimization pass (hw-doloop.cc). Specifically, it aims
 * to cover the bitmap intersection logic that determines parent/child
 * relationships between loops.
 *
 * Compilation for coverage (example for ARM target with hardware loops):
 *   gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a+lse \
 *       -fdump-rtl-doloop -fdump-rtl-loop2 \
 *       hwloop_coverage_test.c -o hwloop_coverage_test
 *
 * Run the executable to generate profile data:
 *   ./hwloop_coverage_test
 *
 * Then process coverage with gcov:
 *   gcov -b hwloop_coverage_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfectly nested loops - other is subset of loop
 * This should trigger: !bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap)
 * Result: loop->loops.safe_push(other)
 */
NOINLINE
int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' in the hierarchy */
    for (i = 0; i < n; ++i) {
        /* No code here - ensures outer loop has no blocks outside inner */
        
        /* Inner loop - this will be 'other' in the hierarchy */
        for (j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a + j;
            int c = b - i;
            int d = c * a;
            int e = d >> 2;
            
            /* Prevent optimization */
            asm volatile("" : "+r" (a), "+r" (b), "+r" (c));
            
            result ^= (a + b + c + d + e) & 0xFF;
        }
        
        /* No code here either - maintains subset relationship */
    }
    
    return result;
}

/* Function 2: Outer loop contains multiple inner loops - loop is subset of other
 * This should trigger: !bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap)
 * Result: other->loops.safe_push(loop)
 */
NOINLINE
int outer_with_multiple_inner(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'other' in the hierarchy */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i + j;
            int b = a * 2;
            result += b;
            asm volatile("" : "+r" (a), "+r" (b));
        }
        
        /* Some intermediate code in outer loop */
        int temp = i * 7;
        result ^= temp;
        
        /* Second inner loop - this will be 'loop' in the hierarchy */
        for (j = 0; j < i % 5 + 1; ++j) {
            /* Create complex body for register pressure */
            int x = i * j;
            int y = x + result;
            int z = y - j;
            asm volatile("" : "+r" (x), "+r" (y), "+r" (z));
            result = (result + x + y + z) & 0xFFFF;
        }
        
        /* More outer loop code */
        result += i * 11;
    }
    
    return result;
}

/* Function 3: Partially overlapping loops via goto
 * This should trigger the first condition: bitmap_intersect_p returns true
 * But neither subset condition is true, so no push occurs
 */
NOINLINE
int overlapping_via_goto(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - will be 'loop' */
    for (i = 0; i < n; ++i) {
        int a = i * 3;
        
    shared_label:
        /* This block will be shared via goto */
        result += a + i;
        asm volatile("" : "+r" (a));
        
        /* Second loop - will be 'other' */
        while (j < n) {
            int b = j * 5;
            result ^= b;
            
            if (j == i + 2) {
                /* Jump into first loop's body */
                goto shared_label;
            }
            
            j++;
            
            /* Create register pressure */
            int c = b + result;
            int d = c - j;
            int e = d * 2;
            asm volatile("" : "+r" (c), "+r" (d), "+r" (e));
            result += e & 0xFF;
        }
        
        result += i;
    }
    
    return result;
}

/* Function 4: Mixed loop types with complex control flow
 * Creates various loop relationships for broader coverage
 */
NOINLINE
int mixed_loop_types(int n) {
    int result = 0;
    int i = 0;
    
    /* do-while inside for */
    for (i = 0; i < n; i++) {
        int j = 0;
        
        /* do-while loop */
        do {
            int a = i + j;
            int b = a * j;
            result += b;
            asm volatile("" : "+r" (a), "+r" (b));
            j++;
        } while (j < 3);
        
        /* while loop */
        int k = 0;
        while (k < i % 4) {
            int c = result * k;
            int d = c - i;
            int e = d + k;
            asm volatile("" : "+r" (c), "+r" (d), "+r" (e));
            result ^= e;
            k++;
        }
    }
    
    /* Another for loop that shares some blocks via break */
    for (i = 0; i < n * 2; i++) {
        if (i >= n) {
            /* Break to label in next loop */
            goto break_target;
        }
        result += i * 7;
    }
    
    /* Following loop with label */
    i = 0;
    while (i < n) {
        int f = i * 11;
        
    break_target:
        result -= f;
        asm volatile("" : "+r" (f));
        
        i++;
        
        /* Nested short loop */
        for (int m = 0; m < 2; m++) {
            result += m * i;
        }
    }
    
    return result;
}

/* Function 5: Sibling loops with no intersection
 * Should not trigger any of the conditions (bitmap_intersect_p returns false)
 */
NOINLINE
int disjoint_loops(int n) {
    int result = 0;
    
    /* First independent loop */
    for (int i = 0; i < n; i++) {
        int a = i * 3;
        result += a;
        asm volatile("" : "+r" (a));
    }
    
    /* Unrelated code between loops */
    int temp = result * 2;
    result = temp ^ 0x55;
    
    /* Second independent loop */
    for (int j = 0; j < n / 2 + 1; j++) {
        int b = j * 7;
        result -= b;
        asm volatile("" : "+r" (b));
        
        /* Small nested loop inside second loop only */
        for (int k = 0; k < 2; k++) {
            result += k * j;
        }
    }
    
    return result;
}

/* Main function to drive all test cases */
int main(int argc, char *argv[]) {
    int total_result = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc + global_seed;
    int n1 = (seed % 50) + 10;
    int n2 = (seed % 40) + 15;
    int n3 = (seed % 30) + 20;
    int n4 = (seed % 60) + 5;
    int n5 = (seed % 45) + 8;
    
    /* Call all test functions */
    total_result ^= perfect_nesting(n1);
    total_result ^= outer_with_multiple_inner(n2);
    total_result ^= overlapping_via_goto(n3);
    total_result ^= mixed_loop_types(n4);
    total_result ^= disjoint_loops(n5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total_result & 0xFF);
    
    return total_result & 1;
}
