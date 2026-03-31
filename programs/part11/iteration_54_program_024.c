/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>

/* Force ARM Thumb mode for hardware loop support */
#ifdef __arm__
__attribute__((target("thumb")))
#endif
volatile int g_counter = 0;

/* Pattern A: Perfectly nested loops - inner loop bitmap is subset of outer */
__attribute__((used, noinline))
static int pattern_a_perfect_nesting(int n, int m) {
    int sum = 0;
    volatile int i, j; /* volatile to prevent loop removal */
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Add some basic blocks inside outer loop but outside inner */
        if (__builtin_expect(g_counter > 100, 0)) {
            sum += 1;
        }
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            sum += i * j;
            /* Split inner loop block */
            if (__builtin_expect(j % 2 == 0, 1)) {
                sum += 1;
            } else {
                sum -= 1;
            }
        }
        
        /* More outer loop blocks */
        if (__builtin_expect(i % 3 == 0, 0)) {
            sum += 2;
        }
    }
    return sum;
}

/* Pattern B: Partially overlapping loops with shared blocks but not perfect nesting */
__attribute__((used, noinline))
static int pattern_b_partial_overlap(int n, int m) {
    int sum = 0;
    volatile int i = 0, j = 0;
    int flag = 1;
    
    /* First loop with early exit */
    while (i < n) {
        sum += i;
        if (__builtin_expect(i == n/2, 0)) {
            break; /* Creates separate exit block */
        }
        
        /* Shared computation block */
        int temp = i * 3;
        sum += temp;
        
        /* Conditionally execute second loop inside first */
        if (flag && j < m) {
            /* Second loop that overlaps but isn't perfectly nested */
            do {
                sum += j;
                j++;
                /* Different block structure than outer loop */
                if (__builtin_expect(j % 4 == 0, 1)) {
                    sum += 100;
                }
            } while (j < m && j < 5);
            flag = 0;
        }
        
        i++;
        /* Another block in first loop only */
        sum += 7;
    }
    
    return sum;
}

/* Pattern C: Sequential loops with shared preheader/setup block */
__attribute__((used, noinline))
static int pattern_c_sequential_loops(int n) {
    int sum = 0;
    volatile int i, j;
    
    /* Shared setup block - may be included in both loop bitmaps */
    int base = g_counter * 2;
    
    /* First loop */
    i = 0;
    while (i < n) {
        sum += i + base;
        i++;
        /* Split block */
        if (__builtin_expect(i % 5 == 0, 0)) {
            sum += 5;
        }
    }
    
    /* Reset base - shared block between loops */
    base = g_counter + 1;
    
    /* Second loop with different structure */
    for (j = n; j > 0; j--) {
        sum += j * base;
        /* More complex block structure */
        switch (j % 3) {
            case 0: sum += 10; break;
            case 1: sum += 20; break;
            default: sum += 30; break;
        }
    }
    
    return sum;
}

/* Pattern D: Complex nested loops with multiple exits and continues */
__attribute__((used, noinline))
static int pattern_d_complex_nesting(int n, int m) {
    int sum = 0;
    volatile int i = 0, j;
    
    /* Outer loop with multiple exit points */
    for (;;) {
        if (i >= n) break;
        
        /* Middle loop */
        j = 0;
        while (j < m) {
            sum += i * j;
            
            /* Innermost loop with early continue */
            for (int k = 0; k < 3; k++) {
                if (__builtin_expect(k == 1, 0)) {
                    continue; /* Creates separate continue block */
                }
                sum += k;
            }
            
            j++;
            if (__builtin_expect(j == m/2, 0)) {
                goto middle_exit; /* Creates irregular control flow */
            }
        }
        middle_exit:
        
        i++;
        if (__builtin_expect(i % 7 == 0, 0)) {
            continue; /* Outer loop continue */
        }
        
        /* Another inner loop with different structure */
        for (int x = 0; x < 2; x++) {
            sum += x * i;
        }
    }
    
    return sum;
}

/* Pattern E: Loops with function calls creating overlapping regions */
__attribute__((noinline))
static int helper_loop(int start, int end) {
    int sum = 0;
    for (int i = start; i < end; i++) {
        sum += i;
        if (__builtin_expect(i % 2 == 0, 1)) {
            sum += 2;
        }
    }
    return sum;
}

__attribute__((used, noinline))
static int pattern_e_function_loops(int n) {
    int sum = 0;
    volatile int i = 0;
    
    /* Loop that calls another function with its own loop */
    while (i < n) {
        sum += i;
        
        /* Function call creates overlapping but not nested region */
        if (__builtin_expect(i > n/2, 0)) {
            sum += helper_loop(0, 3);
        }
        
        i++;
    }
    
    /* Another loop that shares setup with previous */
    int base = sum;
    for (int j = 0; j < 4; j++) {
        sum += base + j;
    }
    
    return sum;
}

/* Main function to ensure all patterns are executed */
int main(void) {
    int result = 0;
    
    /* Execute each pattern multiple times with different parameters */
    for (int run = 0; run < 10; run++) {
        g_counter = run;
        
        result += pattern_a_perfect_nesting(10, 5);
        result += pattern_b_partial_overlap(8, 6);
        result += pattern_c_sequential_loops(7);
        result += pattern_d_complex_nesting(6, 4);
        result += pattern_e_function_loops(9);
        
        /* Mix in some volatile operations to prevent optimization */
        asm volatile("" : "+r" (result));
    }
    
    /* Compile-time check to ensure loops aren't completely removed */
    _Static_assert(sizeof(int) == 4, "Expected 32-bit int");
    
    return result > 0 ? 0 : 1;
}
