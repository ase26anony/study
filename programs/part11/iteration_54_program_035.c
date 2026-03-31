/* test_hw_doloop.c
 * 
 * This program is designed to trigger specific conditions in GCC's
 * hardware loop optimization pass (hw-doloop.cc lines 429-436).
 * It creates various loop nesting patterns to exercise bitmap
 * intersection and complement checks.
 */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Function 1: Perfectly nested loops (Pattern A)
 * Outer loop completely contains inner loop
 * Should trigger: bitmap_intersect_p = true
 *                 bitmap_intersect_compl_p(other, loop) = false
 *                 => inner loop pushed to outer's loops vector
 */
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
            /* Another block splitter */
            if (__builtin_expect(j & 1, 1)) {
                sink = j;
            }
        }
        
        /* Early exit to affect block structure */
        if (i > n/2) {
            break;
        }
    }
    
    sink = sum;
}

/* Function 2: Partially overlapping loops (Pattern B)
 * Two loops share some blocks but neither contains the other
 * Should trigger: bitmap_intersect_p = true
 *                 both bitmap_intersect_compl_p checks = true
 *                 => neither safe_push occurs
 */
__attribute__((noinline))
void partial_overlap(int n) {
    int i, j;
    int sum = 0;
    
    /* Shared setup block */
    int shared = n * 2;
    sink = shared;
    
    /* First loop */
    i = 0;
    while (i < n) {
        sum += i;
        
        /* Conditional that might execute second loop */
        if (i > n/3 && i < 2*n/3) {
            /* Second loop inside first loop's body but not nested */
            j = 0;
            do {
                sum -= j;
                j++;
                /* Shared block between loops */
                if (__builtin_expect(j & 3, 0)) {
                    sink = j;
                }
            } while (j < i);
        }
        
        i++;
        
        /* Another shared operation */
        if (__builtin_expect(i & 1, 1)) {
            sink = i;
        }
    }
    
    /* Second loop that starts after first but shares some blocks */
    for (j = 0; j < n/2; j++) {
        /* This block is shared with the do-while above */
        if (__builtin_expect(j & 3, 0)) {
            sink = j;
        }
        sum += j * 2;
    }
    
    sink = sum;
}

/* Function 3: Sibling loops with shared preheader (Pattern C)
 * Two sequential loops sharing a common setup block
 */
__attribute__((noinline))
void sibling_loops(int n) {
    int i;
    int sum = 0;
    
    /* Shared preheader block */
    int setup = n + 10;
    sink = setup;
    
    /* First loop */
    for (i = 0; i < n; i++) {
        sum += i;
        /* Block splitter */
        if (__builtin_expect(i == n/2, 0)) {
            sink = i;
            break;  /* Early exit affects bitmap */
        }
    }
    
    /* Shared block between loops */
    setup = sum;
    sink = setup;
    
    /* Second loop - shares the setup block */
    for (i = n-1; i >= 0; i--) {
        sum -= i;
        /* Different block structure */
        switch (i % 3) {
            case 0: sink = i; break;
            case 1: sum += 1; break;
            case 2: sum -= 1; break;
        }
    }
    
    sink = sum;
}

/* Function 4: Complex nested structure with multiple levels */
__attribute__((noinline))
void complex_nesting(int n) {
    int i, j, k;
    int sum = 0;
    
    /* Level 1 loop */
    for (i = 0; i < n; i++) {
        /* Level 2 loop - perfectly nested */
        for (j = 0; j < i; j++) {
            /* Level 3 loop - triple nesting */
            for (k = 0; k < j; k++) {
                sum += i * j * k;
            }
            
            /* Partial overlap with another loop structure */
            if (j > i/2) {
                /* Another loop at same level as j loop but not nested */
                int m = 0;
                while (m < 5) {
                    sum -= m;
                    m++;
                    /* Shared with outer loops */
                    if (__builtin_expect(m == 3, 0)) {
                        sink = m;
                    }
                }
            }
        }
        
        /* Another loop at i level */
        int p = 0;
        do {
            sum += p * i;
            p++;
        } while (p < 3);
    }
    
    sink = sum;
}

/* Function 5: Irregular control flow with goto */
__attribute__((noinline))
void irregular_loops(int n) {
    int i = 0;
    int sum = 0;
    
    /* Loop with goto creating overlapping blocks */
start_loop1:
    if (i >= n) goto end_loop1;
    
    sum += i;
    i++;
    
    /* Conditional jump to shared block */
    if (i % 2 == 0) {
        goto shared_block;
    }
    
    goto start_loop1;

shared_block:
    sink = i;
    
    /* Second loop overlapping with first */
    int j = 0;
loop2:
    if (j >= i) goto end_loop2;
    
    sum -= j;
    j++;
    
    /* Jump back to first loop's block */
    if (j % 3 == 0) {
        goto start_loop1;
    }
    
    goto loop2;

end_loop2:
end_loop1:
    sink = sum;
}

/* Main function to ensure all functions are called */
int main(void) {
    const int N = 100;
    const int M = 50;
    
    /* Prevent dead code elimination */
    __attribute__((used)) volatile int result = 0;
    
    /* Call all functions with different parameters */
    perfect_nesting(N, M);
    partial_overlap(N);
    sibling_loops(N);
    complex_nesting(N/2);
    irregular_loops(N/3);
    
    /* Compile-time check to ensure compilation proceeds */
    _Static_assert(sizeof(int) >= 2, "int must be at least 16 bits");
    
    return 0;
}
