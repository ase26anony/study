/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * Target: ARMv8-A with hardware loop support (or other targets with -doloop)
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfectly nested loops - other is subset of loop */
/* Should trigger: loop->loops.safe_push(other) */
NOINLINE int test_perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in the analysis) */
    for (i = 0; i < n; ++i) {
        /* No code here ensures loop has no blocks outside other */
        
        /* Inner loop (will be 'other' in the analysis) */
        for (j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = i * j;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= d;
        }
        
        /* No code here either - ensures perfect nesting */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop with sibling inner loops - loop is subset of other */
/* Should trigger: other->loops.safe_push(loop) */
NOINLINE int test_sibling_inner_loops(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in the analysis) */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 5; ++j) {
            int a = i * j;
            int b = a + global_seed;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop (will be 'loop' in the analysis) */
        /* This loop's blocks are a subset of outer's blocks */
        for (k = 0; k < i; ++k) {
            int x = i ^ k;
            int y = x * global_seed;
            int z = y >> (k & 3);
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
            result ^= z;
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops via goto */
/* Should trigger bitmap intersection but not subset relationship */
NOINLINE int test_partial_overlap(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop (could be 'loop' or 'other') */
    while (i < n) {
        int a = i * 3;
        
        /* Second loop with goto into first loop's body */
        do {
            int b = j * 2;
            result += b;
            
            if (j == n/2) {
                /* Jump to label inside first loop */
                goto inside_first_loop;
            }
            
            j++;
        } while (j < n);
        
        inside_first_loop:
        result ^= a;
        i++;
        
        /* Reset j for next iteration */
        if (i % 2 == 0) {
            j = 0;
        }
    }
    
    return result & 0xFF;
}

/* Function 4: Complex nested structure with mixed loop types */
NOINLINE int test_mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* for loop */
    for (i = 0; i < n; i++) {
        int j = 0;
        
        /* do-while loop inside for loop */
        do {
            int k = 0;
            
            /* while loop inside do-while */
            while (k < 3) {
                int a = i + j + k;
                int b = a * global_seed;
                int c = b >> (a & 7);
                
                /* Create significant register pressure */
                int d = c * i;
                int e = d ^ j;
                int f = e + k;
                int g = f - global_seed;
                int h = g * 2;
                
                asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), 
                             "r"(e), "r"(f), "r"(g), "r"(h));
                
                result += h;
                k++;
            }
            
            j++;
        } while (j < 5);
        
        /* Additional computation to create more blocks */
        if (i % 3 == 0) {
            int x = result * i;
            asm volatile("" : : "r"(x));
            result = x & 0xFF;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Adjacent loops with shared basic block via break */
NOINLINE int test_adjacent_loops(int n) {
    int result = 0;
    int i, j;
    
    /* First loop */
    for (i = 0; i < n; i++) {
        int a = i * 2;
        
        /* Second loop that can break into shared code */
        for (j = 0; j < n; j++) {
            int b = j * 3;
            result += a + b;
            
            if (result > 1000) {
                /* Break to shared code block */
                goto shared_block;
            }
        }
        
        continue;
        
        shared_block:
        /* This block is shared between both loops */
        int c = result * global_seed;
        asm volatile("" : : "r"(c));
        result = c & 0xFF;
        
        /* Early exit from first loop */
        if (i > n/2) break;
    }
    
    return result & 0xFF;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    /* Call all test functions with different parameters */
    total ^= test_perfect_nesting(N);
    total ^= test_sibling_inner_loops(N + 5);
    total ^= test_partial_overlap(N + 3);
    total ^= test_mixed_loops(N + 7);
    total ^= test_adjacent_loops(N + 2);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
