/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop having multiple blocks */
NOINLINE int test_nested_simple(int n, int m) {
    volatile int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with if statement creating multiple blocks */
        for (int j = 0; j < m; ++j) {
            if (j % 3 == 0) {
                sum += i * 2;  /* Block A1 */
                continue;      /* Creates separate block for continue */
            } else if (j % 3 == 1) {
                sum += j * 3;  /* Block A2 */
                /* No continue/break - falls through */
            } else {
                sum += i + j;  /* Block A3 */
                if (sum > 1000) {
                    break;     /* Block A4 with break */
                }
            }
            /* Additional block after if-else chain */
            sum += 1;
        }
        /* Block after inner loop */
        sum -= 5;
    }
    return sum;
}

/* Function B: Nested loops with shared header/complex control flow */
NOINLINE int test_nested_complex(int n, int m) {
    volatile int sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared block before inner loop */
        if (i % 2 == 0) {
            sum += 10;  /* Block B1 */
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            /* Multiple basic blocks in inner loop */
            switch (j % 4) {
                case 0:
                    sum += i;      /* Block B2 */
                    break;
                case 1:
                    sum += j;      /* Block B3 */
                    if (sum > 500) {
                        goto inner_exit;  /* Block B4 with goto */
                    }
                    break;
                case 2:
                    sum += i * j;  /* Block B5 */
                    break;
                default:
                    sum -= 1;      /* Block B6 */
                    break;
            }
            /* Block after switch */
            sum += 2;
        }
    inner_exit:
        /* Block after inner loop (could be shared) */
        sum += 3;
        i++;
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n, int m) {
    volatile int sum = 0;
    int arr1[100], arr2[100];
    
    /* Initialize arrays to prevent optimization */
    for (int k = 0; k < 100; ++k) {
        arr1[k] = k;
        arr2[k] = 100 - k;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            sum += arr1[i % 100] * 2;  /* Block C1 */
        } else {
            sum += arr1[i % 100] / 2;  /* Block C2 */
            if (sum < 0) {
                return sum;  /* Block C3 with early return */
            }
        }
        /* Block after if-else */
        sum += 1;
    }
    
    /* Intermediate code to ensure disjointness */
    int temp = sum * 2;
    
    /* Second loop - completely disjoint blocks */
    for (int j = 0; j < m; ++j) {
        switch (j % 3) {
            case 0:
                sum += arr2[j % 100];      /* Block C4 */
                break;
            case 1:
                sum -= arr2[j % 100] * 3;  /* Block C5 */
                break;
            case 2:
                sum += temp;               /* Block C6 */
                break;
        }
        /* Block after switch */
        sum += 5;
    }
    
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    volatile int sum = 0;
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < outer_iter; ++outer) {
        /* Inner loop with switch */
        for (int i = 0; i < n; ++i) {
            /* Switch creates multiple basic blocks */
            switch (i % 5) {
                case 0:
                    sum += outer * 10;     /* Block D1 */
                    if (sum > 10000) {
                        goto switch_exit;  /* Block D2 with goto */
                    }
                    break;
                case 1:
                    sum += i * 20;         /* Block D3 */
                    break;
                case 2:
                    sum += outer + i;      /* Block D4 */
                    break;
                case 3:
                    sum -= 15;             /* Block D5 */
                    break;
                default:
                    sum += 7;              /* Block D6 */
                    /* Fall through to increment */
            }
            /* Block after switch (except when goto taken) */
            sum += 1;
        switch_exit:
            /* Shared exit block */
            sum += 2;
        }
        /* Block after inner loop */
        sum += outer * 3;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint loops in branches) */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    volatile int sum = 0;
    
    if (flag > 0) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            if (i % 4 == 0) {
                sum += i * 2;      /* Block E1 */
                continue;          /* Creates separate continue block */
            } else if (i % 4 == 1) {
                sum += i * 3;      /* Block E2 */
            } else {
                sum += i;          /* Block E3 */
            }
            /* Block after if-else chain */
            sum += 5;
        }
        /* Block after loop in true branch */
        sum += 100;
    } else {
        /* Different loop in false branch - disjoint blocks */
        int j = 0;
        while (j < m) {
            sum += j * j;          /* Block E4 */
            j++;
            if (sum > 2000) {
                break;             /* Block E5 with break */
            }
            /* Block after if */
            sum += 2;
        }
        /* Block after loop in false branch */
        sum += 200;
    }
    
    return sum;
}

/* Function F: Complex nested loops with multiple exits */
NOINLINE int test_complex_nesting(int n, int m) {
    volatile int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* First inner loop */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += i + j;      /* Block F1 */
                if (sum > 5000) {
                    goto outer_continue;  /* Exit to outer loop */
                }
            } else {
                sum += i * j;      /* Block F2 */
            }
            /* Block after if-else */
            sum += 1;
        }
        
        /* Second inner loop (nested in outer, after first) */
        for (int k = 0; k < i; ++k) {
            switch (k % 3) {
                case 0:
                    sum += k;      /* Block F3 */
                    break;
                case 1:
                    sum -= k;      /* Block F4 */
                    break;
                case 2:
                    sum += i - k;  /* Block F5 */
                    break;
            }
            /* Block after switch */
            sum += 3;
        }
        
    outer_continue:
        /* Block after inner loops */
        sum += 10;
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create some variability in loop bounds */
    int base_iter = (argc > 1) ? atoi(argv[1]) : 10;
    if (base_iter < 5) base_iter = 5;
    if (base_iter > 100) base_iter = 100;
    
    /* Test all patterns */
    result += test_nested_simple(base_iter, base_iter / 2);
    result += test_nested_complex(base_iter, base_iter / 3);
    result += test_sequential_disjoint(base_iter, base_iter / 2);
    result += test_switch_in_loop(base_iter, 2);
    result += test_conditional_loops(base_iter, base_iter / 2, argc);
    result += test_complex_nesting(base_iter, base_iter / 3);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
