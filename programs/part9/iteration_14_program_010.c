/* test_hwloop.c
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

/* Function 1: Perfect nesting - other is subset of loop
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' */
    for (i = 0; i < n; ++i) {
        /* Inner loop - this will be 'other' (subset of outer) */
        for (j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = i * j;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= d;
        }
        
        /* Small amount of code in outer loop but not in inner */
        if (i & 1) {
            result += i;
        }
    }
    
    return result;
}

/* Function 2: Reverse nesting - loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE int reverse_nesting(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop - this will be 'other' */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' but not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i * j;
            int b = a + 1;
            result ^= b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop - this will be 'loop' (subset of outer) */
        for (k = 0; k < i; ++k) {
            /* Complex body for register pressure */
            int x = i + k;
            int y = i * k;
            int z = y - x;
            int w = (x * y) >> (z & 3);
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z), "r"(w));
            result += w;
        }
        
        /* More code in outer loop after inner loops */
        result -= i;
    }
    
    return result;
}

/* Function 3: Partially overlapping loops with goto
 * This should trigger the first condition (bitmap_intersect_p) but not
 * the subset conditions
 */
NOINLINE int overlapping_loops(int n) {
    int result = 0;
    int i, j;
    
    /* First loop - will be 'loop' */
    for (i = 0; i < n; ++i) {
        int a = i * 2;
        result += a;
        
    shared_label:
        /* This block will be shared via goto */
        int b = i + 1;
        result ^= b;
        
        /* Second loop - will be 'other' */
        for (j = 0; j < 5; ++j) {
            int c = i + j;
            int d = c * 3;
            
            /* Jump into first loop's body */
            if (j == 3 && i < n/2) {
                goto shared_label;
            }
            
            asm volatile("" : : "r"(c), "r"(d));
            result += d;
        }
        
        /* Prevent tail merging */
        if (i & 2) {
            result -= 1;
        }
    }
    
    return result;
}

/* Function 4: Mixed loop types (do-while inside for) */
NOINLINE int mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* Outer for loop */
    for (i = 0; i < n; ++i) {
        int count = i % 10;
        
        /* Inner do-while loop */
        if (count > 0) {
            do {
                int a = count * 3;
                int b = a + i;
                int c = b - count;
                
                asm volatile("" : : "r"(a), "r"(b), "r"(c));
                result ^= (a * b) >> (c & 7);
                
                count--;
            } while (count > 0);
        }
        
        /* While loop after do-while */
        int temp = i;
        while (temp > 0) {
            int x = temp * 2;
            int y = x + result;
            asm volatile("" : : "r"(x), "r"(y));
            result = y & 0xFF;
            temp >>= 1;
        }
    }
    
    return result;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int sibling_loops(int n) {
    int result = 0;
    
    /* Outer loop containing two independent inner loops */
    for (int outer = 0; outer < n; ++outer) {
        /* First sibling loop */
        for (int inner1 = 0; inner1 < outer; ++inner1) {
            int a = outer + inner1;
            int b = a * inner1;
            asm volatile("" : : "r"(a), "r"(b));
            result += b;
        }
        
        /* Code between sibling loops (in outer but not in either inner) */
        result ^= outer;
        
        /* Second sibling loop */
        for (int inner2 = 0; inner2 < (outer % 7); ++inner2) {
            int c = outer - inner2;
            int d = c * 5;
            int e = d >> (inner2 & 3);
            asm volatile("" : : "r"(c), "r"(d), "r"(e));
            result ^= e;
        }
        
        /* More outer loop code */
        if (outer & 1) {
            result <<= 1;
        }
    }
    
    return result;
}

/* Function 6: Disjoint loops (should not intersect) */
NOINLINE int disjoint_loops(int n) {
    int result = 0;
    
    /* First independent loop */
    for (int i = 0; i < n; i += 2) {
        int a = i * 3;
        asm volatile("" : : "r"(a));
        result += a;
    }
    
    /* Unrelated code between loops */
    result = (result * 13) & 0xFF;
    
    /* Second independent loop (different induction variable) */
    for (int j = 1; j < n; j += 2) {
        int b = j * 7;
        asm volatile("" : : "r"(b));
        result ^= b;
    }
    
    return result;
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    printf("Testing hardware loop coverage with N=%d\n", N);
    
    /* Call all test functions */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N);
    total ^= overlapping_loops(N);
    total ^= mixed_loops(N);
    total ^= sibling_loops(N);
    total ^= disjoint_loops(N);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 255);
    
    return (total & 255) == 0 ? 0 : 1;
}
