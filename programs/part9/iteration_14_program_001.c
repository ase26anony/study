/* test_hwloop_coverage.c
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

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE OPTIMIZE
int perfect_nesting(int n) {
    int result = 0;
    int a, b, c;
    
    /* Outer loop - this will be 'loop' in the analysis */
    for (int i = 0; i < n; ++i) {
        /* No code here to ensure other is subset of loop */
        
        /* Inner loop - this will be 'other' in the analysis */
        for (int j = 0; j < i % 10 + 1; ++j) {
            /* Create register pressure */
            a = i * 3;
            b = j * 7;
            c = a - b;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            
            result ^= (a * b) >> (c & 0xF);
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE OPTIMIZE
int reverse_nesting(int n) {
    int result = 0;
    int x = 0, y = 0, z = 0;
    
    /* Outer loop - this will be 'other' in the analysis */
    for (int i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            x = i * j;
            y = x ^ j;
            asm volatile("" : : "r"(x), "r"(y));
            result += x - y;
        }
        
        /* Second inner loop - this will be 'loop' in the analysis */
        for (int k = 0; k < i % 5 + 2; ++k) {
            /* Create more register pressure */
            int t1 = k * 11;
            int t2 = i * 13;
            int t3 = t1 ^ t2;
            
            asm volatile("" : : "r"(t1), "r"(t2), "r"(t3));
            z = (t1 * t2) >> (t3 & 0x7);
            result ^= z;
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partial overlap with goto - loops intersect but neither is subset */
NOINLINE OPTIMIZE
int partial_overlap_goto(int n) {
    int result = 0;
    int counter = 0;
    
    /* Loop A - will be 'loop' in some analysis */
    for (int i = 0; i < n; ++i) {
        int val = i * 3;
        
    shared_label:
        /* This block will be shared between both loops */
        result += val;
        asm volatile("" : : "r"(val));
        
        /* Loop B - will be 'other' in some analysis */
        for (int j = 0; j < 2; ++j) {
            int tmp = j * 5;
            result ^= tmp;
            
            /* Jump to shared block in Loop A */
            if (counter++ % 3 == 0) {
                val = tmp;
                goto shared_label;
            }
            
            asm volatile("" : : "r"(tmp));
        }
        
        /* More operations to create additional blocks */
        for (int k = 0; k < 2; ++k) {
            result -= k;
        }
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with do-while */
NOINLINE OPTIMIZE
int mixed_loop_types(int n) {
    int result = 0;
    int i = 0;
    
    /* Outer for loop */
    for (i = 0; i < n; ++i) {
        int a = i * 2;
        int b = 0;
        
        /* Inner do-while loop */
        do {
            b++;
            int c = a * b;
            result ^= c;
            asm volatile("" : : "r"(c));
        } while (b < 3);
        
        /* While loop after do-while */
        int j = 0;
        while (j < 2) {
            int d = (a + j) * 3;
            result += d;
            asm volatile("" : : "r"(d));
            j++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Complex sibling loops with break to shared block */
NOINLINE OPTIMIZE
int sibling_loops_break(int n) {
    int result = 0;
    
    /* First loop */
    for (int i = 0; i < n; ++i) {
        int x = i * 7;
        
        /* Second loop that can break to a shared block */
        for (int j = 0; j < 5; ++j) {
            int y = j * 11;
            
            if (x + y > 30) {
                /* Break to code that's also in the first loop's continuation */
                result = x - y;
                goto shared_exit;
            }
            
            result += x * y;
            asm volatile("" : : "r"(x), "r"(y));
        }
        
    shared_exit:
        /* This block is in both loops' bitmaps */
        result &= 0xFFFF;
        asm volatile("" : : "r"(result));
        
        /* Additional computation to differentiate the loops */
        for (int k = 0; k < 2; ++k) {
            result ^= k * 13;
        }
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and command line to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 50) + 20;
    int N2 = (seed % 40) + 25;
    int N3 = (seed % 30) + 15;
    int N4 = (seed % 35) + 10;
    int N5 = (seed % 45) + 5;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= reverse_nesting(N2);
    total ^= partial_overlap_goto(N3);
    total ^= mixed_loop_types(N4);
    total ^= sibling_loops_break(N5);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
