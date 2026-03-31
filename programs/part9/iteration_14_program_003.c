/* test_hwloop_coverage.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates complex loop structures with specific block relationships
 * to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Force specific optimization level */
#define OPTIMIZE_O2 __attribute__((optimize("O2")))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE OPTIMIZE_O2
int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure inner loop is perfect subset */
        
        /* Inner loop - this will be 'other' */
        for (j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = (i << 2) + (j << 1);
            int c = b - a;
            int d = (a * b) >> (c & 3);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= d;
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE OPTIMIZE_O2
int reverse_nesting(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop - this will be 'other' */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int temp = i * j * 7;
            result += temp;
            asm volatile("" : : "r"(temp));
        }
        
        /* Second inner loop - this will be 'loop' (subset of other) */
        for (k = 0; k < i + 2; ++k) {
            /* Complex body for register pressure */
            int a = i ^ k;
            int b = k * 3;
            int c = (a << 1) | (b >> 1);
            int d = c * a - b;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            result ^= d;
        }
        
        /* More code in outer loop after inner loops */
        result += i * 11;
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto
 * This should trigger the first condition (bitmap_intersect_p is true)
 * but not the subset conditions
 */
NOINLINE OPTIMIZE_O2
int overlapping_loops(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - will be 'loop' */
    for (i = 0; i < n; ++i) {
        int a = i * 3;
        result += a;
        
        if (i == n/2) {
            /* Jump into second loop's body */
            goto shared_block;
        }
    }
    
    /* Second loop - will be 'other' */
    while (j < n) {
        shared_block:  /* Shared basic block */
        int b = j * 5;
        result ^= b;
        
        /* Create register pressure */
        int x = b << 1;
        int y = x - j;
        int z = y * x;
        
        asm volatile("" : : "r"(x), "r"(y), "r"(z));
        
        j++;
        
        if (j > n/2) {
            /* Break back to first loop's domain */
            break;
        }
    }
    
    /* Continue with first loop's logic */
    for (; i < n; ++i) {
        result += i * 7;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex control flow
 * Creates various loop relationships
 */
NOINLINE OPTIMIZE_O2
int mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* do-while inside for */
    for (i = 0; i < n; i += 2) {
        int j = 0;
        
        /* do-while loop */
        do {
            int a = i + j;
            int b = a * 2;
            int c = b - j;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            result ^= c;
            
            j++;
        } while (j < 3);
        
        /* while loop after do-while */
        int k = 0;
        while (k < i) {
            result += k * 13;
            k++;
        }
    }
    
    /* Another for loop that shares some blocks via switch */
    for (int m = 0; m < n; m++) {
        switch (m % 4) {
            case 0:
                result += m;
                break;
            case 1:
                result ^= m * 2;
                /* Fall through to shared code */
            case 2:
            case 3:
                /* Shared block between loops */
                int temp = m * 3;
                asm volatile("" : : "r"(temp));
                result -= temp;
                break;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with shared header
 * Creates adjacent loops that might be discovered in different orders
 */
NOINLINE OPTIMIZE_O2
int sibling_loops(int n) {
    int result = 0;
    
    /* Two sequential loops that are siblings */
    for (int i = 0; i < n; i++) {
        int a = i * 17;
        result += a;
        asm volatile("" : : "r"(a));
    }
    
    /* This loop starts where previous one ends */
    for (int j = 0; j < n/2; j++) {
        int b = j * 19;
        result ^= b;
        
        /* Nested loop inside second sibling */
        for (int k = 0; k < 2; k++) {
            int c = b + k;
            asm volatile("" : : "r"(c));
            result -= c;
        }
    }
    
    /* Third loop that partially overlaps with first via control flow */
    int m = 0;
    while (m < n) {
        if (m % 3 == 0) {
            /* This block might be shared if compiler optimizes */
            result += 23;
        }
        m++;
    }
    
    return result & 0xFF;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    /* Also use global seed */
    N = (N + global_seed) % 50 + 20;
    
    printf("Running with N=%d\n", N);
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N + 1);
    total ^= overlapping_loops(N + 2);
    total ^= mixed_loops(N + 3);
    total ^= sibling_loops(N + 4);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return total & 1;  /* Return 0 or 1 based on computation */
}
