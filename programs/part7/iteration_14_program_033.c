/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop.c
 * For ARM targets with hardware loop support.
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
    volatile int temp;
    
    /* Outer loop (will be 'loop' in the analysis) */
    for (int i = 0; i < n; ++i) {
        /* No code here ensures inner loop blocks are subset of outer */
        
        /* Inner loop (will be 'other' in the analysis) */
        for (int j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a + j;
            int c = b - i;
            int d = c * a;
            int e = d >> 2;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            
            result ^= (a + b + c + d + e) & 0xFF;
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    temp = result;
    return temp;
}

/* Function 2: Loop with subset relationship reversed - loop is subset of other */
NOINLINE int test_reverse_subset(int n) {
    int result = 0;
    
    /* Outer loop (will be 'other' in the analysis) */
    for (int i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 2; ++j) {
            int a = i + j;
            int b = a * 3;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop (will be 'loop' in the analysis) */
        for (int k = 0; k < i; ++k) {
            /* This loop's blocks are subset of outer loop */
            int x = k * i;
            int y = x >> 1;
            int z = y + result;
            
            /* Create more complex operations for register pressure */
            for (int m = 0; m < 3; ++m) {
                z = (z * 1103515245 + 12345) & 0x7FFFFFFF;
            }
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
            result = z & 0xFF;
        }
    }
    
    return result;
}

/* Function 3: Partially overlapping loops using goto */
NOINLINE int test_partial_overlap(int n) {
    int result = 0;
    
    /* First loop (will be 'loop' in some analysis) */
    for (int i = 0; i < n; ++i) {
        int a = i * 2;
        
    shared_block:
        /* This block will be shared between loops */
        result += a;
        asm volatile("" : : "r"(a));
        
        /* Second loop (will be 'other' in some analysis) */
        for (int j = 0; j < 3; ++j) {
            int b = j + i;
            
            /* Jump to shared block in first loop */
            if (j == 1 && (i & 1)) {
                goto shared_block;
            }
            
            result ^= b;
            asm volatile("" : : "r"(b));
        }
    }
    
    return result;
}

/* Function 4: Mixed loop types with complex relationships */
NOINLINE int test_mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < n) {
        /* do-while loop inside while */
        int j = 0;
        do {
            int a = i * j;
            int b = (a << 2) | (a >> 6);
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
            j++;
        } while (j < 5);
        
        /* for loop after do-while */
        for (int k = 0; k < i; k++) {
            /* Nested if to create more basic blocks */
            if (k & 1) {
                result = (result * 1664525 + 1013904223) & 0xFFFF;
            } else {
                result = (result ^ 0xDEADBEEF) & 0xFFFF;
            }
            
            /* Another inner loop */
            for (int m = 0; m < 2; m++) {
                result = result ^ (k << m);
                asm volatile("" : : "r"(result));
            }
        }
        
        i++;
    }
    
    return result;
}

/* Function 5: Sibling loops with shared exit block */
NOINLINE int test_sibling_loops(int n) {
    int result = 0;
    
    /* First sibling loop */
    for (int i = 0; i < n; i += 2) {
        int a = i * i;
        result += a;
        asm volatile("" : : "r"(a));
    }
    
    /* Shared exit block that both loops can reach */
    shared_exit:
    
    /* Second sibling loop that can jump to shared exit */
    for (int j = 1; j < n; j += 2) {
        int b = j * 3;
        result ^= b;
        
        /* Conditional jump to shared exit */
        if (j > n/2) {
            goto shared_exit;
        }
        
        asm volatile("" : : "r"(b));
    }
    
    return result;
}

/* Main function to drive all tests */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    /* Call each test function with different parameters */
    total ^= test_perfect_nesting(N);
    total ^= test_reverse_subset(N + 5);
    total ^= test_partial_overlap(N + 3);
    total ^= test_mixed_loops(N + 7);
    total ^= test_sibling_loops(N + 2);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return total & 0xFF;
}
