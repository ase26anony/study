/* test_hwloop.c
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop.c -o test_hwloop.o
 * Or for generic testing: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage test_hwloop.c -o test_hwloop_executable
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    int a, b, c, d, e, f, g, h;
    
    /* Outer loop (will be 'loop' in the analysis) */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure other is perfect subset */
        
        /* Inner loop (will be 'other' in the analysis) */
        for (int j = 0; j < (N - i); ++j) {
            /* Create register pressure */
            a = i + j;
            b = a * 2;
            c = b - a;
            d = c ^ b;
            e = d << 2;
            f = e >> 1;
            g = f * a;
            h = g ^ result;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), 
                         "r"(e), "r"(f), "r"(g), "r"(h));
            
            result ^= (a * b) >> (c & 3);
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    int x, y, z;
    
    /* Outer loop (will be 'other' in the analysis) */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            x = i * j;
            y = x ^ result;
            asm volatile("" : : "r"(x), "r"(y));
            result += x - y;
        }
        
        /* Second inner loop (will be 'loop' in the analysis) */
        for (int k = 0; k < (N - i); ++k) {
            /* More register pressure */
            int t1 = i + k;
            int t2 = t1 * 3;
            int t3 = t2 ^ k;
            int t4 = t3 << 1;
            int t5 = t4 >> 2;
            
            asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5));
            result ^= t1 * t2 + t3;
        }
        
        /* More code in outer loop after inner loops */
        z = i * 7;
        asm volatile("" : : "r"(z));
        result += z;
    }
    
    return result & 0xFF;
}

/* Function 3: Partial overlap with goto (Condition 1) */
NOINLINE int partial_overlap_goto(int N) {
    int result = 0;
    
    /* Loop A (will be 'loop' in the analysis) */
    for (int i = 0; i < N; ++i) {
        int a = i * 2;
        int b = a ^ i;
        
    loop_body:
        /* Shared block label */
        asm volatile("" : : "r"(a), "r"(b));
        result += a - b;
        
        /* Loop B (will be 'other' in the analysis) */
        for (int j = 0; j < 5; ++j) {
            int c = j * 3;
            int d = c ^ result;
            
            asm volatile("" : : "r"(c), "r"(d));
            result ^= c + d;
            
            /* Jump into loop A's body, creating intersection */
            if (j == 2 && i < N/2) {
                goto loop_body;
            }
        }
        
        /* More code in loop A */
        int e = i * 5;
        asm volatile("" : : "r"(e));
        result -= e;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with do-while */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    int counter = 0;
    
    /* Outer for loop */
    for (int i = 0; i < N; ++i) {
        /* Inner do-while loop */
        int j = 0;
        do {
            int a = i * j;
            int b = a ^ counter;
            int c = b << 1;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            result += a + b - c;
            
            j++;
            counter++;
        } while (j < 4);
        
        /* While loop after do-while */
        int k = 0;
        while (k < 3) {
            int d = i * k;
            int e = d ^ result;
            
            asm volatile("" : : "r"(d), "r"(e));
            result ^= d - e;
            
            k++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Complex sibling loops */
NOINLINE int complex_siblings(int N) {
    int result = 0;
    
    /* First sibling loop */
    for (int i = 0; i < N; ++i) {
        int a = i * 11;
        int b = a ^ 0x55;
        asm volatile("" : : "r"(a), "r"(b));
        result += a ^ b;
    }
    
    /* Second sibling loop with different bound */
    for (int j = 0; j < N * 2; ++j) {
        int c = j * 7;
        int d = c ^ result;
        int e = d << 2;
        int f = e >> 1;
        
        asm volatile("" : : "r"(c), "r"(d), "r"(e), "r"(f));
        result ^= c * d + e - f;
    }
    
    /* Third loop that shares some computation */
    for (int k = 0; k < N; ++k) {
        int g = k * 3;
        int h = g ^ k;
        asm volatile("" : : "r"(g), "r"(h));
        result += g - h;
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to create varying loop bounds */
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 25) + 25;
    int N5 = (seed % 20) + 30;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= reverse_nesting(N2);
    total ^= partial_overlap_goto(N3);
    total ^= mixed_loop_types(N4);
    total ^= complex_siblings(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 255);
    
    return 0;
}
