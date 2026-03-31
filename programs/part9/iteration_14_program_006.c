/* 
 * Test program for hw-doloop.cc coverage
 * Specifically targets bitmap intersection logic in discover_loop_hierarchy
 * 
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 *   or for generic target: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Force hardware loop optimization on supported targets */
#ifdef __ARM_ARCH
#define HWLOOP_ATTR __attribute__((target("arch=armv8-a+lse")))
#else
#define HWLOOP_ATTR
#endif

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Barrier to prevent optimization */
#define OPT_BARRIER(var) asm volatile("" : : "r"(var))

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE HWLOOP_ATTR
int perfect_nesting(int N) {
    int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' */
    for (i = 0; i < N; ++i) {
        /* No code here to ensure other is subset */
        
        /* Inner loop - this will be 'other' */
        for (j = 0; j < (N - i); ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = a * 2;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            result ^= d;
            OPT_BARRIER(result);
        }
        
        /* No code here either */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop is subset of other */
NOINLINE HWLOOP_ATTR
int loop_subset_of_other(int N) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop - this will be 'other' */
    for (i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 5; ++j) {
            int a = i * j;
            result += a;
            OPT_BARRIER(result);
        }
        
        /* Second inner loop - this will be 'loop' (subset of other) */
        for (k = 0; k < (N - i); ++k) {
            int a = i + k;
            int b = a * 3;
            int c = b >> 2;
            result ^= c;
            OPT_BARRIER(result);
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto */
NOINLINE HWLOOP_ATTR
int overlapping_with_goto(int N) {
    int result = 0;
    int i = 0, j = 0;
    
    /* Loop A - will be 'loop' */
    for (i = 0; i < N; ++i) {
        int a = i * 2;
        result += a;
        
        /* Loop B - will be 'other' */
        j = 0;
        while (j < 10) {
            int b = j * 3;
            result ^= b;
            
            /* Conditional goto into loop A's body */
            if ((i + j) % 7 == 0) {
                goto shared_label;
            }
            
            j++;
        }
        
        continue;
        
    shared_label:
        /* This label is inside loop A but reachable from loop B */
        int c = i * j;
        result -= c;
        OPT_BARRIER(result);
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex nesting */
NOINLINE HWLOOP_ATTR
int mixed_loop_types(int N) {
    int result = 0;
    int i = 0;
    
    /* Outer for loop */
    for (i = 0; i < N; ++i) {
        int j = 0;
        
        /* Inner do-while loop */
        do {
            int a = i + j;
            int b = a * a;
            result += b;
            OPT_BARRIER(result);
            j++;
        } while (j < 5);
        
        int k = 0;
        
        /* Inner while loop */
        while (k < (N - i)) {
            int c = i * k;
            int d = c >> 1;
            result ^= d;
            OPT_BARRIER(result);
            
            /* Nested for inside while */
            for (int m = 0; m < 2; ++m) {
                int e = k * m;
                result += e;
            }
            
            k++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with break to shared block */
NOINLINE HWLOOP_ATTR
int sibling_loops_with_break(int N) {
    int result = 0;
    
    /* First sibling loop */
    for (int i = 0; i < N; ++i) {
        if (i % 3 == 0) {
            /* Break to shared code block */
            goto shared_block;
        }
        result += i * 2;
    }
    
    /* Second sibling loop that can also reach shared block */
    for (int j = N; j > 0; --j) {
        if (j % 4 == 0) {
            goto shared_block;
        }
        result += j * 3;
    }
    
    return result & 0xFF;
    
shared_block:
    /* Shared block between the two loops */
    result = (result * 13) & 0xFF;
    OPT_BARRIER(result);
    return result;
}

/* Function 6: Complex hierarchy with multiple levels */
NOINLINE HWLOOP_ATTR
int complex_hierarchy(int N) {
    int result = 0;
    
    /* Level 1 */
    for (int a = 0; a < N; ++a) {
        /* Level 2 - Loop X */
        for (int b = 0; b < 3; ++b) {
            /* Level 3 */
            for (int c = 0; c < 2; ++c) {
                int val = a * b * c;
                result += val;
                OPT_BARRIER(result);
            }
        }
        
        /* Level 2 - Loop Y (sibling of X) */
        for (int d = 0; d < 5; ++d) {
            /* Level 3 - different structure */
            int e = 0;
            while (e < 2) {
                result ^= (a * d * e);
                e++;
            }
        }
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 25) + 25;
    int N5 = (seed % 20) + 30;
    int N6 = (seed % 15) + 35;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= loop_subset_of_other(N2);
    total ^= overlapping_with_goto(N3);
    total ^= mixed_loop_types(N4);
    total ^= sibling_loops_with_break(N5);
    total ^= complex_hierarchy(N6);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
