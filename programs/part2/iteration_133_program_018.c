/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop having multiple blocks */
NOINLINE int test_nested_simple(int n, int m) {
    volatile int sum = 0;  /* volatile to prevent optimization */
    int i, j;
    
    /* Outer loop */
    for (i = 0; i < n; ++i) {
        sum += i;
        
        /* Inner loop with multiple basic blocks */
        for (j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += j * 2;
                continue;  /* Creates separate block for continue path */
            } else {
                sum += j;
                if (j == m - 1) {
                    break;  /* Creates separate block for break */
                }
            }
        }
    }
    
    return sum;
}

/* Function B: Nested loops with shared header complexity */
NOINLINE int test_nested_shared_header(int n, int m) {
    volatile int sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared block before inner loop */
        if (i % 3 == 0) {
            sum += 100;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            sum += i * j;
            if (j % 4 == 0) {
                continue;  /* Multiple blocks in inner loop */
            }
            sum -= 1;
        }
        
        i++;
        if (i > n) {
            break;  /* Multiple exit points from outer loop */
        }
    } while (1);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n, int m) {
    volatile int sum = 0;
    int i;
    
    /* First loop with multiple blocks */
    for (i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            sum += i * 2;
        } else {
            sum += i;
            if (i == n - 1) {
                break;  /* Creates separate block */
            }
        }
    }
    
    /* Second loop - completely disjoint from first */
    for (i = 0; i < m; ++i) {
        if (i % 3 == 0) {
            sum -= i;
            continue;
        }
        sum += i * 3;
    }
    
    return sum;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    volatile int sum = 0;
    int k;
    
    /* Outer wrapper loop */
    for (k = 0; k < outer_iter; ++k) {
        /* Inner loop with switch */
        for (int i = 0; i < n; ++i) {
            switch (i % 5) {
                case 0:
                    sum += i;
                    break;
                case 1:
                    sum += i * 2;
                    if (i == n - 1) {
                        goto end_inner;  /* Multiple exit points */
                    }
                    break;
                case 2:
                    sum += i * 3;
                    continue;  /* Continue to next iteration */
                case 3:
                    sum += i * 4;
                    break;
                default:  /* case 4 */
                    sum += i * 5;
                    if (sum > 1000) {
                        return sum;  /* Early return creates exit block */
                    }
                    break;
            }
            sum += 1;  /* Common block after switch */
        }
        end_inner:
        sum += 1000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting - disjoint loops in branches */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    volatile int sum = 0;
    
    if (flag) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            sum += i;
            if (i % 2 == 0) {
                continue;
            }
            sum *= 2;
        }
    } else {
        /* Different loop in false branch - disjoint from true branch loop */
        int j = m;
        while (j-- > 0) {
            sum -= j;
            if (j % 3 == 0) {
                break;  /* Multiple exit points */
            }
            sum += 100;
        }
    }
    
    return sum;
}

/* Function F: Complex nested loops with multiple levels */
NOINLINE int test_multi_level_nested(int n, int m, int p) {
    volatile int sum = 0;
    
    /* Level 1 outer loop */
    for (int i = 0; i < n; ++i) {
        sum += i;
        
        /* Level 2 middle loop */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += i * j;
                continue;
            }
            
            /* Level 3 inner loop */
            for (int k = 0; k < p; ++k) {
                sum += k;
                if (k % 3 == 0) {
                    sum -= 1;
                    if (k == p - 1) {
                        goto next_j;  /* Jump to middle loop */
                    }
                }
            }
            next_j:
            sum += 10;
        }
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc for variability in loop bounds */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    int m = (argc > 2) ? atoi(argv[2]) : 50;
    int p = (argc > 3) ? atoi(argv[3]) : 25;
    
    /* Force loop bounds into registers to prevent constant propagation */
    asm volatile("" : : "r"(n), "r"(m), "r"(p));
    
    printf("Testing hardware loop optimization patterns...\n");
    
    result += test_nested_simple(n, m);
    printf("test_nested_simple: %d\n", result);
    
    result += test_nested_shared_header(n / 2, m / 2);
    printf("test_nested_shared_header: %d\n", result);
    
    result += test_sequential_disjoint(n, m);
    printf("test_sequential_disjoint: %d\n", result);
    
    result += test_switch_in_loop(n, 2);
    printf("test_switch_in_loop: %d\n", result);
    
    result += test_conditional_loops(n, m, argc % 2);
    printf("test_conditional_loops: %d\n", result);
    
    result += test_multi_level_nested(n / 4, m / 4, p);
    printf("test_multi_level_nested: %d\n", result);
    
    printf("Total result: %d\n", result);
    
    return result != 0 ? 0 : 1;  /* Return non-zero for success */
}
