/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * Target: ARMv8-A with hardware loop support
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfectly nested loops - other is subset of loop */
NOINLINE int test_perfect_nesting(int n) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' in the analysis */
    for (int i = 0; i < n; ++i) {
        /* Inner loop - this will be 'other' in the analysis */
        for (int j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = i * j;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= d;
        }
        
        /* Small amount of code in outer loop but not in inner loop */
        if (i % 2 == 0) {
            result += i;
        }
    }
    
    return result & 0xFF;
}

/* Function 2: Loop with subset relationship reversed - loop is subset of other */
NOINLINE int test_reverse_subset(int n) {
    int result = 0;
    
    /* Outer loop - this will be 'other' in the analysis */
    for (int i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' but not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            int a = i * j;
            int b = a + j;
            result ^= (a * b) & 0xFF;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop - this will be 'loop' in the analysis */
        for (int k = 0; k < i; ++k) {
            /* Create more complex operations for register pressure */
            int x = i ^ k;
            int y = k * 7;
            int z = x - y;
            int w = (x * y) >> (z & 3);
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z), "r"(w));
            result += w;
        }
        
        /* More code in outer loop after both inner loops */
        result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return result & 0xFF;
}

/* Function 3: Loops with partial overlap using goto */
NOINLINE int test_partial_overlap(int n) {
    int result = 0;
    int i = 0;
    
    /* First loop - will be 'other' */
    while (i < n) {
        int j = 0;
        
        /* Second loop - will be 'loop' */
        do {
            /* Shared computation */
            int a = i * j;
            int b = a + (i ^ j);
            
            /* Conditional goto creates shared basic block */
            if ((i + j) % 5 == 0) {
                goto shared_block;
            }
            
            result ^= b;
            j++;
        } while (j < 5);
        
        i++;
        continue;
        
    shared_block:
        /* This block is shared between both loops */
        result = (result * 31 + 17) & 0xFF;
        i++;
    }
    
    return result;
}

/* Function 4: Mixed loop types with complex nesting */
NOINLINE int test_mixed_loops(int n) {
    int result = 0;
    
    /* Outer for loop */
    for (int i = 0; i < n; i++) {
        /* Inner while loop */
        int j = 0;
        while (j < i) {
            /* Innermost do-while */
            int k = 0;
            do {
                /* Heavy computation for register pressure */
                int a = i + j + k;
                int b = i * j - k;
                int c = (a << 3) | (b & 7);
                int d = c ^ (a * b);
                int e = d >> (k & 3);
                
                asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
                result += e;
                k++;
            } while (k < 3);
            
            j++;
        }
        
        /* Another inner for loop in sequence */
        for (int m = 0; m < 2; m++) {
            result = (result * 3 + m) & 0xFFF;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Complex control flow with breaks and continues */
NOINLINE int test_complex_flow(int n) {
    int result = 0;
    
    /* Loop A */
    for (int i = 0; i < n; i++) {
        /* Loop B - partially overlapping */
        for (int j = 0; j < n; j++) {
            if (i == j) {
                /* Break to create shared exit edge */
                break;
            }
            
            /* Loop C - deeply nested */
            for (int k = 0; k < 3; k++) {
                int val = (i * j * k) & 0xFF;
                
                /* Conditional continue */
                if (val % 7 == 0) {
                    result += val;
                    continue;
                }
                
                /* More operations */
                result ^= val;
                asm volatile("" : : "r"(val));
            }
            
            /* Early exit creates more CFG edges */
            if (result > 1000) {
                goto early_exit;
            }
        }
        
        /* Label for goto target */
        early_exit:
        if (i % 10 == 0) {
            result = result / 2;
        }
    }
    
    return result & 0xFF;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 50) + 20;  /* Ensure loops run enough iterations */
    
    /* Call all test functions */
    total += test_perfect_nesting(N);
    total += test_reverse_subset(N);
    total += test_partial_overlap(N);
    total += test_mixed_loops(N);
    total += test_complex_flow(N);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    /* Additional runs with different parameters */
    for (int i = 0; i < 3; i++) {
        total += test_perfect_nesting(N + i);
        total += test_reverse_subset(N - i);
    }
    
    return total & 1;  /* Return 0 or 1 based on computation */
}
