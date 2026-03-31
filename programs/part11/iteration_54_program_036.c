/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of loops */
static volatile int g_counter __attribute__((used)) = 0;

/* Pattern A: Perfectly nested loops - inner loop is subset of outer loop */
__attribute__((noinline))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Split basic block to create more complex bitmap */
        if (__builtin_expect(g_counter != 0, 0)) {
            /* This branch is rarely taken but creates separate block */
            sum += g_counter;
        }
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            sum += i * j;
            /* Early exit to affect block structure */
            if (j == m/2 && i == n/2) {
                break;
            }
        }
        
        /* Another split in outer loop */
        if (__builtin_expect(sum > 1000, 0)) {
            sum -= 1000;
        }
    }
    
    g_counter += sum;
}

/* Pattern B: Partially overlapping loops with shared blocks */
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
        
        /* Conditional inner loop - creates partial overlap */
        if (i % 3 == 0) {
            /* This creates a loop that shares some blocks but not all */
            for (j = 0; j < m/2; j++) {
                sum += j;
                if (j == m/4) {
                    /* Early exit creates different block structure */
                    break;
                }
            }
        }
        
        /* Split block */
        if (__builtin_expect(shared > 100, 0)) {
            sum += shared % 10;
        }
    } while (i < n);
    
    /* Second loop that overlaps with first loop's blocks */
    for (j = 0; j < m; j++) {
        /* This block may overlap with blocks from first loop */
        sum += j * 2;
        
        /* Create irregular control flow with goto */
        if (j == m/2) {
            goto skip_part;
        }
        sum += 1;
    skip_part:
        /* Continue with loop */
        ;
    }
    
    g_counter += sum;
}

/* Pattern C: Sibling loops with shared preheader */
__attribute__((noinline))
void sibling_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared preheader block */
    int setup = n + m;
    
    /* First sibling loop */
    i = 0;
    while (i < n) {
        sum += i * setup;
        i++;
        
        /* Split block inside loop */
        if (__builtin_expect(i % 5 == 0, 0)) {
            sum -= setup;
        }
    }
    
    /* Code between loops (still part of shared region) */
    sum += setup * 2;
    
    /* Second sibling loop */
    for (j = 0; j < m; j++) {
        sum += j * setup;
        
        /* Different internal structure */
        if (j % 3 == 0) {
            sum += 100;
            if (j % 6 == 0) {
                sum += 50;
            }
        }
    }
    
    g_counter += sum;
}

/* Pattern D: Complex nested loops with multiple exits */
__attribute__((noinline, target("thumb")))
void complex_nesting(int n, int m, int k) {
    int i, j, l;
    int sum = 0;
    
    /* Triple nested loops */
    for (i = 0; i < n; i++) {
        /* Middle loop with early exit */
        j = 0;
        while (j < m) {
            sum += i * j;
            
            /* Innermost loop */
            for (l = 0; l < k; l++) {
                sum += l;
                
                /* Multiple exit points */
                if (l == k/2 && i == n/2) {
                    goto exit_inner;
                }
                if (sum > 10000) {
                    break;
                }
            }
        exit_inner:
            
            j++;
            if (j == m/2) {
                /* Another exit point */
                break;
            }
        }
        
        /* Additional block in outer loop */
        if (__builtin_expect(i % 7 == 0, 1)) {
            sum += 777;
        }
    }
    
    g_counter += sum;
}

/* Pattern E: Loops with switch statements inside */
__attribute__((noinline))
void loops_with_switch(int n) {
    int i;
    int sum = 0;
    
    for (i = 0; i < n; i++) {
        /* Switch creates multiple basic blocks within loop */
        switch (i % 4) {
            case 0:
                sum += i;
                /* Fall through */
            case 1:
                sum += i * 2;
                break;
            case 2:
                sum += i * 3;
                /* Nested conditional */
                if (i % 8 == 0) {
                    sum += 8;
                }
                break;
            case 3:
                sum += i * 4;
                /* Small inner loop */
                {
                    int j = 0;
                    while (j < 3) {
                        sum += j;
                        j++;
                    }
                }
                break;
        }
    }
    
    g_counter += sum;
}

/* Pattern F: Irregular loop structure with gotos */
__attribute__((noinline))
void irregular_loops(int n) {
    int i = 0;
    int sum = 0;
    
start_loop1:
    if (i >= n) goto end_loop1;
    
    sum += i;
    i++;
    
    /* Conditional jump to create overlapping block structure */
    if (i % 2 == 0) {
        goto mid_loop;
    }
    
    /* Different path */
    sum += 100;
    
mid_loop:
    /* This block is shared between paths */
    sum += i * 2;
    
    /* Nested irregular loop */
    {
        int j = 0;
    loop2:
        if (j >= 5) goto end_loop2;
        sum += j;
        j++;
        
        /* Jump back creates loop */
        goto loop2;
    end_loop2:
        ;
    }
    
    goto start_loop1;
end_loop1:
    
    g_counter += sum;
}

/* Main function to ensure all functions are called */
int main(void) {
    int iterations = 100;
    
    /* Call each pattern with different parameters */
    perfect_nesting(iterations, 50);
    partial_overlap(iterations, 40);
    sibling_loops(iterations, 30);
    complex_nesting(20, 15, 10);
    loops_with_switch(iterations);
    irregular_loops(iterations);
    
    /* Ensure computation isn't optimized away */
    if (g_counter > 0) {
        return 0;
    }
    return 1;
}
