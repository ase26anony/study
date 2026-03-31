/* test_hw_doloop.c - Test program to cover hw-doloop.cc lines 429-436 */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

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

/* Pattern B: Partially overlapping loops - share some blocks but not subsets */
__attribute__((noinline))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* First loop with complex control flow */
    do {
        sum += i;
        i++;
        
        /* Shared block that both loops will include */
        if (__builtin_expect(sum < 100, 1)) {
            /* This block belongs to both loops */
            sink = sum;
            
            /* Start second loop conditionally */
            if (i > n/2) {
                /* Second loop that overlaps but isn't nested */
                while (j < m) {
                    sum += j;
                    j++;
                    
                    /* Break back to first loop */
                    if (j > m/2) {
                        goto continue_first;
                    }
                }
            }
        }
        
    continue_first:
        /* More code in first loop */
        if (i & 1) {
            sum *= 2;
        }
    } while (i < n);
    
    sink = sum;
}

/* Pattern C: Sequential loops with shared preheader/setup block */
__attribute__((noinline))
void sequential_shared(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared setup block - may be included in both loop bitmaps */
    int shared_counter = n + m;
    sink = shared_counter;
    
    /* First loop */
    i = 0;
    while (i < n) {
        sum += i;
        i++;
        
        /* Split block */
        if (__builtin_expect(i == n/2, 0)) {
            sink = i;
        }
    }
    
    /* Re-use shared setup (compiler might merge) */
    shared_counter--;
    sink = shared_counter;
    
    /* Second loop - sequential but shares setup block */
    for (j = 0; j < m; j++) {
        sum -= j;
        
        /* Different internal structure */
        switch (j % 3) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 2: sum += 3; break;
        }
    }
    
    sink = sum;
}

/* Pattern D: Complex nested structure with multiple levels */
__attribute__((noinline))
void multi_level_nesting(int n, int m, int k) {
    int a, b, c;
    int sum = 0;
    
    /* Level 1 */
    for (a = 0; a < n; a++) {
        /* Split block */
        if (a % 2) {
            sink = a;
        }
        
        /* Level 2 */
        b = 0;
        while (b < m) {
            sum += a * b;
            b++;
            
            /* Level 3 - innermost */
            for (c = 0; c < k; c++) {
                sum += c;
                
                /* Conditional continue affects bitmap */
                if (c & 1) {
                    continue;
                }
                sum -= 1;
            }
            
            /* Early exit from level 2 */
            if (b > m/2 && (a & 1)) {
                break;
            }
        }
        
        /* Another split at level 1 */
        if (__builtin_expect(sum > 10000, 0)) {
            break;
        }
    }
    
    sink = sum;
}

/* Pattern E: Loops with goto creating irregular overlap */
__attribute__((noinline))
void irregular_overlap(int n) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Loop 1 */
    for (i = 0; i < n; i++) {
        sum += i;
        
        /* Jump into another loop structure */
        if (i == n/3) {
            goto middle_of_loop2;
        }
        
        if (i == n/2) {
            /* Loop 2 entry */
            j = 0;
    loop2_start:
            while (j < n/2) {
                sum += j * 2;
                j++;
                
    middle_of_loop2:
                /* Shared code between loops */
                if (sum & 1) {
                    sum--;
                }
                
                if (j < n/4) {
                    goto loop2_start;
                }
            }
        }
    }
    
    sink = sum;
}

/* Pattern F: Do-while and for loops mixed */
__attribute__((noinline))
void mixed_loop_types(int n) {
    int i, j, k;
    int sum = 0;
    
    /* do-while loop */
    i = 0;
    do {
        sum += i;
        i++;
        
        /* Nested for loop */
        for (j = 0; j < 5; j++) {
            sum += j;
            
            /* Another nested while */
            k = 0;
            while (k < 3) {
                sum -= k;
                k++;
                
                if (k == 2) {
                    /* Early continue affects block structure */
                    continue;
                }
                sum += 1;
            }
        }
    } while (i < n);
    
    sink = sum;
}

/* Main function to call all patterns */
int main(void) {
    int iterations = 100;
    
    /* Call each pattern multiple times with different parameters */
    perfect_nesting(iterations, 50);
    partial_overlap(iterations, 50);
    sequential_shared(iterations, 50);
    multi_level_nesting(10, 20, 30);
    irregular_overlap(iterations);
    mixed_loop_types(iterations);
    
    /* Ensure functions aren't optimized away */
    __attribute__((used)) static void (*funcs[])(void) = {
        (void (*)(void))perfect_nesting,
        (void (*)(void))partial_overlap,
        (void (*)(void))sequential_shared,
        (void (*)(void))multi_level_nesting,
        (void (*)(void))irregular_overlap,
        (void (*)(void))mixed_loop_types
    };
    
    return 0;
}
