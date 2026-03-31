/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Function 1: Perfectly nested loops (Pattern A)
   Should trigger: bitmap_intersect_p = true, 
                   bitmap_intersect_compl_p(inner, outer) = false
                   => outer->loops.safe_push(inner) */
__attribute__((noinline))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Split basic block */
        if (__builtin_expect(i & 1, 0)) {
            sink = i;
        }
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            sum += i * j;
            /* Another block splitter */
            if (__builtin_expect(j & 1, 1)) {
                sink = j;
            }
        }
        
        /* Early exit possibility */
        if (sum > 1000) {
            break;
        }
    }
    
    sink = sum;
}

/* Function 2: Partially overlapping loops (Pattern B)
   Should trigger: bitmap_intersect_p = true,
                   both bitmap_intersect_compl_p checks = true
                   => neither safe_push, continue after first if */
__attribute__((noinline))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared setup block */
    int shared = n * 2;
    
    /* First loop */
    do {
        sum += i;
        i++;
        
        /* Conditional entry to second loop */
        if (i == n/2) {
            /* Second loop that shares some blocks */
            j = 0;
            while (j < m) {
                sum += shared * j;
                j++;
                
                /* Shared computation block */
                if (j & 1) {
                    shared++;
                }
            }
        }
        
        /* Block only in first loop */
        if (i & 1) {
            sink = i;
        }
    } while (i < n);
    
    sink = sum;
}

/* Function 3: Sibling loops with shared preheader (Pattern C)
   Should trigger: bitmap_intersect_p = true,
                   one bitmap_intersect_compl_p check = false
                   => one safe_push occurs */
__attribute__((noinline))
void sibling_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared preheader block */
    int setup = n + m;
    sink = setup;
    
    /* First sibling loop */
    i = 0;
    while (i < n) {
        sum += i * setup;
        i++;
        
        /* Split block */
        if (__builtin_expect(setup > 0, 1)) {
            setup--;
        }
    }
    
    /* Shared middle block */
    setup = sum % 100;
    
    /* Second sibling loop - shares setup block */
    j = 0;
    do {
        sum += j * setup;
        j++;
        
        /* Different block structure */
        switch (j % 3) {
            case 0: sink = j; break;
            case 1: setup += j; break;
            default: break;
        }
    } while (j < m);
    
    sink = sum;
}

/* Function 4: Complex nested structure with multiple exits */
__attribute__((noinline))
void complex_nesting(int n, int m, int k) {
    int a, b, c;
    int result = 0;
    
    /* Triple nesting */
    for (a = 0; a < n; a++) {
        /* Middle loop with early exit */
        b = 0;
        while (b < m) {
            result += a * b;
            
            /* Innermost loop */
            for (c = 0; c < k; c++) {
                result -= c;
                
                /* Conditional break in innermost */
                if (result < -1000) {
                    goto inner_exit;
                }
                
                /* Block splitter */
                if (__builtin_expect(c & 1, 0)) {
                    sink = c;
                }
            }
        inner_exit:
            
            b++;
            
            /* Another early exit */
            if (b > m/2 && (a & 1)) {
                break;
            }
        }
        
        /* Block only in outer loop */
        if (a % 7 == 0) {
            result += 100;
        }
    }
    
    sink = result;
}

/* Function 5: Irregular control flow with goto creating overlap */
__attribute__((noinline))
void irregular_overlap(int n) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Loop with goto creating overlapping regions */
    for (i = 0; i < n; i++) {
        sum += i;
        
        if (i == n/3) {
            /* Jump into what looks like another loop */
            j = 0;
        shared_block:
            sum += j * 2;
            j++;
            
            if (j < 5) {
                goto shared_block;
            }
        }
        
        /* Continue with first loop */
        if (i & 1) {
            sum *= 2;
        }
    }
    
    sink = sum;
}

/* Main function to ensure all functions are called */
int main(void) {
    int iterations = 100;
    
    /* Call each function with different parameters
       to create varied loop structures */
    perfect_nesting(iterations, 50);
    partial_overlap(iterations, 30);
    sibling_loops(iterations, 40);
    complex_nesting(20, 15, 10);
    irregular_overlap(iterations);
    
    return 0;
}
