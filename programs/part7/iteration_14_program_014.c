/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * For best results, also use: -funroll-loops -fpeel-loops
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE int test_perfect_nesting(int n) {
    int result = 0;
    volatile int temp;
    
    /* Outer loop (will be 'loop' in the analysis) */
    for (int i = 0; i < n; ++i) {
        /* No code here to ensure inner loop is subset */
        
        /* Inner loop (will be 'other' in the analysis) */
        for (int j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a ^ j;
            int c = b - i;
            int d = c * a;
            result ^= (d >> 2);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
        }
        
        /* No code here either - inner loop is perfect subset */
    }
    
    temp = result;
    return temp & 0xFF;
}

/* Function 2: Loop is subset of other */
NOINLINE int test_loop_subset_of_other(int n) {
    int result = 0;
    volatile int temp;
    
    /* Outer loop (will be 'other' in the analysis) */
    for (int i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 2; ++j) {
            int a = i + j;
            int b = a * 3;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop (will be 'loop' in the analysis) */
        /* This loop's blocks are subset of outer loop's blocks */
        for (int k = 0; k < i; ++k) {
            int x = k * i;
            int y = x ^ result;
            int z = y - k;
            result ^= z;
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
        }
        
        /* More code in outer loop after inner loops */
        result += i * 7;
    }
    
    temp = result;
    return temp & 0xFF;
}

/* Function 3: Partially overlapping loops using goto */
NOINLINE int test_partial_overlap_goto(int n) {
    int result = 0;
    volatile int temp;
    int i, j;
    
    /* Loop A */
    for (i = 0; i < n; ++i) {
        int a = i * 3;
        result += a;
        
        if (i == n/2) {
            /* Jump into Loop B's body */
            goto inside_loop_b;
        }
        
        /* Loop B */
        for (j = 0; j < 5; ++j) {
            inside_loop_b:
            int b = j * 7;
            result ^= b;
            
            /* Complex body for register pressure */
            int c = b * i;
            int d = c ^ j;
            int e = d - i;
            result += e;
            asm volatile("" : : "r"(c), "r"(d), "r"(e));
            
            if (j == 3 && i > n/2) {
                /* Break from inner to outer */
                break;
            }
        }
        
        result += i;
    }
    
    temp = result;
    return temp & 0xFF;
}

/* Function 4: Mixed loop types - do-while inside for */
NOINLINE int test_mixed_loop_types(int n) {
    int result = 0;
    volatile int temp;
    
    /* Outer for loop */
    for (int i = 0; i < n; ++i) {
        int counter = 0;
        
        /* Inner do-while loop */
        do {
            int a = counter * i;
            int b = a ^ result;
            int c = b << 2;
            result += c;
            
            /* Register pressure */
            int d = c * a;
            int e = d - b;
            int f = e ^ i;
            result ^= f;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f));
            counter++;
        } while (counter < 3);
        
        /* While loop after do-while */
        int w = 0;
        while (w < 2) {
            result += w * i;
            w++;
        }
    }
    
    temp = result;
    return temp & 0xFF;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int test_complex_siblings(int n) {
    int result = 0;
    volatile int temp;
    
    /* Level 1 outer loop */
    for (int i = 0; i < n; ++i) {
        /* Sibling loop A at level 2 */
        for (int j = 0; j < 3; ++j) {
            result += i * j;
            asm volatile("" : : "r"(j));
        }
        
        /* Sibling loop B at level 2 (partially overlaps with A via shared parent) */
        for (int k = 0; k < i + 1; ++k) {
            int x = k * 11;
            int y = x ^ i;
            result ^= y;
            
            /* Level 3 inner loop */
            for (int l = 0; l < 2; ++l) {
                int z = l * y;
                result += z;
                asm volatile("" : : "r"(z));
            }
        }
        
        /* Another sibling at level 2 */
        int m = 0;
        while (m < 2) {
            result -= i * m;
            m++;
        }
    }
    
    temp = result;
    return temp & 0xFF;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    int total = 0;
    volatile int seed;
    
    /* Use command line or volatile for loop bounds */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = global_seed;
    }
    
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 25) + 25;
    int N5 = (seed % 20) + 30;
    
    /* Call all test functions */
    total ^= test_perfect_nesting(N1);
    total ^= test_loop_subset_of_other(N2);
    total ^= test_partial_overlap_goto(N3);
    total ^= test_mixed_loop_types(N4);
    total ^= test_complex_siblings(N5);
    
    /* Ensure result is used */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
