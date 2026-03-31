/* test_hw_doloop.c
 * Test program to exercise GCC's hardware loop optimization pass
 * Specifically targets lines 429-436 in hw-doloop.cc
 */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Function 1: Perfectly nested loops (Pattern A)
 * Outer loop completely contains inner loop
 * Should trigger: bitmap_intersect_p = true
 *                 bitmap_intersect_compl_p(inner, outer) = false
 *                 => inner loop pushed to outer's loops vector
 */
__attribute__((target("arch=armv7-a")))
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
            /* Early exit to affect block structure */
            if (j == m/2 && (i & 1)) {
                break;
            }
        }
        
        /* Another split in outer loop */
        if (__builtin_expect(sum > 1000, 0)) {
            sink = sum;
        }
    }
    
    sink = sum;
}

/* Function 2: Partially overlapping loops (Pattern B)
 * Two loops share some blocks but aren't perfectly nested
 * Should trigger: bitmap_intersect_p = true
 *                 both bitmap_intersect_compl_p checks = true
 *                 => neither safe_push occurs
 */
__attribute__((target("arch=armv7-a")))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared setup block */
    if (n > 0 && m > 0) {
        sink = n + m;
    }
    
    /* First loop */
    do {
        sum += i;
        i++;
        
        /* Conditional entry to second loop's blocks */
        if (i % 3 == 0) {
            /* This block belongs to both loops' reachable sets */
            j = 0;
            goto second_loop_entry;
        }
    } while (i < n);
    
    goto done;
    
second_loop_entry:
    /* Second loop - partially overlaps with first */
    while (j < m) {
        sum -= j;
        j++;
        
        /* Jump back to first loop's blocks */
        if (j % 2 == 0 && i < n) {
            goto first_loop_body;
        }
    }
    
    if (i < n) {
first_loop_body:
        /* Re-enter first loop */
        goto second_loop_entry;
    }
    
done:
    sink = sum;
}

/* Function 3: Sibling loops with shared preheader (Pattern C)
 * Two sequential loops sharing common setup blocks
 */
__attribute__((target("arch=armv7-a")))
void sibling_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared preheader block */
    int shared = n + m;
    sink = shared;
    
    /* First loop */
    i = 0;
    while (i < n) {
        sum += i * 2;
        i++;
        
        /* Split block */
        if (__builtin_expect(i == n/2, 0)) {
            sink = i;
        }
    }
    
    /* Shared middle block */
    shared = sum % 256;
    
    /* Second loop - uses same shared block */
    for (j = 0; j < m; j++) {
        sum -= j;
        
        /* Complex exit condition */
        if (j > m/2 && sum < 0) {
            break;
        }
    }
    
    sink = sum + shared;
}

/* Function 4: Complex nested structure with multiple levels
 * Creates deeper hierarchy for loop tree construction
 */
__attribute__((target("arch=armv7-a")))
void multi_level_nesting(int n, int m, int k) {
    int a, b, c;
    int result = 0;
    
    /* Level 1 */
    for (a = 0; a < n; a++) {
        /* Always-true if to split block */
        if (__builtin_expect(1, 1)) {
            result += a;
        }
        
        /* Level 2 */
        b = 0;
        while (b < m) {
            /* Level 3 - innermost */
            for (c = 0; c < k; c++) {
                result += a * b * c;
                
                /* Conditional continue */
                if (c % 2 == 0) {
                    continue;
                }
                
                /* Another block split */
                if (__builtin_expect(result > 1000000, 0)) {
                    sink = result;
                }
            }
            
            b++;
            
            /* Early exit from level 2 */
            if (b > m/3 && a > n/2) {
                break;
            }
        }
        
        /* Another loop at same level as while */
        do {
            result -= a;
            sink = result;
        } while (result > 0 && a < n/2);
    }
    
    sink = result;
}

/* Function 5: Mixed loop types with irregular control flow
 * Uses switch inside loop to create complex block patterns
 */
__attribute__((target("arch=armv7-a")))
void irregular_loops(int n) {
    int i = 0;
    int state = 0;
    int total = 0;
    
    while (i < n) {
        switch (state) {
            case 0:
                total += i;
                state = 1;
                /* Fall through */
            case 1:
                total *= 2;
                state = 2;
                break;
            case 2:
                total -= i;
                state = (i % 3 == 0) ? 0 : 3;
                break;
            case 3:
                total /= 2;
                state = 0;
                i++;
                break;
        }
        
        /* Nested for loop with different structure */
        if (state == 0) {
            int j;
            for (j = 0; j < 5; j++) {
                total += j;
                if (j == 3) {
                    /* Jump creates overlap */
                    goto switch_merge;
                }
            }
        }
        
        if (state == 2) {
            int k = 0;
            do {
                total += k * 2;
                k++;
            } while (k < 3 && total < 1000);
        }
        
switch_merge:
        /* Empty to create merge block */
        ;
    }
    
    sink = total;
}

/* Main function to ensure all functions are called */
int main(void) {
    /* Prevent dead code elimination */
    __attribute__((used)) static volatile int force_call = 0;
    
    /* Call each function with different parameters */
    perfect_nesting(100, 50);
    
    if (force_call) {
        partial_overlap(100, 50);
        sibling_loops(100, 50);
        multi_level_nesting(10, 20, 30);
        irregular_loops(100);
    } else {
        /* Ensure all functions are called */
        partial_overlap(50, 25);
        sibling_loops(75, 25);
        multi_level_nesting(5, 10, 15);
        irregular_loops(50);
    }
    
    /* Compile-time check to ensure optimization */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return sink != 0;
}
