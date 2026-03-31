/* test_hwloop.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates complex loop structures with specific block relationships
 * to trigger bitmap intersection logic in GCC's hardware loop optimization.
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
int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in the analysis) */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure inner loop is perfect subset */
        
        /* Inner loop (will be 'other' in the analysis) */
        for (j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a ^ j;
            int c = b - i;
            int d = c * a;
            result ^= (d >> 2) & 0xFF;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
        }
        
        /* No code here either - inner loop is perfect subset */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE OPTIMIZE
int reverse_nesting(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in the analysis) */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            result += i * j;
            asm volatile("" : : "r"(result));
        }
        
        /* Second inner loop (will be 'loop' in the analysis) */
        for (k = 0; k < i; ++k) {
            /* Complex body for register pressure */
            int a = i ^ k;
            int b = k * 7;
            int c = a - b;
            int d = c >> 1;
            result ^= d;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops via goto
 * This should trigger the first condition (bitmap_intersect_p)
 * but not the subset conditions
 */
NOINLINE OPTIMIZE
int overlapping_loops(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop */
    while (i < n) {
        int a = i * 3;
        
        if (a % 7 == 0) {
            /* Jump into second loop's body */
            goto shared_block;
        }
        
        result += a;
        i++;
        continue;
        
    shared_block:
        /* This block is shared between both loops */
        result ^= 0x55;
        
        /* Second loop - do-while style */
        do {
            int b = j * 5;
            result -= b;
            j++;
            
            if (j >= n/2) {
                /* Break back to first loop */
                i++;
                break;
            }
        } while (1);
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with sibling relationships */
NOINLINE OPTIMIZE
int mixed_loops(int n) {
    int result = 0;
    int i, j;
    
    /* for loop */
    for (i = 0; i < n; i += 2) {
        /* while loop inside */
        j = 0;
        while (j < i) {
            int a = i ^ j;
            int b = a * 11;
            result += b % 256;
            j++;
            asm volatile("" : : "r"(a), "r"(b));
        }
    }
    
    /* Separate do-while loop that follows */
    i = n - 1;
    if (i > 0) {
        do {
            result ^= i * 3;
            i--;
            asm volatile("" : : "r"(i));
        } while (i > 0);
    }
    
    return result & 0xFF;
}

/* Function 5: Complex hierarchy with multiple levels */
NOINLINE OPTIMIZE
int complex_hierarchy(int n) {
    int result = 0;
    int a, b, c;
    
    /* Level 1: Outer loop */
    for (a = 0; a < n; a++) {
        /* Level 2: Middle loop 1 */
        for (b = 0; b < a; b++) {
            /* Level 3: Innermost loop */
            for (c = 0; c < b; c++) {
                int x = a * b + c;
                int y = x ^ (a + b + c);
                result += y & 0xF;
                asm volatile("" : : "r"(x), "r"(y));
            }
            
            /* Extra code in middle loop creates non-perfect nesting */
            if (b % 3 == 0) {
                result ^= 0xAA;
            }
        }
        
        /* Level 2: Middle loop 2 (sibling) */
        for (b = n - 1; b > a; b--) {
            int z = a * b;
            result -= z % 128;
            asm volatile("" : : "r"(z));
        }
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and command line to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N + 5);
    total ^= overlapping_loops(N + 3);
    total ^= mixed_loops(N + 7);
    total ^= complex_hierarchy(N + 2);
    
    /* Generate side effect to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return (total & 0xFF) == 0 ? 0 : 1;
}
