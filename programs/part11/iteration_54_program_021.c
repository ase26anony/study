/* test_hw_doloop.c - Test program to cover hw-doloop.cc lines 429-436 */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Function 1: Perfectly nested loops (Pattern A)
   Outer loop contains inner loop completely */
__attribute__((target("thumb")))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop - will contain inner loop's blocks */
    for (i = 0; i < n; i++) {
        /* Split block to create more complex bitmap */
        if (__builtin_expect(i & 1, 0)) {
            sink = i;
        }
        
        /* Inner loop - subset of outer's blocks */
        for (j = 0; j < m; j++) {
            sum += i * j;
            /* Early exit affects block structure */
            if (j == m - 2) break;
        }
        
        /* Another block in outer loop only */
        if (__builtin_expect(sum > 1000, 0)) {
            sink = sum;
        }
    }
    
    sink = sum;
}

/* Function 2: Partially overlapping loops (Pattern B)
   Two loops share some blocks but not completely nested */
__attribute__((target("thumb")))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared setup block */
    int shared = n * m;
    
    /* First loop */
    do {
        sum += i;
        i++;
        
        /* Conditional that sometimes executes second loop */
        if (i % 3 == 0) {
            /* Second loop - shares the 'if' block but has its own body */
            j = 0;
            while (j < m) {
                sum += j;
                j++;
                /* Shared exit condition check */
                if (j == m - 1) goto after_second;
            }
        }
        
        /* Block only in first loop */
        sink = i;
        
    } while (i < n);
    
after_second:
    /* Shared cleanup block */
    sink = sum + shared;
}

/* Function 3: Sibling loops with shared preheader (Pattern C) */
__attribute__((target("thumb")))
void sibling_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared preheader block */
    int setup = n + m;
    
    /* First sibling loop */
    i = 0;
    while (i < n) {
        sum += i * 2;
        i++;
        /* Split block */
        if (__builtin_expect(i == n/2, 0)) {
            sink = i;
        }
    }
    
    /* Shared block between loops */
    sum += setup;
    
    /* Second sibling loop - shares the setup block */
    j = 0;
    do {
        sum += j * 3;
        j++;
        
        /* Different internal structure */
        switch (j % 3) {
            case 0: sink = j; break;
            case 1: sum += 1; break;
            default: break;
        }
    } while (j < m);
    
    sink = sum;
}

/* Function 4: Complex nested structure with multiple levels */
__attribute__((target("thumb")))
void multi_level_nesting(int n, int m, int k) {
    int a, b, c;
    int sum = 0;
    
    /* Level 1 loop */
    for (a = 0; a < n; a++) {
        /* Level 2 loop - perfectly nested in level 1 */
        for (b = 0; b < m; b++) {
            /* Level 3 loop - perfectly nested in level 2 */
            for (c = 0; c < k; c++) {
                sum += a + b + c;
                /* Early exit creates different block structure */
                if (c == k/2) break;
            }
            
            /* Block only in level 2 */
            if (__builtin_expect(b % 7 == 0, 0)) {
                sink = b;
            }
        }
        
        /* Another level 2 loop at same nesting level as first */
        b = 0;
        while (b < m) {
            sum -= b;
            b++;
            /* Shared block pattern with first level 2 loop */
            if (b == m - 1) {
                sink = sum;
            }
        }
    }
}

/* Function 5: Irregular control flow with goto creating overlapping loops */
__attribute__((target("thumb")))
void irregular_loops(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
start_outer:
    if (i >= n) goto end;
    
    sum += i;
    i++;
    
    /* Conditional inner loop */
    if (i % 2 == 0) {
        j = 0;
    inner_start:
        if (j >= m) goto inner_end;
        sum += j;
        j++;
        
        /* Jump back creates loop that shares blocks with outer */
        if (j < m/2) goto inner_start;
        
        /* Block shared between inner and outer */
        sink = j;
        goto inner_start;
    inner_end:
        ;
    }
    
    /* Block that might be shared depending on path */
    if (sum > 100) {
        sink = sum;
    }
    
    goto start_outer;
end:
    sink = sum;
}

/* Main function to call all test patterns */
int main(void) {
    int test_n = 100;
    int test_m = 50;
    int test_k = 25;
    
    /* Call each function multiple times with different parameters
       to ensure various paths are taken */
    for (int iter = 0; iter < 10; iter++) {
        perfect_nesting(test_n + iter, test_m);
        partial_overlap(test_n - iter, test_m + iter);
        sibling_loops(test_n, test_m - iter);
        multi_level_nesting(test_n / (iter + 1), test_m, test_k);
        irregular_loops(test_n, test_m + iter);
    }
    
    /* Compile-time check to ensure optimization doesn't remove everything */
    static_assert(sizeof(int) == 4, "int must be 4 bytes for this test");
    
    return 0;
}
