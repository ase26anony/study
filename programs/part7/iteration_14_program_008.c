/* test_hwloop_coverage.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates complex loop structures with specific block relationships
 * to trigger the bitmap intersection logic in hw-doloop.cc lines 429-436.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))
#define OPTIMIZE __attribute__((optimize("O2")))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE OPTIMIZE
int perfect_nesting(int N) {
    int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in the analysis) */
    for (i = 0; i < N; ++i) {
        /* No code here to ensure other is a perfect subset */
        
        /* Inner loop (will be 'other' in the analysis) */
        for (j = 0; j < N/2; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = i * 2 - j;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= d;
        }
        
        /* No code here either - inner loop is perfect subset */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE OPTIMIZE
int loop_subset_of_other(int N) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in the analysis) */
    for (i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i * j;
            int b = a + global_seed;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Some intermediate code in 'other' but not in 'loop' */
        int temp = i * 2;
        asm volatile("" : : "r"(temp));
        
        /* Second inner loop (will be 'loop' in the analysis) */
        for (k = 0; k < N/3; ++k) {
            /* This loop is a subset of 'other' */
            int x = i + k;
            int y = x * temp;
            int z = y >> 2;
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
            result ^= z;
        }
        
        /* More code in 'other' but not in 'loop' */
        result += temp;
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto
 * This ensures bitmap_intersect_p returns true
 */
NOINLINE OPTIMIZE
int overlapping_loops_goto(int N) {
    int result = 0;
    int i, j;
    
    /* Loop A */
    for (i = 0; i < N; ++i) {
        int a = i * 3;
        
    loop_b_start:
        /* Loop B - shares block via goto */
        for (j = 0; j < 5; ++j) {
            if (j == 3 && i % 2 == 0) {
                /* Jump into loop A's body */
                goto inside_loop_a;
            }
            
            int b = a + j;
            result += b;
            asm volatile("" : : "r"(b));
        }
        
        continue;
        
    inside_loop_a:
        /* This label is inside loop A but reachable from loop B */
        int c = a * 2;
        result ^= c;
        asm volatile("" : : "r"(c));
        
        /* Jump back to loop B */
        goto loop_b_start;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with do-while inside for */
NOINLINE OPTIMIZE
int mixed_loop_types(int N) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < N) {
        int j = 0;
        
        /* do-while nested inside while */
        do {
            int a = i * j;
            int b = a + global_seed;
            int c = b - i;
            int d = c * j;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            result += d;
            
            j++;
        } while (j < 4);
        
        /* for loop after do-while */
        for (int k = 0; k < 3; ++k) {
            int x = i + k;
            int y = x * result;
            asm volatile("" : : "r"(x), "r"(y));
            result ^= y;
        }
        
        i++;
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE OPTIMIZE
int complex_sibling_loops(int N) {
    int result = 0;
    
    /* Outer loop */
    for (int i = 0; i < N; ++i) {
        /* First sibling inner loop */
        for (int j = 0; j < i % 5 + 1; ++j) {
            int a = i * j + global_seed;
            result += a;
            asm volatile("" : : "r"(a));
        }
        
        /* Intermediate computation */
        int temp = result * 3;
        
        /* Second sibling inner loop - partially overlaps with first via shared temp */
        for (int k = 0; k < 3; ++k) {
            int b = temp + k;
            int c = b * i;
            result ^= c;
            asm volatile("" : : "r"(b), "r"(c));
        }
        
        /* Third loop with break to outer */
        for (int m = 0; m < 2; ++m) {
            if (result > 1000) {
                /* Break creates control flow between loops */
                break;
            }
            result += m * 7;
        }
    }
    
    return result & 0xFF;
}

/* Main function to drive all test cases */
int main(int argc, char *argv[]) {
    int total_result = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 50) + 20;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 10;
    int N4 = (seed % 25) + 5;
    int N5 = (seed % 20) + 8;
    
    /* Call all test functions */
    total_result ^= perfect_nesting(N1);
    total_result ^= loop_subset_of_other(N2);
    total_result ^= overlapping_loops_goto(N3);
    total_result ^= mixed_loop_types(N4);
    total_result ^= complex_sibling_loops(N5);
    
    /* Generate side effect to prevent dead code elimination */
    printf("Result: %d\n", total_result & 0xFF);
    
    return 0;
}
