/* 
 * Test program for hw-doloop.cc coverage
 * Designed to trigger bitmap intersection logic in discover_loop_hierarchy
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure other is subset */
        
        /* Inner loop (will be 'other' in hierarchy) */
        for (j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a ^ j;
            int c = b - i;
            int d = c * a;
            result ^= (d >> 2) & 0xFF;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(result));
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result;
}

/* Function 2: Loop is subset of other */
NOINLINE int loop_subset_of_other(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i * j;
            result += a;
            asm volatile("" : : "r"(result));
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (k = 0; k < i; ++k) {
            /* This loop is subset of outer */
            int b = i ^ k;
            int c = b * k;
            result ^= c & 0xFF;
            asm volatile("" : : "r"(result));
        }
    }
    
    return result;
}

/* Function 3: Partially overlapping loops with goto */
NOINLINE int overlapping_with_goto(int n) {
    int result = 0;
    int i, j;
    
    /* Loop A */
    for (i = 0; i < n; ++i) {
        int a = i * 2;
        
    loop_b_start:
        /* Loop B - shares block via goto */
        for (j = 0; j < 5; ++j) {
            if (j == 3 && i % 2 == 0) {
                /* Jump into loop A's body */
                result += a;
                goto inside_loop_a;
            }
            
            int b = j * i;
            result ^= b;
            asm volatile("" : : "r"(result));
        }
        
        continue;
        
    inside_loop_a:
        /* This label is inside loop A but reachable from loop B */
        int c = result * i;
        result = c & 0xFFF;
        asm volatile("" : : "r"(result));
        
        /* Jump back to loop B */
        if (i < n - 1) {
            goto loop_b_start;
        }
    }
    
    return result;
}

/* Function 4: Mixed loop types (do-while inside for) */
NOINLINE int mixed_loop_types(int n) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < n) {
        int j = 0;
        
        /* do-while inside for */
        for (int k = 0; k < 3; ++k) {
            /* do-while loop */
            do {
                int a = i * j * k;
                int b = a ^ result;
                int c = b + k;
                int d = c - j;
                result = (result + d) & 0xFFFF;
                asm volatile("" : : "r"(result));
                j++;
            } while (j < 2);
            
            j = 0;
        }
        
        i++;
    }
    
    return result;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int complex_sibling_loops(int n) {
    int result = 0;
    
    /* Outer container */
    for (int outer = 0; outer < 2; ++outer) {
        /* First sibling loop */
        for (int sib1 = 0; sib1 < n; ++sib1) {
            int a = sib1 * outer;
            result += a;
            
            /* Small inner loop in first sibling */
            for (int inner1 = 0; inner1 < 2; ++inner1) {
                result ^= (a + inner1);
                asm volatile("" : : "r"(result));
            }
        }
        
        /* Second sibling loop (partially overlaps via shared variable) */
        for (int sib2 = 0; sib2 < n; ++sib2) {
            int b = sib2 + outer;
            result -= b;
            
            /* Different inner structure */
            int counter = 0;
            while (counter < 3) {
                result |= (b << counter);
                asm volatile("" : : "r"(result));
                counter++;
            }
        }
    }
    
    return result;
}

/* Main function with volatile inputs */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 50) + 20;  /* Ensure loops run */
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= loop_subset_of_other(N + 5);
    total ^= overlapping_with_goto(N + 3);
    total ^= mixed_loop_types(N + 2);
    total ^= complex_sibling_loops(N + 1);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
