/* 
 * Test program for hw-doloop.cc coverage
 * Designed to trigger bitmap intersection logic in discover_loop_hierarchy
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop.c -o test_hwloop.o
 * For best results, also try: -O3 -funroll-loops -fpeel-loops
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE int test_perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure inner loop is perfect subset */
        
        /* Inner loop (will be 'other' in hierarchy) */
        for (j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = a * 2;
            int c = b - a;
            int d = c * 3;
            int e = d >> 2;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            
            result ^= (a * b) >> c;
        }
        
        /* No code here either - inner loop is perfect subset */
    }
    
    return result & 0xFF;
}

/* Function 2: loop is subset of other */
NOINLINE int test_loop_subset_of_other(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int temp = i * j;
            asm volatile("" : : "r"(temp));
            result += temp;
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (k = 0; k < i; ++k) {
            /* Register pressure */
            int a = i + k;
            int b = a * 3;
            int c = b - a;
            int d = c << 1;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            result ^= (b * d) >> 2;
        }
        
        /* More code in outer loop after inner loops */
        result += i * 7;
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto */
NOINLINE int test_partial_overlap_goto(int n) {
    int result = 0;
    int i, j;
    
    /* First loop (will be 'loop' in hierarchy) */
    for (i = 0; i < n; ++i) {
        int a = i * 2;
        
    loop_body:
        /* This label will be inside loop's bitmap */
        result += a;
        
        /* Second loop (will be 'other' in hierarchy) */
        for (j = 0; j < 5; ++j) {
            int b = i + j;
            
            /* Create register pressure */
            int c = b * 3;
            int d = c - b;
            int e = d >> 1;
            
            asm volatile("" : : "r"(b), "r"(c), "r"(d), "r"(e));
            
            result ^= (c * e);
            
            /* Conditional goto into first loop's body */
            if (j == 3 && (result & 1)) {
                goto loop_body;  /* Creates intersection */
            }
        }
        
        /* More code in first loop */
        result -= i;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types (do-while inside for) */
NOINLINE int test_mixed_loop_types(int n) {
    int result = 0;
    int i = 0;
    
    /* Outer for loop */
    for (i = 0; i < n; ++i) {
        int counter = 0;
        
        /* Inner do-while loop */
        do {
            /* Register pressure */
            int a = i + counter;
            int b = a * 5;
            int c = b % 13;
            int d = c << counter;
            int e = d ^ a;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            
            result += (b * c) >> 2;
            counter++;
        } while (counter < 4 && (result & 0xF) != 0);
        
        /* While loop after do-while */
        int k = 0;
        while (k < 2) {
            result ^= (i * k);
            k++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int test_complex_siblings(int n) {
    int result = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* First sibling inner loop */
        for (int j = 0; j < i % 5 + 1; ++j) {
            int a = i * j;
            int b = a + 7;
            asm volatile("" : : "r"(a), "r"(b));
            result += a * b;
        }
        
        /* Code between sibling loops */
        result ^= i;
        
        /* Second sibling inner loop */
        for (int k = 0; k < 3; ++k) {
            /* Heavy register pressure */
            int x = i + k;
            int y = x * 11;
            int z = y % 17;
            int w = z << 1;
            int v = w ^ x;
            int u = v * 3;
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z), "r"(w), "r"(v), "r"(u));
            
            result += u;
        }
        
        /* Third loop with break to outer */
        for (int m = 0; m < 4; ++m) {
            if ((result & 0xFF) == 0) {
                break;  /* Creates CFG edge */
            }
            result -= m;
        }
    }
    
    return result & 0xFF;
}

/* Main function to drive all tests */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    /* Call all test functions */
    total ^= test_perfect_nesting(N);
    total ^= test_loop_subset_of_other(N + 5);
    total ^= test_partial_overlap_goto(N + 3);
    total ^= test_mixed_loop_types(N + 7);
    total ^= test_complex_siblings(N + 2);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
