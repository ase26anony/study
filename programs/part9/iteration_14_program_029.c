/* hwloop-coverage-test.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The test creates specific loop structures to trigger bitmap intersection
 * logic in GCC's hw-doloop.cc discover_loop_hierarchy function.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop (Condition 2)
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure other is subset */
        
        /* Inner loop - this will be 'other' */
        for (j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a ^ j;
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
    
    /* Outer loop - this will be 'other' */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i * j;
            result += a;
            asm volatile("" : : "r"(a));
        }
        
        /* Some intermediate code in 'other' but not in 'loop' */
        int temp = i * 7;
        result ^= temp;
        
        /* Second inner loop - this will be 'loop' (subset of other) */
        for (k = 0; k < i % 5 + 1; ++k) {
            int a = i * k;
            int b = a ^ k;
            int c = b - i;
            result += c;
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
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
        result += a;
        
    shared_label:
        /* This block will be shared between loops */
        int b = a ^ i;
        result ^= b;
        asm volatile("" : : "r"(a), "r"(b));
    }
    
    /* Second loop - will be 'other' */
    while (j < n) {
        int c = j * 5;
        result += c;
        
        /* Jump into the first loop's body */
        if (j % 2 == 0) {
            goto shared_label;
        }
        
        j++;
    }
    
    return result;
}

/* Function 4: Mixed loop types with complex relationships */
NOINLINE int mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* do-while inside for */
    for (i = 0; i < n; ++i) {
        int j = 0;
        
        /* do-while loop */
        do {
            int a = i * j;
            int b = a + j;
            result ^= b;
            asm volatile("" : : "r"(a), "r"(b));
            j++;
        } while (j < 3);
        
        /* while loop after do-while */
        int k = 0;
        while (k < i % 4) {
            int c = i * k * 2;
            result += c;
            asm volatile("" : : "r"(c));
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
    for (i = 0; i < n; ++i) {
        int a = i * 2;
        result += a;
        asm volatile("" : : "r"(a));
    }
    
    /* Second loop with different bound but similar structure */
    for (i = n - 1; i >= 0; --i) {
        int b = i * 3;
        result ^= b;
        asm volatile("" : : "r"(b));
        
        /* This creates potential for shared optimization blocks */
        if (i % 2 == 0) {
            int c = b + i;
            result += c;
            asm volatile("" : : "r"(c));
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile to prevent compile-time computation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N + 5);
    total ^= overlapping_loops(N + 3);
    total ^= mixed_loops(N + 7);
    total ^= sibling_loops(N + 2);
    
    /* Generate side effect to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
