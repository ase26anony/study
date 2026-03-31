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

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop
 * Triggers: loop->loops.safe_push(other)
 */
NOINLINE int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop (loop) */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure inner loop blocks are subset */
        
        /* Inner loop (other) - perfectly nested */
        for (j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a ^ j;
            int c = b - i;
            int d = c * a;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= (a + b + c + d) & 0xFF;
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result;
}

/* Function 2: Reverse nesting - loop is subset of other
 * Triggers: other->loops.safe_push(loop)
 */
NOINLINE int reverse_nesting(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (other) */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i * j;
            result += a;
            asm volatile("" : : "r"(a));
        }
        
        /* Second inner loop (loop) - subset of 'other' */
        for (k = 0; k < i + 2; ++k) {
            /* Complex body for register pressure */
            int x = i * k;
            int y = x ^ k;
            int z = y - i;
            int w = z * x;
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z), "r"(w));
            result ^= (x + y + z + w) & 0xFF;
        }
        
        /* More code in outer loop after inner loops */
        result += i * 7;
    }
    
    return result;
}

/* Function 3: Partially overlapping loops via goto
 * Triggers: bitmap_intersect_p returns true, but both intersect_compl_p
 *           return false (loops partially overlap)
 */
NOINLINE int overlapping_loops(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop (loop) */
    for (i = 0; i < n; ++i) {
        int a = i * 3;
        result += a;
        
        /* Label that can be jumped to from second loop */
        shared_block:
        int b = a ^ 0x55;
        result ^= b;
        
        asm volatile("" : : "r"(a), "r"(b));
    }
    
    /* Second loop (other) - shares block via goto */
    while (j < n) {
        int c = j * 5;
        result += c;
        
        if (c % 7 == 0) {
            /* Jump into first loop's body */
            goto shared_block;
        }
        
        j++;
        asm volatile("" : : "r"(c));
    }
    
    return result;
}

/* Function 4: Mixed loop types with complex control flow
 * Creates varied CFG structures */
NOINLINE int mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* do-while inside for */
    for (i = 0; i < n; ++i) {
        int j = 0;
        
        /* do-while loop */
        do {
            int a = i * j;
            int b = a ^ j;
            result += a - b;
            j++;
            asm volatile("" : : "r"(a), "r"(b));
        } while (j < 5);
        
        /* while loop after do-while */
        int k = 0;
        while (k < i) {
            int c = i * k * 3;
            result ^= c;
            k++;
            asm volatile("" : : "r"(c));
        }
    }
    
    return result;
}

/* Function 5: Sibling loops with shared header
 * Two loops that share entry block but have different bodies */
NOINLINE int sibling_loops(int n) {
    int result = 0;
    int i;
    
    /* Shared initialization */
    i = 0;
    
    /* First loop */
    for (; i < n/2; ++i) {
        int a = i * 2;
        result += a;
        asm volatile("" : : "r"(a));
    }
    
    /* Reset i for second loop */
    i = n/2;
    
    /* Second loop starting from same basic block as first */
    while (i < n) {
        int b = i * 3;
        result ^= b;
        i++;
        asm volatile("" : : "r"(b));
    }
    
    return result;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char **argv) {
    int total = 0;
    int iterations;
    
    /* Use volatile/argc to prevent constant propagation */
    volatile int seed = argc;
    iterations = (seed % 50) + 20;  /* 20-69 iterations */
    
    printf("Running with %d iterations\n", iterations);
    
    /* Call all test functions */
    total += perfect_nesting(iterations);
    total += reverse_nesting(iterations);
    total += overlapping_loops(iterations);
    total += mixed_loops(iterations);
    total += sibling_loops(iterations);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return (total & 0xFF) == 0 ? 0 : 1;
}
