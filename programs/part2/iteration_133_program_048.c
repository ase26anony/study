/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop having multiple basic blocks */
NOINLINE int test_nested_simple(int n, int m) {
    volatile int sum = 0;
    /* Outer loop - creates one set of blocks */
    for (int i = 0; i < n; ++i) {
        sum += i;
        /* Inner loop with multiple basic blocks due to if/continue */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += j * 2;
                continue;  /* Creates separate basic block for continue path */
            }
            sum += j;
            /* Additional block complexity */
            if (j % 3 == 0) {
                sum += 1;
            }
        }
        /* Block after inner loop */
        if (i % 2 == 0) {
            sum -= 1;
        }
    }
    return sum;
}

/* Function B: Nested loops with do-while outer and for inner, shared header complexity */
NOINLINE int test_nested_do_while(int n, int m) {
    volatile int sum = 0;
    int i = 0;
    /* do-while creates different loop structure */
    do {
        /* This block is part of outer loop only initially */
        if (i > n/2) {
            sum += 100;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            /* Multiple blocks in inner loop */
            switch (j % 3) {
                case 0: sum += j * 3; break;
                case 1: sum += j * 2; break;
                default: sum += j; break;
            }
            
            /* Early exit from inner loop creates another block */
            if (sum > 1000 && j > m/2) {
                break;
            }
        }
        
        i++;
        /* Conditional continue in outer loop */
        if (i % 4 == 0) {
            continue;
        }
        sum += i;
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
    
    /* First loop - completely disjoint from second */
    for (int i = 0; i < n && i < 100; ++i) {
        if (i % 2 == 0) {
            sum += arr1[i];
        } else {
            sum -= arr1[i];
        }
        /* Multiple exit points */
        if (sum < -1000) {
            return sum;  /* Early return creates exit block */
        }
    }
    
    /* Intermediate code to ensure disjointness */
    int temp = sum * 2;
    
    /* Second loop - disjoint from first */
    for (int j = 0; j < m && j < 100; ++j) {
        if (j % 3 == 0) {
            sum += arr2[j] * 2;
        } else if (j % 3 == 1) {
            sum += arr2[j];
        } else {
            sum -= arr2[j];
        }
    }
    
    return sum + temp;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    volatile int sum = 0;
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < outer_iter; ++outer) {
        sum += outer * 100;
        
        /* Inner loop with switch creating many basic blocks */
        for (int i = 0; i < n; ++i) {
            /* Switch with multiple cases - each creates a basic block */
            switch (i % 5) {
                case 0:
                    sum += i;
                    /* Nested if inside case */
                    if (i % 10 == 0) {
                        sum += 10;
                    }
                    break;
                case 1:
                    sum += i * 2;
                    break;
                case 2:
                    sum += i * 3;
                    /* Another conditional */
                    if (sum > 500) {
                        sum -= 50;
                    }
                    break;
                case 3:
                    sum += i * 4;
                    /* continue creates another block */
                    if (i % 7 == 0) {
                        continue;
                    }
                    break;
                default:  /* case 4 */
                    sum += i * 5;
                    /* break from switch only */
                    break;
            }
            
            /* Block after switch */
            if (i % 6 == 0) {
                sum += 1;
            }
        }
        
        /* Block between inner loop iterations */
        if (outer % 2 == 0) {
            sum += 999;
        }
    }
    
    return sum;
}

/* Function E: Conditional loop nesting - two disjoint loops in different branches */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    volatile int sum = 0;
    
    if (flag) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            sum += i * i;
            /* Multiple basic blocks */
            if (i % 3 == 0) {
                sum += 3;
                continue;
            }
            if (i % 4 == 0) {
                sum += 4;
            }
            /* Early exit */
            if (sum > 10000) {
                break;
            }
        }
        
        /* Additional code in true branch */
        for (int j = 0; j < 10; ++j) {
            sum += j;
        }
    } else {
        /* Different loop in false branch - disjoint from true branch loops */
        int k = 0;
        while (k < m) {
            sum -= k * k;
            k++;
            /* Nested if for block complexity */
            if (k % 5 == 0) {
                sum += 50;
                if (k % 10 == 0) {
                    sum += 100;
                }
            }
        }
        
        /* Another loop in false branch */
        for (int p = 0; p < 5; ++p) {
            sum += p * 100;
        }
    }
    
    /* Common code after if-else */
    for (int q = 0; q < 3; ++q) {
        sum += q;
    }
    
    return sum;
}

/* Function F: Complex nested loops with multiple levels */
NOINLINE int test_multi_level_nested(int n, int m, int p) {
    volatile int sum = 0;
    
    /* Level 1: Outermost loop */
    for (int i = 0; i < n; ++i) {
        sum += i;
        
        /* Level 2: Middle loop */
        for (int j = 0; j < m; ++j) {
            sum += j * 10;
            
            /* Level 3: Innermost loop */
            for (int k = 0; k < p; ++k) {
                sum += k * 100;
                
                /* Multiple blocks in innermost loop */
                if (k % 2 == 0) {
                    sum += 1;
                    if (k % 4 == 0) {
                        sum += 2;
                        continue;  /* Skips to next k iteration */
                    }
                } else {
                    sum -= 1;
                }
                
                /* Another conditional */
                if (sum > 5000) {
                    /* Break from innermost only */
                    break;
                }
            }
            
            /* Block after innermost loop */
            if (j % 3 == 0) {
                sum += 1000;
            }
        }
        
        /* Early exit from outer loop */
        if (sum > 100000) {
            return sum;
        }
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create some runtime variability in loop bounds */
    int base = (argc > 1) ? atoi(argv[1]) : 10;
    if (base <= 0) base = 10;
    
    /* Test 1: Simple nested loops */
    result += test_nested_simple(base, base + 2);
    
    /* Test 2: Do-while nested loops */
    result += test_nested_do_while(base + 1, base + 3);
    
    /* Test 3: Sequential disjoint loops */
    result += test_sequential_disjoint(base + 2, base + 4);
    
    /* Test 4: Loop with switch inside, wrapped */
    result += test_switch_in_loop(base + 3, 2);
    
    /* Test 5: Conditional loops (alternate branches) */
    result += test_conditional_loops(base + 4, base + 5, argc % 2);
    
    /* Test 6: Multi-level nested loops */
    result += test_multi_level_nested(base % 5 + 3, base % 7 + 2, base % 3 + 2);
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
