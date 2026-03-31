/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop having multiple basic blocks */
NOINLINE int test_nested_simple(int n, int m) {
    volatile int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with multiple basic blocks due to if-continue */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                /* Creates separate basic block */
                sum += i * 2;
                continue;
            }
            /* Another basic block */
            sum += j;
            
            /* Additional complexity with break */
            if (j > 10) {
                break;
            }
        }
        
        /* Another if to create more blocks in outer loop */
        if (i % 3 == 0) {
            sum -= 1;
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
        /* Block that could be shared in intersection analysis */
        int temp = i * 2;
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            /* Multiple basic blocks in inner loop */
            if (j < temp) {
                sum += j;
                if (j == 5) {
                    /* Early exit creates another block */
                    goto inner_exit;
                }
            } else {
                sum -= 1;
            }
        }
        inner_exit:
        
        /* Another block in outer loop */
        if (sum > 1000) {
            break;
        }
        
        i++;
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n, int m) {
    volatile int sum = 0;
    int arr1[100], arr2[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            sum += arr1[i % 100];
        } else {
            sum -= arr1[i % 100];
        }
        
        /* Early return creates exit block */
        if (sum > 10000) {
            return sum;
        }
    }
    
    /* Second loop - completely disjoint blocks */
    for (int j = 0; j < m; ++j) {
        if (j % 3 == 0) {
            sum += arr2[j % 100] * 2;
        } else if (j % 3 == 1) {
            sum += arr2[j % 100];
        } else {
            /* Empty else creates another block */
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE int test_loop_with_switch(int n, int outer_iter) {
    volatile int sum = 0;
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < outer_iter; ++outer) {
        /* Inner loop with switch */
        for (int i = 0; i < n; ++i) {
            /* Switch creates multiple basic blocks */
            switch (i % 5) {
                case 0:
                    sum += i;
                    break;
                case 1:
                    sum += i * 2;
                    /* Fall through */
                case 2:
                    sum += i * 3;
                    break;
                case 3:
                    sum -= i;
                    /* Complex block with nested if */
                    if (sum < 0) {
                        sum = 0;
                    }
                    break;
                case 4:
                    sum += 100;
                    /* Early continue */
                    continue;
                default:
                    sum += 1;
            }
            
            /* Additional block after switch */
            if (i == n - 1) {
                sum += 1000;
            }
        }
        
        /* Block in outer loop but not in inner */
        sum += outer * 10000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint loops) */
NOINLINE int test_conditional_loops(int n, int m, int condition) {
    volatile int sum = 0;
    
    if (condition) {
        /* First loop in true branch */
        for (int i = 0; i < n; ++i) {
            sum += i * i;
            /* Multiple exit points */
            if (i > 50) {
                return sum;
            }
            if (i % 7 == 0) {
                sum += 7;
                continue;
            }
        }
        
        /* Additional code in true branch */
        sum += 100;
    } else {
        /* Second loop in false branch - disjoint from first */
        for (int j = 0; j < m; ++j) {
            sum -= j * 3;
            /* Different control flow */
            switch (j % 4) {
                case 0: sum += 1; break;
                case 1: sum += 2; break;
                case 2: sum += 3; break;
                case 3: sum += 4; break;
            }
        }
        
        /* Additional code in false branch */
        sum -= 200;
    }
    
    return sum;
}

/* Function F: Complex nested loops with multiple levels */
NOINLINE int test_complex_nesting(int n, int m, int p) {
    volatile int sum = 0;
    
    /* Level 1 */
    for (int i = 0; i < n; ++i) {
        /* Level 2 */
        for (int j = 0; j < m; ++j) {
            /* Level 3 */
            for (int k = 0; k < p; ++k) {
                /* Complex body with multiple blocks */
                if (k % 2 == 0) {
                    sum += i + j + k;
                    if (sum > 1000) {
                        goto level2_continue;
                    }
                } else {
                    sum -= 1;
                }
                
                /* Another block */
                if (k == p - 1) {
                    sum += 100;
                }
            }
            level2_continue:
            
            /* Block in level 2 but not in level 3 */
            if (j % 3 == 0) {
                sum += 10;
            }
        }
        
        /* Block in level 1 but not in level 2 or 3 */
        if (i % 4 == 0) {
            sum += 1000;
        }
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use argc to create some variability in loop bounds */
    int base = (argc > 1) ? atoi(argv[1]) : 10;
    if (base <= 0) base = 10;
    
    /* Test all patterns */
    total += test_nested_simple(base, base * 2);
    total += test_nested_shared_header(base, base + 5);
    total += test_sequential_disjoint(base, base * 3);
    total += test_loop_with_switch(base, 2);
    total += test_conditional_loops(base, base + 2, argc % 2);
    total += test_complex_nesting(base / 2 + 1, base / 3 + 1, base / 4 + 1);
    
    /* Prevent dead code elimination */
    volatile int result = total;
    
    /* Output to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
