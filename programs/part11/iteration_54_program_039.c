/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Function 1: Perfectly nested loops (Pattern A) */
/* Should trigger: bitmap_intersect_p = true, bitmap_intersect_compl_p(inner, outer) = false */
__attribute__((target("thumb")))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            sum += i * j;
            /* Split basic block */
            if (__builtin_expect(sum & 1, 0)) {
                sink = sum;
            }
        }
        
        /* Additional block in outer loop only */
        if (__builtin_expect(i % 2 == 0, 1)) {
            sum += 1;
        }
    }
    
    sink = sum;
}

/* Function 2: Partially overlapping loops (Pattern B) */
/* Should trigger: bitmap_intersect_p = true, both bitmap_intersect_compl_p checks = true */
__attribute__((target("thumb")))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* First loop with early exit */
    while (i < n) {
        sum += i;
        i++;
        
        /* Conditional second loop inside first */
        if (__builtin_expect(i % 3 == 0, 0)) {
            /* Second loop that shares some blocks */
            for (j = 0; j < m; j++) {
                sum += j;
                /* Shared computation block */
                if (__builtin_expect(j % 2 == 0, 1)) {
                    sum += 1;
                }
            }
        }
        
        /* Additional block only in first loop */
        if (__builtin_expect(i % 4 == 0, 0)) {
            sum += 2;
        }
    }
    
    /* Second independent loop that shares setup code */
    j = 0;
    do {
        sum -= j;
        j++;
        
        /* Block that might overlap with previous loops */
        if (__builtin_expect(j % 5 == 0, 0)) {
            sink = sum;
        }
    } while (j < m);
    
    sink = sum;
}

/* Function 3: Sibling loops with shared preheader (Pattern C) */
/* Should trigger bitmap_intersect_p = true and one bitmap_intersect_compl_p = false */
__attribute__((target("thumb")))
void sibling_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared preheader block */
    int shared = n + m;
    sink = shared;
    
    /* First sibling loop */
    i = 0;
    while (i < n) {
        sum += i * shared;
        i++;
        
        /* Split block */
        if (__builtin_expect(i % 3 == 0, 0)) {
            shared++;
        }
    }
    
    /* Shared intermediate block */
    sink = sum;
    
    /* Second sibling loop - shares the intermediate block as part of its bitmap? */
    j = 0;
    do {
        sum -= j * shared;
        j++;
        
        if (__builtin_expect(j % 2 == 0, 1)) {
            sink = j;
        }
    } while (j < m);
    
    sink = sum;
}

/* Function 4: Complex nested loops with breaks */
__attribute__((target("thumb")))
void complex_nesting(int n, int m) {
    int i, j, k;
    int sum = 0;
    
    /* Triple nested loops */
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            /* Innermost loop with early exit */
            k = 0;
            while (k < 10) {
                sum += i + j + k;
                k++;
                
                /* Early exit condition */
                if (__builtin_expect(k > 5 && (i + j) % 2 == 0, 0)) {
                    break;
                }
            }
            
            /* Block only in middle loop */
            if (__builtin_expect(j % 3 == 0, 1)) {
                sum += 2;
            }
        }
        
        /* Block only in outer loop */
        if (__builtin_expect(i % 4 == 0, 0)) {
            sink = sum;
        }
    }
    
    sink = sum;
}

/* Function 5: Irregular control flow with goto */
__attribute__((target("thumb")))
void irregular_loops(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
start_outer:
    if (i >= n) goto end;
    
    sum += i;
    i++;
    
    /* Conditional inner loop */
    if (__builtin_expect(i % 2 == 0, 1)) {
        j = 0;
    start_inner:
        if (j >= m) goto end_inner;
        
        sum += j;
        j++;
        
        /* Shared block accessed via goto */
        if (__builtin_expect(j % 3 == 0, 0)) {
            goto shared_block;
        }
        
        goto start_inner;
    end_inner:
        ;
    }
    
    goto start_outer;

shared_block:
    sum += 100;
    sink = sum;
    
    /* Jump back to inner loop */
    if (j < m) {
        goto start_inner;
    } else {
        goto end_inner;
    }

end:
    sink = sum;
}

/* Function 6: Mixed loop types (for, while, do-while) */
__attribute__((target("thumb")))
void mixed_loops(int n, int m) {
    int i, j, k;
    int sum = 0;
    
    /* for loop */
    for (i = 0; i < n; i++) {
        sum += i;
        
        /* while loop inside */
        j = 0;
        while (j < m) {
            sum += j;
            j++;
            
            /* do-while inside while */
            k = 0;
            do {
                sum += k;
                k++;
            } while (k < 5);
        }
    }
    
    sink = sum;
}

/* Main function to call all test patterns */
int main(void) {
    int n = 100;
    int m = 50;
    
    /* Call all functions to ensure they're not optimized away */
    perfect_nesting(n, m);
    partial_overlap(n, m);
    sibling_loops(n, m);
    complex_nesting(n, m);
    irregular_loops(n, m);
    mixed_loops(n, m);
    
    /* Static assert to ensure compilation */
    static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return 0;
}
