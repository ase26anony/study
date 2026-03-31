/* 
 * Test program for hardware loop hierarchy discovery in GCC.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))
#define OPTIMIZE __attribute__((optimize("O2")))

/* Use volatile to prevent constant propagation */
volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE OPTIMIZE
int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure inner loop is perfect subset */
        
        /* Inner loop (will be 'other' in hierarchy) */
        for (j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = (i << 3) ^ j;
            int c = a - b;
            int d = c * (i + j);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= (a * b) >> (c & 7);
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other */
NOINLINE OPTIMIZE
int reverse_nesting(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 5; ++j) {
            int a = i + j;
            int b = a * 3;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (k = 0; k < i; ++k) {
            /* This loop is subset of outer loop */
            int x = i * k;
            int y = x ^ result;
            int z = y - k;
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
            result ^= (x * y) >> (z & 3);
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto */
NOINLINE OPTIMIZE
int overlapping_with_goto(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* Loop A */
    for (i = 0; i < n; ++i) {
        int a = i * 2;
        result += a;
        
        /* Loop B that shares blocks via goto */
        if (i > n/2) {
            j = 0;
            do {
                int b = j * 3;
                result ^= b;
                
                if (j == i/2) {
                    /* Jump into Loop A's body */
                    goto shared_block;
                }
                
                j++;
            } while (j < 10);
        }
        
        shared_block:
        /* This block is shared between loops */
        int c = result * i;
        asm volatile("" : : "r"(c));
        result = c & 0xFFFF;
    }
    
    return result & 0xFF;
}

/* Function 4: Complex mixed loop types */
NOINLINE OPTIMIZE
int mixed_loop_types(int n) {
    int result = 0;
    int i = 0;
    
    /* for loop */
    for (i = 0; i < n; ++i) {
        int j = 0;
        
        /* while loop inside for */
        while (j < 5) {
            int a = i + j;
            int b = a * 7;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
            j++;
        }
        
        /* do-while loop inside for */
        int k = 0;
        do {
            int x = i * k;
            int y = x ^ result;
            result = y;
            asm volatile("" : : "r"(x), "r"(y));
            k++;
        } while (k < 3);
    }
    
    /* Separate while loop that follows for loop */
    int m = 0;
    while (m < n/2) {
        result ^= m * 11;
        asm volatile("" : : "r"(m));
        m++;
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with shared condition */
NOINLINE OPTIMIZE
int sibling_loops(int n) {
    int result = 0;
    int i;
    
    /* Two sequential loops that might intersect in discovery */
    for (i = 0; i < n; ++i) {
        int a = i * 13;
        result += a;
        asm volatile("" : : "r"(a));
    }
    
    /* Second loop that uses same induction variable */
    i = 0;
    while (i < n) {
        int b = result - i;
        result = b ^ (i * 17);
        asm volatile("" : : "r"(b));
        i++;
        
        /* This creates potential intersection in CFG analysis */
        if (i == n/2) {
            /* Jump back to first loop's concept */
            result ^= 0xAA;
        }
    }
    
    return result & 0xFF;
}

/* Main driver that calls all functions */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    /* Call each function multiple times with different parameters */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N + 5);
    total ^= overlapping_with_goto(N + 3);
    total ^= mixed_loop_types(N + 7);
    total ^= sibling_loops(N + 2);
    
    /* Ensure result is used */
    printf("Result: %d\n", total & 255);
    
    return 0;
}
