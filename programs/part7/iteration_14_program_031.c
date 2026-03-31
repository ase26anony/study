/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * Target: ARMv8-A with hardware loop support
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))
#define OPTIMIZE __attribute__((optimize("O2")))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE OPTIMIZE
int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' */
    for (i = 0; i < n; ++i) {
        /* No code here ensures loop has no blocks outside other */
        
        /* Inner loop - this will be 'other' (subset of loop) */
        for (j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = i * j;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            result ^= d;
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE OPTIMIZE
int reverse_nesting(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop - this will be 'other' */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 2; ++j) {
            int a = i * j;
            asm volatile("" : : "r"(a));
            result += a;
        }
        
        /* Second inner loop - this will be 'loop' (subset of other) */
        for (k = 0; k < i; ++k) {
            /* Register pressure */
            int x = i + k;
            int y = i * k;
            int z = y - x;
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
            result ^= (x * y) >> (z & 3);
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partial overlap with goto - shared blocks (Condition 1) */
NOINLINE OPTIMIZE
int partial_overlap_goto(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - will be 'loop' */
    for (i = 0; i < n; ++i) {
        int a = i * 3;
        asm volatile("" : : "r"(a));
        
    shared_label:
        result += a;
        
        /* Second loop - will be 'other' */
        while (j < n) {
            int b = j * 2;
            asm volatile("" : : "r"(b));
            
            if (b > a) {
                /* Jump into first loop's body - creates intersection */
                goto shared_label;
            }
            
            result ^= b;
            j++;
            
            /* Break to avoid infinite loop */
            if (j > i + 5) break;
        }
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex CFG */
NOINLINE OPTIMIZE
int mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* do-while inside for */
    for (i = 0; i < n; ++i) {
        int j = 0;
        int a = i;
        
        do {
            /* Register pressure */
            int b = j * 2;
            int c = a + b;
            int d = c * 3;
            int e = d - a;
            asm volatile("" : : "r"(b), "r"(c), "r"(d), "r"(e));
            
            result ^= e;
            j++;
        } while (j < 3);
        
        /* while loop after for */
        int k = 0;
        while (k < i) {
            int f = k * i;
            asm volatile("" : : "r"(f));
            result += f;
            k++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with break to shared code */
NOINLINE OPTIMIZE
int sibling_loops_break(int n) {
    int result = 0;
    int i, j;
    
    /* First loop */
    for (i = 0; i < n; ++i) {
        int a = i * 2;
        
        /* Second loop - sibling, not nested */
        for (j = 0; j < n; ++j) {
            int b = j * 3;
            asm volatile("" : : "r"(a), "r"(b));
            
            if (a + b > n * 2) {
                /* Break to shared code block */
                goto shared_exit;
            }
            
            result += a - b;
        }
        
        continue;
        
    shared_exit:
        /* Shared block between loops */
        int c = a + 100;
        asm volatile("" : : "r"(c));
        result ^= c;
        break;
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 50) + 20;  /* Ensure loops run */
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N + 5);
    total ^= partial_overlap_goto(N + 3);
    total ^= mixed_loops(N + 2);
    total ^= sibling_loops_break(N + 4);
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
