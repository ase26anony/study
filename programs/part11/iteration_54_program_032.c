/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop is subset of outer */
__attribute__((noinline))
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
        if (__builtin_expect(i & 1, 0)) {
            sink = i;
        }
    }
    
    sink = sum;
}

/* Pattern B: Partially overlapping loops with shared blocks */
__attribute__((noinline))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared setup block */
    int shared = n * m;
    
    /* First loop */
    while (i < n) {
        sum += i;
        i++;
        
        /* Conditional entry to second loop */
        if (i == n/2) {
            /* Second loop that shares some blocks */
            j = 0;
            while (j < m) {
                sum += j;
                j++;
                
                /* Shared computation block */
                if (__builtin_expect(j & 1, 0)) {
                    sink = shared;
                }
            }
        }
    }
    
    /* More code after loops */
    if (__builtin_expect(sum > 1000, 0)) {
        sink = sum;
    }
}

/* Pattern C: Sequential loops with shared preheader */
__attribute__((noinline))
void sequential_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared preheader block */
    int setup = n + m;
    sink = setup;
    
    /* First loop */
    for (i = 0; i < n; i++) {
        sum += i;
        /* Early exit creates different block structure */
        if (__builtin_expect(i > n/2, 0)) {
            break;
        }
    }
    
    /* Code between loops */
    sum *= 2;
    
    /* Second loop - sibling, not nested */
    for (j = 0; j < m; j++) {
        sum -= j;
        /* Different internal structure */
        if (__builtin_expect(j == m/2, 1)) {
            sink = j;
        }
    }
}

/* Pattern D: Complex nested structure with multiple levels */
__attribute__((noinline))
void multi_level_nesting(int n, int m, int k) {
    int a, b, c;
    int sum = 0;
    
    /* Level 1 */
    for (a = 0; a < n; a++) {
        /* Level 2 */
        for (b = 0; b < m; b++) {
            /* Level 3 - innermost */
            for (c = 0; c < k; c++) {
                sum += a + b + c;
                
                /* Create multiple basic blocks */
                switch (c % 3) {
                    case 0: sink = a; break;
                    case 1: sink = b; break;
                    case 2: sink = c; break;
                }
            }
            
            /* Middle loop has its own blocks */
            if (__builtin_expect(b == m/2, 0)) {
                sum *= 2;
            }
        }
        
        /* Outer loop block not in inner loops */
        sum -= a;
    }
}

/* Pattern E: Do-while loops with irregular structure */
__attribute__((noinline))
void irregular_loops(int n) {
    int i = 0, j = n;
    int sum = 0;
    
    /* First loop - do-while */
    do {
        sum += i;
        i++;
        
        /* Nested while loop with partial overlap */
        if (i % 3 == 0) {
            int k = 0;
            while (k < 5) {
                sum += k;
                k++;
                /* Shared block with outer loop */
                sink = i;
            }
        }
    } while (i < n);
    
    /* Second independent loop */
    while (j > 0) {
        sum -= j;
        j--;
        
        /* Goto creates interesting control flow */
        if (j == n/2) {
            goto skip;
        }
    }
    
skip:
    sink = sum;
}

/* Pattern F: Loops with function calls (may affect bitmap) */
static int helper(int x) {
    return x * 2;
}

__attribute__((noinline))
void loops_with_calls(int n) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Function call creates separate basic blocks */
        sum += helper(i);
        
        /* Inner loop after call */
        for (j = 0; j < i; j++) {
            sum -= j;
            if (__builtin_expect(j == i/2, 0)) {
                sink = sum;
            }
        }
    }
}

/* Main function to ensure all patterns are executed */
int main(void) {
    int test_n = 100;
    int test_m = 50;
    int test_k = 20;
    
    /* Execute all patterns */
    perfect_nesting(test_n, test_m);
    partial_overlap(test_n, test_m);
    sequential_loops(test_n, test_m);
    multi_level_nesting(test_n, test_m, test_k);
    irregular_loops(test_n);
    loops_with_calls(test_n);
    
    /* Ensure compiler doesn't optimize everything away */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes for consistent behavior");
    
    return 0;
}
