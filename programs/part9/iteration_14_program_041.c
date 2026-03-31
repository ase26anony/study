/* test_hwloop_coverage.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates specific loop structures to trigger bitmap intersection
 * logic in GCC's hardware loop optimization pass.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop (Condition 2)
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' in the hierarchy */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure other is a perfect subset */
        
        /* Inner loop - this will be 'other' in the hierarchy */
        for (j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a + j;
            int c = b - i;
            int d = c * a;
            result ^= (d >> 2) & 0xFF;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3)
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE int reverse_nesting(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop - this will be 'other' in the hierarchy */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i + j;
            int b = a * 2;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop - this will be 'loop' in the hierarchy */
        for (k = 0; k < i; ++k) {
            /* This loop is a subset of the outer loop */
            int x = i * k;
            int y = x - k;
            int z = y >> 1;
            result ^= z;
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
        }
    }
    
    return result;
}

/* Function 3: Partially overlapping loops with goto (Condition 1)
 * Loops share blocks but neither is subset of the other
 */
NOINLINE int overlapping_loops(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - will be 'loop' */
    for (i = 0; i < n; ++i) {
        int a = i * 3;
        
    shared_label:
        /* This block will be shared between both loops */
        result += a;
        asm volatile("" : : "r"(a));
        
        /* Second loop - will be 'other' */
        while (j < n) {
            int b = j * 2;
            
            /* Jump into the first loop's body */
            if (j == i && i > n/2) {
                goto shared_label;
            }
            
            result ^= b;
            asm volatile("" : : "r"(b));
            j++;
        }
        
        /* More operations to create distinct blocks */
        int c = result * i;
        result = c & 0xFFFF;
    }
    
    return result;
}

/* Function 4: Mixed loop types with complex control flow */
NOINLINE int mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* do-while inside a for loop */
    for (i = 0; i < n; i++) {
        int j = 0;
        
        /* do-while loop */
        do {
            int a = i + j;
            int b = a * j;
            int c = b - i;
            result += c;
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            j++;
        } while (j < 5);
        
        /* while loop after do-while */
        int k = 0;
        while (k < i) {
            int x = result ^ k;
            int y = x * 3;
            result = y & 0xFF;
            asm volatile("" : : "r"(x), "r"(y));
            k++;
        }
    }
    
    return result;
}

/* Function 5: Sibling loops with shared header block */
NOINLINE int sibling_loops(int n) {
    int result = 0;
    int i;
    
    /* Two sequential loops that might share some CFG structure */
    for (i = 0; i < n; i++) {
        int a = i * 7;
        result += a;
        asm volatile("" : : "r"(a));
    }
    
    /* Second loop with similar structure */
    for (i = n-1; i >= 0; i--) {
        int b = i * 11;
        result ^= b;
        asm volatile("" : : "r"(b));
        
        /* Inner loop to create hierarchy */
        for (int j = 0; j < 2; j++) {
            int c = result + j;
            result = c & 0x7F;
        }
    }
    
    return result;
}

/* Main function to drive all test cases */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 50) + 20;  /* Ensure loops run enough iterations */
    
    printf("Running hardware loop coverage tests with N=%d\n", N);
    
    /* Call all test functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N);
    total ^= overlapping_loops(N);
    total ^= mixed_loops(N);
    total ^= sibling_loops(N);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
