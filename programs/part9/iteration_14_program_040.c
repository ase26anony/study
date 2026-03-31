/* test_hwloop_coverage.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates complex loop structures to exercise the loop hierarchy
 * building logic in hw-doloop.cc, specifically the bitmap intersection
 * conditions at lines 429-436.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' in the hierarchy */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure inner loop blocks are subset of outer */
        
        /* Inner loop - this will be 'other' in the hierarchy */
        for (int j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = (i << 2) + (j >> 1);
            int c = a ^ b;
            int d = c * 3;
            int e = d - a;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            
            result ^= (a + b + c + d + e) & 0xFF;
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result;
}

/* Function 2: Reverse nesting - loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'other' in the hierarchy */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            int temp = i * j * 7;
            result += temp;
            asm volatile("" : : "r"(temp));
        }
        
        /* Second inner loop - this will be 'loop' in the hierarchy */
        for (int k = 0; k < i + 2; ++k) {
            /* Register pressure */
            int x = i * k;
            int y = k * k - i;
            int z = x ^ y;
            int w = z * 5;
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z), "r"(w));
            result ^= (x + y + z + w) & 0xFF;
        }
        
        /* More code in outer loop after 'loop' */
        int outer_temp = i * 11;
        result -= outer_temp;
        asm volatile("" : : "r"(outer_temp));
    }
    
    return result;
}

/* Function 3: Partially overlapping loops via goto
 * This should trigger the first condition (bitmap_intersect_p returns true)
 * but not the subset conditions
 */
NOINLINE int overlapping_loops(int N) {
    int result = 0;
    
    /* Loop A */
    for (int i = 0; i < N; ++i) {
        if (i % 3 == 0) {
            /* Jump into Loop B's body */
            goto into_loop_b;
        }
        
        /* Loop B */
        for (int j = 0; j < 5; ++j) {
            into_loop_b:
            int a = i + j * 2;
            int b = a * 3 - j;
            
            asm volatile("" : : "r"(a), "r"(b));
            result += a ^ b;
            
            if (j == 4 && i < N - 1) {
                /* Break back to Loop A */
                break;
            }
        }
        
        /* More computation in Loop A */
        int c = i * 13;
        result ^= c;
        asm volatile("" : : "r"(c));
    }
    
    return result;
}

/* Function 4: Mixed loop types with complex control flow
 * Creates varied loop structures in the CFG
 */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < N) {
        /* do-while loop inside while */
        int j = 0;
        do {
            int a = i * j;
            int b = (i << 3) | j;
            int c = a + b;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            result += c & 0x7F;
            
            j++;
        } while (j < 4);
        
        /* for loop after do-while */
        for (int k = 0; k < 3; ++k) {
            int d = (i * k) + result;
            int e = d ^ k;
            
            asm volatile("" : : "r"(d), "r"(e));
            result = e;
        }
        
        i++;
    }
    
    return result;
}

/* Function 5: Sibling loops with shared basic blocks
 * Two adjacent loops that share some control flow structure
 */
NOINLINE int sibling_loops(int N) {
    int result = 0;
    
    /* Common setup block */
    int setup = N * 2;
    asm volatile("" : : "r"(setup));
    
    /* First sibling loop */
    for (int i = 0; i < N; i += 2) {
        int a = i * i;
        int b = a + setup;
        
        asm volatile("" : : "r"(a), "r"(b));
        result ^= b;
    }
    
    /* Shared intermediate block */
    int intermediate = result * 3;
    asm volatile("" : : "r"(intermediate));
    
    /* Second sibling loop - shares some CFG structure with first */
    for (int j = 1; j < N; j += 2) {
        int c = j * intermediate;
        int d = c - j;
        
        asm volatile("" : : "r"(c), "r"(d));
        result += d;
    }
    
    return result;
}

/* Main function to drive all test cases */
int main(int argc, char *argv[]) {
    int total_result = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 50) + 20;  /* Ensure loops run enough iterations */
    
    printf("Running hardware loop coverage tests with N=%d\n", N);
    
    /* Run all test functions */
    total_result ^= perfect_nesting(N);
    total_result += reverse_nesting(N);
    total_result ^= overlapping_loops(N);
    total_result += mixed_loop_types(N);
    total_result ^= sibling_loops(N);
    
    /* Additional runs with different parameters */
    for (int run = 0; run < 3; ++run) {
        int M = (N + run * 7) % 40 + 10;
        total_result += perfect_nesting(M);
        total_result ^= reverse_nesting(M);
    }
    
    /* Ensure result is used to prevent dead code elimination */
    printf("Final result: %d\n", total_result & 0xFF);
    
    return (total_result & 0xFF) == 0 ? 0 : 1;
}
