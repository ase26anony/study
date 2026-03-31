/* test_hwloop_coverage.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop_coverage.c
 * Or for generic testing: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage test_hwloop_coverage.c -o test_hwloop
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' in the hierarchy */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure other is subset of loop */
        
        /* Inner loop - this will be 'other' in the hierarchy */
        for (int j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a + j;
            int c = b - i;
            int d = c * a;
            int e = d >> 2;
            
            /* Prevent optimization */
            asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            
            result ^= (a + b + c + d + e) & 0xFF;
        }
        
        /* No code here either - ensures perfect subset relationship */
    }
    
    return result;
}

/* Function 2: Loop is subset of other */
NOINLINE int loop_subset_of_other(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'other' in the hierarchy */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 5; ++j) {
            int temp = i * j;
            result += temp;
            asm volatile ("" : : "r"(temp));
        }
        
        /* Some intermediate code in 'other' but not in 'loop' */
        int intermediate = i * 2;
        asm volatile ("" : : "r"(intermediate));
        
        /* Second inner loop - this will be 'loop' in the hierarchy */
        for (int k = 0; k < i; ++k) {
            /* This loop is subset of 'other' */
            int a = k * 3;
            int b = a + intermediate;
            int c = b ^ k;
            
            asm volatile ("" : : "r"(a), "r"(b), "r"(c));
            result ^= (a + b + c) & 0xFF;
        }
        
        /* More code in 'other' after 'loop' */
        result += intermediate;
    }
    
    return result;
}

/* Function 3: Partially overlapping loops with goto */
NOINLINE int overlapping_with_goto(int N) {
    int result = 0;
    int i, j;
    
    /* Loop A - will be 'loop' */
    for (i = 0; i < N; ++i) {
        int a = i * 2;
        
    shared_label:
        /* This block will be shared between loops */
        result += a;
        asm volatile ("" : : "r"(a));
        
        /* Loop B - will be 'other' */
        for (j = 0; j < 3; ++j) {
            int b = i + j;
            
            /* Jump into Loop A's body */
            if (j == 2 && (result & 1)) {
                goto shared_label;
            }
            
            result ^= b;
            asm volatile ("" : : "r"(b));
        }
        
        /* More code in Loop A */
        result -= i;
    }
    
    return result;
}

/* Function 4: Mixed loop types (do-while inside for) */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    
    /* Outer for loop */
    for (int i = 0; i < N; ++i) {
        int counter = i % 10;
        
        /* Inner do-while loop */
        do {
            int a = counter * 3;
            int b = a + i;
            int c = b ^ counter;
            
            /* Create complex expression for register pressure */
            result += (a * b) / (c + 1);
            asm volatile ("" : : "r"(a), "r"(b), "r"(c));
            
            counter--;
        } while (counter > 0);
        
        /* While loop after do-while */
        int temp = i;
        while (temp > 0) {
            result ^= temp;
            asm volatile ("" : : "r"(temp));
            temp >>= 1;
        }
    }
    
    return result;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int complex_sibling_loops(int N) {
    int result = 0;
    
    /* Level 1: Outer loop */
    for (int i = 0; i < N; ++i) {
        /* Level 2: First middle loop */
        for (int j = 0; j < i; ++j) {
            /* Level 3: Innermost loop A */
            for (int k = 0; k < 2; ++k) {
                int a = i * j * k;
                result += a;
                asm volatile ("" : : "r"(a));
            }
        }
        
        /* Code between sibling loops at level 2 */
        int between = i * 3;
        asm volatile ("" : : "r"(between));
        
        /* Level 2: Second middle loop (sibling to first) */
        for (int m = 0; m < 5; ++m) {
            /* Level 3: Innermost loop B */
            for (int n = 0; n < 3; ++n) {
                int b = i * m * n + between;
                result ^= b;
                asm volatile ("" : : "r"(b));
            }
        }
    }
    
    return result;
}

/* Function 6: Disjoint loops (should not intersect) */
NOINLINE int disjoint_loops(int N) {
    int result = 0;
    
    /* First independent loop */
    for (int i = 0; i < N; ++i) {
        result += i * 2;
        asm volatile ("" : : "r"(i));
    }
    
    /* Unrelated code block */
    int temp = result * 3;
    
    /* Second independent loop */
    for (int j = 0; j < N/2; ++j) {
        result ^= j + temp;
        asm volatile ("" : : "r"(j));
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= loop_subset_of_other(N);
    total ^= overlapping_with_goto(N);
    total ^= mixed_loop_types(N);
    total ^= complex_sibling_loops(N);
    total ^= disjoint_loops(N);
    
    /* Generate side effect to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    /* Additional calls with different parameters */
    for (int i = 1; i < 5; i++) {
        total ^= perfect_nesting(N + i);
        total ^= loop_subset_of_other(N - i);
    }
    
    return total & 0xFF;
}
