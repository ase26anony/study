/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of loops */
static volatile int g_volatile_counter = 0;

/* Function 1: Perfectly nested loops (Pattern A) */
__attribute__((target("thumb")))
int perfect_nesting(int n, int m) {
    int sum = 0;
    int i, j;
    
    /* Outer loop - will contain inner loop completely */
    for (i = 0; i < n; i++) {
        g_volatile_counter++; /* Force side effect */
        
        /* Inner loop - subset of outer loop's blocks */
        for (j = 0; j < m; j++) {
            sum += i * j;
            if (__builtin_expect(j & 1, 0)) {
                /* Split basic block inside inner loop */
                sum += 1;
            }
        }
        
        /* Additional block in outer loop only */
        if (__builtin_expect(i & 1, 1)) {
            sum -= 1;
        }
    }
    
    return sum;
}

/* Function 2: Partially overlapping loops (Pattern B) */
__attribute__((used))
int partial_overlap(int n, int m) {
    int sum = 0;
    int i = 0, j = 0;
    
    /* First loop with complex control flow */
    while (i < n) {
        sum += i;
        g_volatile_counter++;
        
        /* Conditional that creates shared block */
        if (__builtin_expect(i > n/2, 0)) {
            /* Start second loop inside first loop's body */
            j = 0;
            do {
                sum += j;
                if (__builtin_expect(j & 3, 1)) {
                    sum >>= 1;
                }
                j++;
            } while (j < m && j < 5); /* Limited iterations */
            
            /* Early exit from first loop */
            if (sum > 1000) break;
        }
        
        i++;
        
        /* Another loop that shares some blocks */
        if (__builtin_expect(i == n/2, 0)) {
            for (j = 0; j < 3; j++) {
                sum += i * j;
            }
        }
    }
    
    return sum;
}

/* Function 3: Sibling loops with shared preheader (Pattern C) */
#pragma GCC target("arch=armv7-a")
int sibling_loops(int n, int m) {
    int sum = 0;
    int i;
    
    /* Shared setup block - may be included in both loop bitmaps */
    int shared = n + m;
    g_volatile_counter = shared;
    
    /* First sibling loop */
    for (i = 0; i < n; i++) {
        sum += i * shared;
        if (__builtin_expect(i == n-1, 0)) {
            /* Create unique block in first loop */
            sum *= 2;
        }
    }
    
    /* Intermediate code that might be shared */
    shared = sum % 256;
    
    /* Second sibling loop - shares some blocks with first */
    for (i = 0; i < m; i++) {
        sum -= i * shared;
        if (__builtin_expect(i == m-1, 0)) {
            /* Create unique block in second loop */
            sum /= 2;
        }
    }
    
    return sum;
}

/* Function 4: Complex nested loops with early exits */
__attribute__((target("thumb")))
int complex_nesting(int n, int m, int k) {
    int sum = 0;
    int i, j, l;
    
    /* Three-level nesting with conditions */
    for (i = 0; i < n; i++) {
        if (__builtin_expect(i == 0, 0)) {
            /* Extra block in middle loop only */
            sum += 100;
        }
        
        for (j = 0; j < m; j++) {
            /* Early exit from middle loop */
            if (j > k) break;
            
            for (l = 0; l < k; l++) {
                sum += i * j * l;
                
                /* Conditional block inside innermost loop */
                if (__builtin_expect(l & 1, 1)) {
                    sum += g_volatile_counter;
                }
            }
            
            /* Another block in middle loop */
            if (__builtin_expect(j & 1, 0)) {
                sum -= 1;
            }
        }
        
        /* Final block in outer loop */
        sum = (sum * 31) & 0xFF;
    }
    
    return sum;
}

/* Function 5: Disjoint loops (should not trigger intersection) */
int disjoint_loops(int n, int m) {
    int sum = 0;
    int i;
    
    /* First independent loop */
    for (i = 0; i < n; i++) {
        sum += i * i;
    }
    
    /* Completely separate code between loops */
    int temp = sum * 3 + 7;
    g_volatile_counter = temp;
    
    /* Second independent loop - no block overlap */
    for (i = 0; i < m; i++) {
        sum -= i * temp;
    }
    
    return sum;
}

/* Function 6: Loop with switch inside creating multiple blocks */
int loop_with_switch(int n) {
    int sum = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        switch (i % 4) {
            case 0:
                sum += i;
                break;
            case 1:
                sum += i * 2;
                /* Fall through */
            case 2:
                sum += i * 3;
                break;
            case 3:
                sum += i * 4;
                /* Nested loop inside switch case */
                for (int j = 0; j < 2; j++) {
                    sum += j;
                }
                break;
        }
        
        /* Additional conditional */
        if (__builtin_expect(i > n/2, 0)) {
            sum >>= 1;
        }
    }
    
    return sum;
}

/* Main function to ensure all functions are called */
int main(void) {
    int result = 0;
    
    /* Call each function with different parameters to create
       various loop structures in the compiler's IR */
    result += perfect_nesting(100, 50);
    result += partial_overlap(200, 30);
    result += sibling_loops(150, 75);
    result += complex_nesting(50, 40, 30);
    result += disjoint_loops(80, 60);
    result += loop_with_switch(100);
    
    /* Use result to prevent dead code elimination */
    if (result > 1000000) {
        return 1;
    }
    
    return 0;
}

/* Compile-time assertion to ensure optimization */
_Static_assert(sizeof(int) == 4, "int must be 32-bit for consistent loop behavior");
