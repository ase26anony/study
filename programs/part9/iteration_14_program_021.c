/* test_hwloop.c
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a test_hwloop.c -o test_hwloop
 * Or for generic targets: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage test_hwloop.c -o test_hwloop_executable
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    volatile int v = N; /* Prevent constant propagation */
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (int i = 0; i < v; ++i) {
        /* No code here to ensure other is perfect subset */
        
        /* Inner loop (will be 'other' in hierarchy) */
        for (int j = 0; j < 5; ++j) {
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
        
        /* No code here either - ensures loop has same blocks as other */
    }
    
    return result;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    volatile int v = N;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (int i = 0; i < v; ++i) {
        /* First inner loop - creates blocks in other that are not in loop */
        for (int j = 0; j < 3; ++j) {
            int a = i + j;
            int b = a * 2;
            result += (a ^ b);
            asm volatile ("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (int k = 0; k < 4; ++k) {
            /* This loop is subset of other */
            int x = i * k;
            int y = x + k;
            int z = y - i;
            result ^= (x * y * z) & 0xFF;
            asm volatile ("" : : "r"(x), "r"(y), "r"(z));
        }
    }
    
    return result;
}

/* Function 3: Partially overlapping loops with goto (Condition 1) */
NOINLINE int overlapping_loops(int N) {
    int result = 0;
    volatile int v = N;
    
    /* First loop (will be 'loop') */
    for (int i = 0; i < v; ++i) {
        int a = i * 2;
        
    shared_block:
        /* This block will be shared between both loops */
        int b = a + i;
        result += b;
        asm volatile ("" : : "r"(a), "r"(b));
        
        if (i & 1) {
            /* Second loop (will be 'other') - jumps into first loop */
            for (int j = 0; j < 3; ++j) {
                int c = j * 3;
                result ^= c;
                
                if (j == 1) {
                    /* Jump to shared block in first loop */
                    goto shared_block;
                }
                
                asm volatile ("" : : "r"(c));
            }
        }
    }
    
    return result;
}

/* Function 4: Mixed loop types with complex relationships */
NOINLINE int mixed_loops(int N) {
    int result = 0;
    volatile int v = N;
    
    /* do-while inside for */
    for (int i = 0; i < v; ++i) {
        int j = 0;
        
        /* do-while loop (nested) */
        do {
            int a = i + j;
            int b = a * j;
            int c = b - i;
            int d = c >> 1;
            result += (a ^ b ^ c ^ d);
            asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            j++;
        } while (j < 3);
        
        /* while loop after for */
        int k = 0;
        while (k < 2) {
            int x = i * k;
            result ^= x;
            asm volatile ("" : : "r"(x));
            k++;
        }
    }
    
    return result;
}

/* Function 5: Sibling loops with partial overlap via break */
NOINLINE int sibling_loops(int N) {
    int result = 0;
    volatile int v = N;
    
    /* First loop */
    for (int i = 0; i < v; ++i) {
        int a = i * 3;
        
        /* Label for break target */
    middle_block:
        int b = a + 1;
        result += b;
        asm volatile ("" : : "r"(a), "r"(b));
        
        /* Second loop that can break into first */
        for (int j = 0; j < 4; ++j) {
            int c = j * 2;
            result ^= c;
            
            if (c == 4) {
                /* Break to middle of first loop */
                goto middle_block;
            }
            
            asm volatile ("" : : "r"(c));
        }
    }
    
    return result;
}

/* Main function with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and command line to prevent optimization */
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 20) + 25;
    int N5 = (seed % 10) + 30;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= reverse_nesting(N2);
    total ^= overlapping_loops(N3);
    total ^= mixed_loops(N4);
    total ^= sibling_loops(N5);
    
    /* Generate side effect to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return total & 1;
}
