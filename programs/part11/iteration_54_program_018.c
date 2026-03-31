/* test_hw_doloop.c
 * Test program to exercise GCC's hardware loop optimization pass
 * Specifically targets the loop hierarchy building logic with
 * bitmap intersection and complement checks
 */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop bitmap is subset of outer */
__attribute__((noinline))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Split basic block to create more complex bitmap */
        if (__builtin_expect(i & 1, 0)) {
            sink = i;
        }
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            sum += i * j;
            /* Another split to increase block count */
            if (__builtin_expect(j & 1, 0)) {
                sink = j;
            }
        }
        
        /* Early exit to affect outer loop structure */
        if (i > n/2) {
            break;
        }
    }
    
    sink = sum;
}

/* Pattern B: Partially overlapping loops with shared blocks but not nested */
__attribute__((noinline))
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
            /* Second loop that shares some execution but isn't nested */
            j = 0;
            while (j < m) {
                sum += j;
                j++;
                /* Shared computation */
                if (j % 2 == 0) {
                    sink = shared;
                }
            }
        }
        
        /* Split block */
        if (__builtin_expect(i & 1, 0)) {
            sink = i;
        }
    } while (i < n);
    
    sink = sum;
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
    i = 0;
    while (i < n) {
        sum += i * 2;
        i++;
        /* Early exit affects bitmap */
        if (i > n - 3) break;
    }
    
    /* Shared intermediate block */
    sink = sum;
    
    /* Second loop - not nested, but shares setup block */
    for (j = 0; j < m; j++) {
        sum -= j;
        /* Split block inside loop */
        if (__builtin_expect(j & 1, 0)) {
            sink = j;
        }
    }
    
    sink = sum;
}

/* Pattern D: Complex nested loops with multiple levels */
__attribute__((noinline))
void multi_level_nesting(int n, int m, int k) {
    int a, b, c;
    int sum = 0;
    
    /* Level 1 */
    for (a = 0; a < n; a++) {
        /* Level 2 */
        b = 0;
        while (b < m) {
            sum += a + b;
            b++;
            
            /* Level 3 - innermost */
            for (c = 0; c < k; c++) {
                sum += c;
                /* Conditional to split blocks */
                if (__builtin_expect(c & 3, 0)) {
                    sink = c;
                }
            }
            
            /* Early exit at level 2 */
            if (b > m/2) break;
        }
        
        /* Another split at level 1 */
        if (__builtin_expect(a & 1, 0)) {
            sink = a;
        }
    }
    
    sink = sum;
}

/* Pattern E: Loops with goto creating irregular control flow */
__attribute__((noinline))
void irregular_flow(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
start_outer:
    if (i >= n) goto end;
    
    sum += i;
    i++;
    
    /* Conditional goto to create overlap */
    if (i % 2 == 0) {
        j = 0;
    start_inner:
        if (j >= m) goto after_inner;
        sum += j;
        j++;
        
        /* Shared block between loops */
        sink = i + j;
        
        goto start_inner;
    after_inner:
        ;
    }
    
    goto start_outer;
end:
    sink = sum;
}

/* Main function to ensure all patterns are used */
int main(void) {
    /* Force compilation of all functions */
    perfect_nesting(100, 50);
    partial_overlap(100, 50);
    sequential_loops(100, 50);
    multi_level_nesting(10, 20, 30);
    irregular_flow(100, 50);
    
    /* Prevent dead code elimination */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return 0;
}
