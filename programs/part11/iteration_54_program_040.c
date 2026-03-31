/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Function 1: Perfectly nested loops (Pattern A) */
__attribute__((used, noinline))
int perfect_nesting(int n, int m) {
    int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; i++) {
        /* Inner loop - perfectly nested */
        for (int j = 0; j < m; j++) {
            sum += i * j;
            /* Split basic block inside inner loop */
            if (__builtin_expect(sum & 1, 0)) {
                sink = sum;
            }
        }
        /* Early exit from outer loop */
        if (i > n/2) {
            break;
        }
    }
    return sum;
}

/* Function 2: Partially overlapping loops (Pattern B) */
__attribute__((used, noinline))
int partial_overlap(int n, int m) {
    int sum = 0;
    int i = 0;
    
    /* First loop with complex control flow */
    do {
        sum += i;
        /* Conditional that creates shared block */
        if (i % 2 == 0) {
            /* Second loop that partially overlaps */
            int j = 0;
            while (j < m) {
                sum += j;
                j++;
                /* Shared exit condition */
                if (sum > 1000) goto cleanup;
            }
        }
        i++;
    } while (i < n);
    
cleanup:
    /* Additional block shared by both loops through goto */
    return sum;
}

/* Function 3: Sibling loops with shared preheader (Pattern C) */
__attribute__((used, noinline))
int sibling_loops(int n, int m) {
    int sum = 0;
    int shared = n * m;  /* Shared computation */
    
    /* Shared preheader block */
    if (shared > 0) {
        sink = shared;
    }
    
    /* First sibling loop */
    for (int i = 0; i < n; i++) {
        sum += i;
        /* Split block to create more complex bitmap */
        if (__builtin_expect(i == n/2, 0)) {
            sink = i;
        }
    }
    
    /* Second sibling loop (shares the preheader) */
    for (int j = 0; j < m; j++) {
        sum -= j;
        /* Different internal structure */
        switch (j % 3) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 2: sum += 3; break;
        }
    }
    
    return sum;
}

/* Function 4: Complex nested structure with multiple exits */
__attribute__((used, noinline))
int complex_nesting(int n, int m, int k) {
    int sum = 0;
    
    /* Level 1 loop */
    for (int a = 0; a < n; a++) {
        /* Level 2 loop */
        int b = 0;
        while (b < m) {
            /* Level 3 loop - innermost */
            for (int c = 0; c < k; c++) {
                sum += a + b + c;
                
                /* Multiple exit points */
                if (sum > 10000) {
                    goto outer_exit;
                }
                if (c == k/2) {
                    break;
                }
            }
            b++;
            
            /* Another exit condition */
            if (b > m/2) {
                goto middle_exit;
            }
        }
        
        /* Label for goto target */
        middle_exit:
        if (a == n-1) {
            break;
        }
    }
    
outer_exit:
    return sum;
}

/* Function 5: Irregular control flow with overlapping bitmaps */
__attribute__((used, noinline))
int irregular_loops(int n) {
    int sum = 0;
    int i = 0;
    
    /* Loop with irregular structure */
    start_loop1:
    if (i >= n) goto end_loop1;
    
    sum += i;
    
    /* Nested loop that shares some blocks */
    if (i % 3 == 0) {
        int j = 0;
        loop2_start:
        if (j >= 5) goto loop2_end;
        
        sum += j;
        j++;
        
        /* Shared block between loops */
        shared_block:
        sink = sum;
        goto loop2_start;
        
        loop2_end:;
    }
    
    i++;
    goto start_loop1;
    
end_loop1:
    return sum;
}

/* Main function to ensure all functions are called */
int main(void) {
    int result = 0;
    
    /* Call each function with different parameters */
    result += perfect_nesting(100, 50);
    result += partial_overlap(100, 50);
    result += sibling_loops(100, 50);
    result += complex_nesting(30, 20, 10);
    result += irregular_loops(100);
    
    /* Use result to prevent dead code elimination */
    sink = result;
    
    return result > 0 ? 0 : 1;
}
