/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * or: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage -march=native
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
    int a, b, c;
    
    /* Outer loop - this will be 'loop' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* Inner loop - this will be 'other' in the analysis */
        for (int j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            a = i * j;
            b = a + j;
            c = b - i;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            
            result ^= (a * b) >> (c & 0xF);
        }
        /* No code here ensures loop's blocks are superset of other's blocks */
    }
    return result & 0xFF;
}

/* Function 2: Loop is subset of other (Condition 3) */
NOINLINE int loop_subset_of_other(int N) {
    int result = 0;
    int x = 0, y = 0, z = 0;
    
    /* Outer loop - this will be 'other' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            x = i * j;
            y = x + global_seed;
            asm volatile("" : : "r"(x), "r"(y));
            result += x - y;
        }
        
        /* Second inner loop - this will be 'loop' in the analysis */
        for (int k = 0; k < i + 2; ++k) {
            z = k * i;
            x = z + k;
            y = x - i;
            
            /* Create more register pressure */
            int t1 = z * x;
            int t2 = y << 2;
            int t3 = t1 ^ t2;
            
            asm volatile("" : : "r"(t1), "r"(t2), "r"(t3));
            result ^= t3;
        }
    }
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto (Condition 1) */
NOINLINE int overlapping_with_goto(int N) {
    int result = 0;
    int a = 0, b = 0;
    
    /* Loop A - will be 'loop' in some analysis */
    for (int i = 0; i < N; ++i) {
        a = i * 3;
        
    shared_block:
        b = a + i;
        result += b;
        
        /* Loop B - will be 'other' in some analysis */
        for (int j = 0; j < 5; ++j) {
            int c = j * 2;
            
            /* Jump to shared block in Loop A */
            if (j == 3 && i < N/2) {
                goto shared_block;
            }
            
            /* Register pressure */
            int d = c * a;
            int e = d >> 1;
            asm volatile("" : : "r"(d), "r"(e));
            result ^= e;
        }
    }
    return result & 0xFF;
}

/* Function 4: Mixed loop types (do-while inside for) */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    int counter = N;
    
    /* Outer for loop */
    for (int i = 0; i < N && counter > 0; ++i) {
        int j = 0;
        
        /* Inner do-while loop */
        do {
            int a = i * j;
            int b = a + global_seed;
            int c = b - j;
            
            /* Multiple operations for register pressure */
            int d = c * a;
            int e = d ^ b;
            int f = e >> (j & 3);
            
            asm volatile("" : : "r"(d), "r"(e), "r"(f));
            result += f;
            
            j++;
        } while (j < 4);
        
        counter--;
    }
    
    /* Additional while loop after for loop */
    int k = 0;
    while (k < 10) {
        result ^= k * global_seed;
        asm volatile("" : : "r"(result));
        k++;
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int complex_sibling_loops(int N) {
    int result = 0;
    
    /* Level 1 outer loop */
    for (int i = 0; i < N; ++i) {
        /* Sibling loop A */
        for (int j = 0; j < 2; ++j) {
            int a = i * j + global_seed;
            asm volatile("" : : "r"(a));
            result += a;
        }
        
        /* Sibling loop B - shares some blocks via control flow */
        for (int k = 0; k < 3; ++k) {
            int b = i * k;
            int c = b - k;
            
            if (k == 1) {
                /* This creates partial overlap */
                goto shared_computation;
            }
            
            result ^= b + c;
            continue;
            
        shared_computation:
            result += c * 2;
        }
        
        /* Another inner loop at same level */
        int m = 0;
        while (m < i % 5 + 1) {
            int d = m * i;
            asm volatile("" : : "r"(d));
            result -= d;
            m++;
        }
    }
    
    return result & 0xFF;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 50) + 20;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 10;
    int N4 = (seed % 25) + 5;
    int N5 = (seed % 20) + 3;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= loop_subset_of_other(N2);
    total ^= overlapping_with_goto(N3);
    total ^= mixed_loop_types(N4);
    total ^= complex_sibling_loops(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
