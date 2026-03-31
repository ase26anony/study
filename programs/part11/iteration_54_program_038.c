/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of loops */
static volatile int force_loop = 1;

/* Pattern A: Perfectly nested loops - inner loop should be subset of outer */
__attribute__((noinline))
void perfect_nesting(int n, int m) {
    int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n && force_loop; i++) {
        /* Add conditional to create more basic blocks */
        if (__builtin_expect(i % 2, 0)) {
            /* Empty block to affect bitmap */
            asm volatile("" ::: "memory");
        }
        
        /* Inner loop - perfectly nested */
        for (int j = 0; j < m && force_loop; j++) {
            sum += i * j;
            /* Split inner loop block */
            if (__builtin_expect(j % 3, 1)) {
                sum += 1;
            }
        }
        
        /* Another block in outer loop but not in inner */
        if (__builtin_expect(i % 5, 0)) {
            sum -= 1;
        }
    }
    
    /* Prevent dead code elimination */
    if (sum == 0) {
        asm volatile("" ::: "memory");
    }
}

/* Pattern B: Partially overlapping loops with shared blocks but not nested */
__attribute__((noinline))
void partial_overlap(int n, int m) {
    int sum = 0;
    int i = 0, j = 0;
    
    /* Shared setup block */
    int shared = n * m;
    
    /* First loop */
    while (i < n && force_loop) {
        sum += i;
        
        /* Shared block between loops */
        if (shared > 0) {
            sum += shared;
            shared--;
        }
        
        /* Conditional early exit affecting bitmap */
        if (i == n/2) {
            break;
        }
        i++;
    }
    
    /* Reset for second loop */
    i = 0;
    
    /* Second loop that overlaps with first in shared block */
    do {
        sum += i * 2;
        
        /* Same shared block as first loop */
        if (shared > -10) {  /* Different condition to avoid identical CFG */
            sum -= shared;
        }
        
        /* Different block not in first loop */
        if (__builtin_expect(j < m, 1)) {
            sum += j;
            j++;
        }
        
        i++;
    } while (i < n && force_loop);
    
    if (sum == 0) {
        asm volatile("" ::: "memory");
    }
}

/* Pattern C: Sequential loops with shared preheader */
__attribute__((noinline))
void sequential_loops(int n) {
    int sum = 0;
    
    /* Shared preheader block */
    int setup = n * 2;
    
    /* First loop */
    for (int i = 0; i < n && force_loop; i++) {
        sum += i + setup;
        
        /* Block only in first loop */
        if (i % 2 == 0) {
            sum *= 2;
        }
    }
    
    /* Shared middle block */
    setup /= 2;
    
    /* Second loop - sequential */
    for (int j = 0; j < n && force_loop; j++) {
        sum += j - setup;
        
        /* Block only in second loop */
        if (j % 3 == 0) {
            sum /= 2;
        }
    }
    
    if (sum == 0) {
        asm volatile("" ::: "memory");
    }
}

/* Pattern D: Complex nested structure with multiple levels */
__attribute__((noinline))
void complex_nesting(int n, int m, int p) {
    int sum = 0;
    
    /* Level 1 loop */
    for (int i = 0; i < n && force_loop; i++) {
        /* Split block */
        asm volatile("" ::: "memory");
        
        /* Level 2 loop */
        int j = 0;
        while (j < m && force_loop) {
            /* Level 3 loop - innermost */
            for (int k = 0; k < p && force_loop; k++) {
                sum += i * j * k;
                
                /* Innermost-only block */
                if (k % 4 == 0) {
                    sum += 1;
                }
            }
            
            /* Level 2-only block */
            if (j % 5 == 0) {
                sum -= 1;
            }
            j++;
        }
        
        /* Another level 2 loop - sibling to previous */
        for (int j2 = 0; j2 < m/2 && force_loop; j2++) {
            sum += i + j2;
        }
    }
    
    if (sum == 0) {
        asm volatile("" ::: "memory");
    }
}

/* Pattern E: Loops with irregular control flow using goto */
__attribute__((noinline))
void irregular_loops(int n) {
    int sum = 0;
    int i = 0;
    
    /* Loop with goto creating overlapping blocks */
    while (i < n && force_loop) {
        if (i % 3 == 0) {
            goto shared_block;
        }
        
        sum += i * 2;
        i++;
        continue;
        
    shared_block:
        sum += i * 3;
        i++;
        
        /* Nested loop inside shared block */
        for (int j = 0; j < 3 && force_loop; j++) {
            sum += j;
        }
    }
    
    /* Another loop that shares the shared_block */
    i = 0;
    while (i < n/2 && force_loop) {
        sum += i * 4;
        
        if (i % 2 == 0) {
            goto shared_block2;
        }
        i++;
        continue;
        
    shared_block2:
        /* Different shared block */
        sum += i * 5;
        i++;
    }
    
    if (sum == 0) {
        asm volatile("" ::: "memory");
    }
}

/* Pattern F: Do-while loops with different structures */
__attribute__((noinline))
void mixed_loop_types(int n) {
    int sum = 0;
    
    /* do-while loop */
    int i = 0;
    do {
        sum += i;
        
        /* while loop inside */
        int j = 0;
        while (j < 5 && force_loop) {
            sum += j;
            j++;
        }
        
        i++;
    } while (i < n && force_loop);
    
    /* for loop with break */
    for (int k = 0; k < n * 2 && force_loop; k++) {
        sum += k;
        if (k == n) {
            break;
        }
        
        /* Another nested loop */
        for (int l = 0; l < 3 && force_loop; l++) {
            sum -= l;
        }
    }
    
    if (sum == 0) {
        asm volatile("" ::: "memory");
    }
}

/* Main function to call all patterns */
int main() {
    /* Use compile-time constants for predictable loop bounds */
    const int N = 100;
    const int M = 50;
    const int P = 20;
    
    /* Call each pattern multiple times with different parameters */
    perfect_nesting(N, M);
    partial_overlap(N, M);
    sequential_loops(N);
    complex_nesting(N/2, M/2, P);
    irregular_loops(N);
    mixed_loop_types(N/4);
    
    /* Also call with different sizes to create different CFGs */
    perfect_nesting(10, 5);
    partial_overlap(20, 10);
    sequential_loops(15);
    
    return 0;
}
