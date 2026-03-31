/* test_hw_doloop.c - Test program to cover hw-doloop.cc lines 429-436 */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of loops */
static volatile int g_counter = 0;

/* Pattern A: Perfectly nested loops - inner loop bitmap is subset of outer */
__attribute__((noinline))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Add some basic blocks inside outer but outside inner */
        if (__builtin_expect(g_counter > 0, 0)) {
            sum += 1;
        }
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            sum += i * j;
            /* Split inner loop block */
            if (__builtin_expect(j % 2 == 0, 1)) {
                sum += 1;
            }
        }
        
        /* More outer loop blocks */
        if (__builtin_expect(i % 3 == 0, 0)) {
            sum += 2;
        }
    }
    
    g_counter += sum;
}

/* Pattern B: Partially overlapping loops - share some blocks but not subsets */
__attribute__((noinline))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared pre-header block */
    int shared = g_counter;
    
    /* First loop */
    while (i < n) {
        sum += i;
        
        /* Block shared with second loop */
        if (shared > 0) {
            sum += shared;
        }
        
        i++;
        
        /* Early exit creates different block structure */
        if (i > n/2 && __builtin_expect(g_counter < 100, 1)) {
            break;
        }
    }
    
    /* Reset j */
    j = 0;
    
    /* Second loop that shares the if(shared>0) block via goto */
    if (m > 0) {
        shared_block:
        if (shared > 0) {
            sum += shared * 2;
        }
    }
    
    while (j < m) {
        sum += j * 2;
        
        /* Jump to shared block */
        if (j == m/2) {
            goto shared_block;
        }
        
        j++;
    }
    
    g_counter += sum;
}

/* Pattern C: Sequential loops with shared setup block */
__attribute__((noinline))
void sequential_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared setup block - may be included in both loop bitmaps */
    int setup = g_counter * 2;
    
    /* First loop */
    i = 0;
    do {
        sum += i + setup;
        /* Split block in first loop */
        if (__builtin_expect(i % 4 == 0, 0)) {
            setup += 1;
        }
        i++;
    } while (i < n);
    
    /* Second loop - reuses setup */
    for (j = 0; j < m; j++) {
        sum += j * setup;
        /* Different block structure in second loop */
        switch (j % 3) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            default: sum += 3; break;
        }
    }
    
    g_counter += sum;
}

/* Pattern D: Complex nested structure with multiple levels */
__attribute__((noinline))
void multi_level_nesting(int n, int m, int k) {
    int i, j, l;
    int sum = 0;
    
    /* Level 1 loop */
    for (i = 0; i < n; i++) {
        /* Level 2 loop - not perfectly nested due to this block */
        if (__builtin_expect(i % 2 == 0, 1)) {
            sum += i;
        }
        
        /* Level 2 loop */
        j = 0;
        while (j < m) {
            /* Level 3 loop - perfectly nested inside level 2 */
            for (l = 0; l < k; l++) {
                sum += i * j * l;
                /* Create multiple blocks in innermost loop */
                if (__builtin_expect(l % 5 == 0, 0)) {
                    sum -= 1;
                } else {
                    sum += 2;
                }
            }
            
            j++;
            /* Early continue affects bitmap */
            if (__builtin_expect(j % 7 == 0, 0)) {
                continue;
            }
            sum += j;
        }
        
        /* More level 1 blocks */
        switch (i % 4) {
            case 0: sum += 10; break;
            case 1: sum += 20; break;
            case 2: sum += 30; break;
            case 3: sum += 40; break;
        }
    }
    
    g_counter += sum;
}

/* Pattern E: Loops with irregular control flow using switch */
__attribute__((noinline))
void irregular_loops(int n) {
    int i = 0;
    int state = 0;
    int sum = 0;
    
    while (i < n) {
        /* This switch creates multiple basic blocks */
        switch (state) {
            case 0:
                sum += i;
                state = 1;
                break;
            case 1:
                sum += i * 2;
                /* Nested loop inside case */
                for (int j = 0; j < 3; j++) {
                    sum += j;
                    if (__builtin_expect(j == 1, 0)) {
                        sum += 10;
                    }
                }
                state = 2;
                break;
            case 2:
                sum += i * 3;
                state = 0;
                break;
        }
        
        i++;
        
        /* Another loop that shares the switch blocks */
        if (i == n/2) {
            state = 0;
            /* goto creates overlapping block structure */
            goto switch_again;
        }
    }
    
    return;
    
switch_again:
    while (state < 3) {
        sum += state * 100;
        state++;
    }
    
    g_counter += sum;
}

/* Main function to ensure all patterns are executed */
int main(void) {
    /* Call each pattern multiple times with different parameters
       to ensure various loop structures are created */
    
    /* Pattern A: Perfect nesting */
    for (int run = 0; run < 10; run++) {
        perfect_nesting(100, 50);
    }
    
    /* Pattern B: Partial overlap */
    for (int run = 0; run < 10; run++) {
        partial_overlap(100, 75);
    }
    
    /* Pattern C: Sequential loops */
    for (int run = 0; run < 10; run++) {
        sequential_loops(100, 60);
    }
    
    /* Pattern D: Multi-level nesting */
    for (int run = 0; run < 10; run++) {
        multi_level_nesting(50, 40, 30);
    }
    
    /* Pattern E: Irregular loops */
    for (int run = 0; run < 10; run++) {
        irregular_loops(80);
    }
    
    /* Prevent dead code elimination */
    if (g_counter > 1000) {
        return 1;
    }
    
    return 0;
}
