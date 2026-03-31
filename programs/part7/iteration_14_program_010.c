/* test_hwloop.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates complex loop structures to trigger bitmap intersection
 * logic in GCC's hardware loop optimization pass (hw-doloop.cc lines 429-436).
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE_COLD __attribute__((noinline, cold))

/* Force specific optimization level */
#define OPTIMIZE_O2 __attribute__((optimize("O2")))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Prevent dead code elimination */
static int global_result = 0;

/* Function 1: Perfect nesting - other is subset of loop
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE_COLD OPTIMIZE_O2
int func_perfect_nesting(int N) {
    int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' */
    for (i = 0; i < N; ++i) {
        /* No code here to ensure inner loop is perfect subset */
        
        /* Inner loop - this will be 'other' */
        for (j = 0; j < N/2; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = i * 2 - j;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            
            /* Prevent optimization */
            asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= d;
        }
        
        /* No code here either - inner loop is perfect subset */
    }
    
    return result;
}

/* Function 2: Loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE_COLD OPTIMIZE_O2  
int func_loop_subset_of_other(int N) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop - this will be 'other' */
    for (i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i * j;
            int b = a + 1;
            result += b;
            asm volatile ("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop - this will be 'loop' (subset of 'other') */
        for (k = 0; k < N/3; ++k) {
            /* Create register pressure */
            int x = i + k;
            int y = k * 3;
            int z = x ^ y;
            int w = (z << 2) | (x & 3);
            
            asm volatile ("" : : "r"(x), "r"(y), "r"(z), "r"(w));
            result ^= w;
        }
    }
    
    return result;
}

/* Function 3: Loops with partial overlap using goto
 * This should trigger the first condition (bitmap_intersect_p = true)
 * but not the subset conditions
 */
NOINLINE_COLD OPTIMIZE_O2
int func_partial_overlap_goto(int N) {
    int result = 0;
    int i, j;
    
    /* Loop A */
    for (i = 0; i < N; ++i) {
        int a = i * 2;
        
    loop_b_start:
        /* Loop B - shares block via goto */
        for (j = 0; j < N/2; ++j) {
            int b = j + 1;
            result += a * b;
            asm volatile ("" : : "r"(a), "r"(b));
            
            /* Jump into loop A's body */
            if (j == N/4) {
                goto shared_block;
            }
        }
        
        continue;
        
    shared_block:
        /* This block is shared between both loops */
        int c = a + i;
        result ^= c;
        asm volatile ("" : : "r"(c));
        
        if (i < N/2) {
            goto loop_b_start;
        }
    }
    
    return result;
}

/* Function 4: Mixed loop types with while and do-while */
NOINLINE_COLD OPTIMIZE_O2
int func_mixed_loop_types(int N) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < N) {
        int j = 0;
        
        /* do-while loop inside while */
        do {
            int a = i * j;
            int b = a + (j << 3);
            int c = b ^ a;
            
            asm volatile ("" : : "r"(a), "r"(b), "r"(c));
            result += c;
            
            j++;
        } while (j < 5);
        
        i++;
    }
    
    /* for loop after while */
    for (int k = 0; k < N/2; ++k) {
        int x = k * 3;
        int y = x - result;
        
        asm volatile ("" : : "r"(x), "r"(y));
        result ^= y;
    }
    
    return result;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE_COLD OPTIMIZE_O2
int func_complex_siblings(int N) {
    int result = 0;
    
    /* Outer loop */
    for (int i = 0; i < N; ++i) {
        /* Sibling loop A */
        for (int j = 0; j < 2; ++j) {
            int a = i + j * 7;
            result += a;
            asm volatile ("" : : "r"(a));
        }
        
        /* Some intermediate code */
        int temp = result & 0xFF;
        
        /* Sibling loop B - partially overlaps with outer via temp */
        for (int k = 0; k < N/4; ++k) {
            int b = temp + k;
            int c = b * 3;
            
            asm volatile ("" : : "r"(b), "r"(c));
            result ^= c;
        }
        
        /* More intermediate code */
        temp = result >> 4;
        
        /* Another loop */
        int m = 0;
        while (m < 3) {
            int d = temp * m;
            asm volatile ("" : : "r"(d));
            result += d;
            m++;
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    /* Also use global seed */
    N = (N + global_seed) % 100 + 20;
    
    printf("Testing with N = %d\n", N);
    
    /* Call all functions to ensure they're compiled and executed */
    int r1 = func_perfect_nesting(N);
    int r2 = func_loop_subset_of_other(N);
    int r3 = func_partial_overlap_goto(N);
    int r4 = func_mixed_loop_types(N);
    int r5 = func_complex_siblings(N);
    
    /* Combine results to prevent elimination */
    global_result = r1 ^ r2 ^ r3 ^ r4 ^ r5;
    
    /* Use the result */
    printf("Result: %d\n", global_result & 255);
    
    return 0;
}
