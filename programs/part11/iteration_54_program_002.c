/* test_hw_loops.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_loops.c -o test_hw_loops */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop completely contained in outer */
__attribute__((noinline))
void pattern_a_perfect_nesting(int n, int m) {
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
            /* Another basic block split */
            if (__builtin_expect(j & 1, 0)) {
                sink = j;
            }
        }
        
        /* More code in outer loop after inner loop */
        if (__builtin_expect(sum > 1000, 0)) {
            sink = sum;
        }
    }
    
    sink = sum;
}

/* Pattern B: Partially overlapping loops - share some blocks but not perfectly nested */
__attribute__((noinline))
void pattern_b_partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared setup block */
    int shared = n * m;
    
    /* First loop */
    while (i < n) {
        sum += i;
        
        /* Conditional that might execute second loop */
        if (i > n/2) {
            /* Second loop inside conditional - partially overlaps */
            j = 0;
            do {
                sum += j;
                j++;
                /* Shared computation */
                if (__builtin_expect(j & 3, 0)) {
                    sink = shared;
                }
            } while (j < m);
        }
        
        i++;
        
        /* Another shared block */
        if (__builtin_expect(sum & 1, 0)) {
            sink = shared;
        }
    }
    
    /* Code after first loop that's not in second loop */
    sink = sum * 2;
}

/* Pattern C: Sequential loops with shared preheader/setup */
__attribute__((noinline))
void pattern_c_sequential_shared(int n, int m) {
    int i, j;
    int sum1 = 0, sum2 = 0;
    
    /* Shared setup block - will be in both loop bitmaps if not careful */
    int setup = n + m;
    sink = setup;
    
    /* First loop */
    for (i = 0; i < n; i++) {
        sum1 += i * setup;
        /* Early exit creates more complex control flow */
        if (i > n/3 && __builtin_expect(sum1 > 100, 0)) {
            break;
        }
    }
    
    /* Shared intermediate block */
    if (__builtin_expect(setup > 10, 0)) {
        sink = sum1;
    }
    
    /* Second loop - sequential but shares some setup */
    for (j = 0; j < m; j++) {
        sum2 += j * setup;
        /* Different control flow in second loop */
        switch (j & 3) {
            case 0: sum2 += 1; break;
            case 1: sum2 += 2; break;
            case 2: sum2 += 3; break;
            default: sum2 += 4; break;
        }
    }
    
    sink = sum1 + sum2;
}

/* Pattern D: Complex nested structure with multiple levels */
__attribute__((noinline))
void pattern_d_complex_nesting(int n, int m, int k) {
    int i, j, l;
    int sum = 0;
    
    /* Level 1 loop */
    for (i = 0; i < n; i++) {
        /* Split block */
        if (__builtin_expect(i & 1, 0)) {
            sink = i;
        }
        
        /* Level 2 loop - middle level */
        j = 0;
        while (j < m) {
            sum += i * j;
            
            /* Level 3 loop - innermost */
            for (l = 0; l < k; l++) {
                sum += l;
                /* Complex control in innermost */
                if (__builtin_expect(l == k/2, 0)) {
                    sum *= 2;
                }
            }
            
            j++;
            
            /* Early exit from middle loop */
            if (sum > 10000) {
                break;
            }
        }
        
        /* Another loop at same level as while - sibling */
        for (l = 0; l < i && l < 10; l++) {
            sum -= l;
        }
    }
    
    sink = sum;
}

/* Pattern E: Loops with goto creating irregular overlap */
__attribute__((noinline))
void pattern_e_goto_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Label for goto */
    restart:
    
    /* First loop structure */
    while (i < n) {
        sum += i;
        i++;
        
        /* Conditional goto creates overlap */
        if (i == n/2) {
            j = 0;
            /* Second loop entered via goto */
            goto inner_loop;
        }
    }
    
    goto done;
    
    inner_loop:
    /* Second loop - overlaps with first due to goto */
    while (j < m) {
        sum += j;
        j++;
        
        /* Can goto back to first loop */
        if (j == m/2) {
            goto restart;
        }
    }
    
    done:
    sink = sum;
}

/* Pattern F: Switch statement inside loop creating many basic blocks */
__attribute__((noinline))
void pattern_f_switch_in_loop(int n) {
    int i;
    int sum = 0;
    
    for (i = 0; i < n; i++) {
        /* Switch creates multiple basic blocks within loop */
        switch (i % 5) {
            case 0:
                sum += i;
                /* Nested if to split block further */
                if (__builtin_expect(i & 1, 0)) {
                    sum += 1;
                }
                break;
            case 1:
                sum += i * 2;
                break;
            case 2:
                sum += i * 3;
                /* Another conditional */
                if (i > n/2) {
                    sum -= 1;
                }
                break;
            case 3:
                sum += i * 4;
                /* Small inner loop */
                for (int j = 0; j < 3; j++) {
                    sum += j;
                }
                break;
            default:
                sum += i * 5;
                break;
        }
    }
    
    sink = sum;
}

/* Main function to ensure all patterns are called */
int main() {
    /* Call each pattern multiple times with different parameters
       to ensure various execution paths are taken */
    
    /* Pattern A - perfect nesting */
    for (int run = 0; run < 3; run++) {
        pattern_a_perfect_nesting(100, 50);
        pattern_a_perfect_nesting(10, 20);
        pattern_a_perfect_nesting(50, 10);
    }
    
    /* Pattern B - partial overlap */
    for (int run = 0; run < 2; run++) {
        pattern_b_partial_overlap(100, 50);
        pattern_b_partial_overlap(30, 60);
    }
    
    /* Pattern C - sequential with shared setup */
    pattern_c_sequential_shared(100, 50);
    pattern_c_sequential_shared(40, 80);
    
    /* Pattern D - complex multi-level nesting */
    pattern_d_complex_nesting(20, 30, 10);
    pattern_d_complex_nesting(50, 10, 5);
    
    /* Pattern E - goto overlap */
    pattern_e_goto_overlap(40, 30);
    pattern_e_goto_overlap(20, 40);
    
    /* Pattern F - switch in loop */
    pattern_f_switch_in_loop(100);
    pattern_f_switch_in_loop(50);
    
    return 0;
}
