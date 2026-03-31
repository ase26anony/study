/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of loops */
static volatile int g_counter __attribute__((used));

/* Pattern A: Perfectly nested loops - inner loop bitmap is subset of outer */
__attribute__((target("thumb")))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    int arr[100];
    
    /* Initialize array to prevent removal */
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Outer loop - will contain inner loop's blocks */
    for (i = 0; i < n; i++) {
        g_counter++; /* Force side effect */
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            /* Split basic block inside inner loop */
            if (__builtin_expect(arr[j] > 0, 1)) {
                sum += arr[j] * i;
            } else {
                sum += 1;
            }
        }
        
        /* Additional block in outer loop only */
        if (i % 2 == 0) {
            sum -= 1;
        }
    }
    
    g_counter = sum;
}

/* Pattern B: Partially overlapping loops with shared blocks but not subset */
__attribute__((target("thumb")))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    int arr[100];
    
    /* Initialize */
    for (i = 0; i < 100; i++) {
        arr[i] = i % 10;
    }
    
    i = 0;
    
    /* First loop with early exit */
    while (i < n) {
        /* Shared block - both loops will include this */
        if (__builtin_expect(arr[i] > 5, 0)) {
            sum += 10;
        }
        
        /* Jump to create irregular control flow */
        if (i == n/2) {
            /* Start second loop from within first */
            j = 0;
            do {
                /* Block only in second loop */
                sum += arr[j] * 2;
                j++;
                
                /* Conditional continue to create more blocks */
                if (j % 3 == 0) {
                    continue;
                }
                
                sum += 1;
            } while (j < m && j < 10);
            
            /* Early exit from first loop */
            break;
        }
        
        sum += arr[i];
        i++;
    }
    
    g_counter = sum;
}

/* Pattern C: Sequential loops with shared preheader block */
__attribute__((target("thumb")))
void sequential_loops(int n, int m) {
    int i, j;
    int sum = 0;
    int arr[100];
    
    /* SHARED BLOCK - will be in both loops' bitmaps */
    /* Setup that can't be optimized away */
    for (i = 0; i < 100; i++) {
        arr[i] = (i * 3) % 7;
    }
    
    /* First loop */
    i = 0;
    do {
        /* Block only in first loop */
        if (arr[i] == 0) {
            sum += 100;
            /* Early continue creates separate block */
            continue;
        }
        
        sum += arr[i] * 2;
        i++;
    } while (i < n);
    
    /* Second loop - sequential but shares setup blocks */
    for (j = 0; j < m; j++) {
        /* Different computation to prevent fusion */
        sum -= arr[j] / 2;
        
        /* Nested conditional to split blocks */
        if (__builtin_expect(j % 4 == 0, 1)) {
            sum += j * 3;
        }
    }
    
    g_counter = sum;
}

/* Pattern D: Complex nested loops with multiple levels */
__attribute__((target("thumb")))
void multi_level_nesting(int n, int m, int k) {
    int i, j, l;
    int sum = 0;
    int arr[50][50];
    
    /* Initialize 2D array */
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            arr[i][j] = (i + j) % 11;
        }
    }
    
    /* Level 1: Outer loop */
    for (i = 0; i < n; i++) {
        /* Level 2: Middle loop */
        for (j = 0; j < m; j++) {
            /* Split block in middle loop */
            if (arr[i][j] > 5) {
                sum += 5;
            }
            
            /* Level 3: Innermost loop */
            for (l = 0; l < k && l < 10; l++) {
                /* Innermost block - only in level 3 */
                sum += arr[i][j] * l;
                
                /* Conditional break creates exit block */
                if (sum > 1000) {
                    break;
                }
            }
            
            /* Block only in level 2 */
            sum += j;
        }
        
        /* Block only in level 1 */
        if (i % 3 == 0) {
            sum -= 2;
        }
    }
    
    g_counter = sum;
}

/* Pattern E: Loops with goto creating irregular overlap */
__attribute__((target("thumb")))
void irregular_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    int arr[100];
    
    /* Initialize */
    for (i = 0; i < 100; i++) {
        arr[i] = i % 13;
    }
    
    i = 0;
    
    /* Loop with goto that jumps into another loop's body */
    while (i < n) {
        sum += arr[i];
        
        if (sum % 7 == 0) {
            /* Jump to middle of next loop */
            j = m / 2;
            goto middle_of_loop;
        }
        
        i++;
        
        if (i >= n) {
            break;
        }
        
        /* Start of second loop structure */
        for (j = 0; j < m; j++) {
            middle_of_loop:
            /* This block will be in both loop structures */
            sum += arr[j] * 3;
            
            if (j % 5 == 0) {
                /* Jump back to first loop */
                goto continue_first;
            }
        }
        
        continue_first:
        if (i < n) {
            sum += 1;
        }
    }
    
    g_counter = sum;
}

/* Pattern F: Switch statement inside loop creating multiple blocks */
__attribute__((target("thumb")))
void switch_in_loops(int n) {
    int i;
    int sum = 0;
    
    for (i = 0; i < n; i++) {
        /* Switch creates multiple basic blocks within loop */
        switch (i % 4) {
            case 0:
                sum += i * 2;
                /* Fall through */
            case 1:
                sum += 5;
                break;
            case 2:
                sum += i / 2;
                /* Nested if to create more blocks */
                if (__builtin_expect(i > n/2, 0)) {
                    sum += 10;
                }
                break;
            case 3:
                sum -= 3;
                break;
            default:
                sum += 1;
        }
        
        /* Another loop inside case blocks */
        if (i % 3 == 0) {
            int j;
            for (j = 0; j < 3; j++) {
                sum += j;
            }
        }
    }
    
    g_counter = sum;
}

/* Main function to call all patterns */
int main(void) {
    int iterations = 10;
    
    /* Call each function multiple times with different parameters */
    perfect_nesting(iterations, iterations/2);
    partial_overlap(iterations, iterations);
    sequential_loops(iterations, iterations*2);
    multi_level_nesting(iterations/2, iterations/3, iterations/4);
    irregular_overlap(iterations, iterations/2);
    switch_in_loops(iterations);
    
    /* Ensure functions aren't optimized away */
    if (g_counter > 1000) {
        return 0;
    }
    
    return 1;
}
