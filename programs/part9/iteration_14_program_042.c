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

/* Function 1: Perfectly nested loops - other is subset of loop
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE int test_perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (i = 0; i < n; ++i) {
        /* Inner loop (will be 'other' in hierarchy) */
        for (j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a + j;
            int c = b - i;
            int d = c * 3;
            int e = d >> 2;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            
            result ^= (a + b + c + d + e) & 0xFF;
        }
        
        /* Small computation to ensure loop has blocks outside inner loop */
        result += i & 1;
    }
    
    return result;
}

/* Function 2: Loop with subset relationship reversed - loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE int test_reverse_subset(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i * j;
            int b = a << 2;
            asm volatile("" : : "r"(a), "r"(b));
            result += b;
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (k = 0; k < i; ++k) {
            /* Create more complex body for 'loop' */
            int x = k * 7;
            int y = x ^ i;
            int z = y - k;
            int w = z * 11;
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z), "r"(w));
            result ^= w;
        }
        
        /* More code in outer loop after 'loop' */
        result -= i * 2;
    }
    
    return result;
}

/* Function 3: Partially overlapping loops via goto
 * This should trigger the first condition (bitmap_intersect_p returns true)
 * but not the subset conditions
 */
NOINLINE int test_partial_overlap(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* Loop A */
    for (i = 0; i < n; ++i) {
        int a = i * 3;
        
    shared_block:
        /* This label creates a shared basic block */
        result += a & 0xF;
        
        /* Loop B - shares the labeled block via goto */
        if (j < n / 2) {
            for (; j < n / 2; ++j) {
                int b = j * 5;
                result ^= b;
                
                /* Jump to shared block in Loop A */
                if (j == i && i > 0)
                    goto shared_block;
            }
        }
        
        /* More computations in Loop A */
        result -= i;
    }
    
    return result;
}

/* Function 4: Mixed loop types (do-while inside for) */
NOINLINE int test_mixed_loops(int n) {
    int result = 0;
    int i;
    
    /* Outer for loop */
    for (i = 0; i < n; ++i) {
        int count = i % 5;
        
        /* Inner do-while loop */
        if (count > 0) {
            do {
                int a = count * 7;
                int b = a ^ result;
                int c = b << 1;
                
                asm volatile("" : : "r"(a), "r"(b), "r"(c));
                result = c & 0xFF;
                
                count--;
            } while (count > 0);
        }
        
        /* While loop after do-while */
        int temp = result;
        while (temp > 0) {
            temp >>= 1;
            result ^= temp;
        }
    }
    
    return result;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int test_sibling_loops(int n) {
    int result = 0;
    int i, j, k;
    
    /* Level 1 loop */
    for (i = 0; i < n; ++i) {
        /* Level 2 - first sibling */
        for (j = 0; j < i + 2; ++j) {
            int a = i * j * 13;
            result += a;
        }
        
        /* Level 2 - second sibling (adjacent, not nested) */
        for (k = 0; k < (n - i); ++k) {
            int b = k * 17;
            int c = b ^ i;
            int d = c - k;
            
            asm volatile("" : : "r"(b), "r"(c), "r"(d));
            result ^= d;
        }
        
        /* Another level 2 loop */
        int m = 0;
        while (m < 3) {
            result += (i * m) & 1;
            m++;
        }
    }
    
    return result;
}

/* Main function to drive all tests */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile/argc to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    printf("Running hardware loop coverage tests with N=%d\n", N);
    
    /* Run all test functions */
    total ^= test_perfect_nesting(N);
    total ^= test_reverse_subset(N);
    total ^= test_partial_overlap(N);
    total ^= test_mixed_loops(N);
    total ^= test_sibling_loops(N);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 255);
    
    return (total & 255) == 0 ? 0 : 1;
}
