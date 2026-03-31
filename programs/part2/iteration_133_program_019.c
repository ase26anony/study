/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Force variable to stay in register */
#define KEEP(var) asm volatile("" : : "r"(var))

/* Function A: Simple nested loops with inner loop having multiple basic blocks */
NOINLINE int test_nested_simple(int n, int m) {
    int sum = 0;
    KEEP(n);
    KEEP(m);
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with multiple basic blocks due to if/continue */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                /* Creates separate basic block */
                sum += i * j;
                continue;  /* Creates another edge */
            } else {
                /* Another basic block */
                sum += i + j;
            }
            /* Common tail block */
            if (j == m - 1) {
                sum += 1;  /* Extra block in inner loop */
            }
        }
        /* Outer loop body continues */
        if (i % 3 == 0) {
            sum -= 1;  /* Another block in outer loop */
        }
    }
    return sum;
}

/* Function B: Nested loops with shared header/complex control flow */
NOINLINE int test_nested_complex(int n, int m) {
    int sum = 0;
    int i = 0;
    KEEP(n);
    KEEP(m);
    
    /* do-while outer loop */
    do {
        /* Block that could be shared in bitmap analysis */
        int temp = i * 2;
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            /* Multiple exit points from inner loop */
            if (j > 10 && sum > 1000) {
                break;  /* Creates exit block */
            }
            if (j == 5) {
                sum += temp * j;
                continue;  /* Creates continue block */
            }
            sum += j;
        }
        
        /* Outer loop may skip inner sometimes */
        if (i % 4 == 0) {
            /* Skip inner loop entirely */
            sum += 100;
            continue;
        }
        
        i++;
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n, int m) {
    int sum = 0;
    int arr1[100], arr2[100];
    KEEP(n);
    KEEP(m);
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = 100 - i;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n && i < 100; ++i) {
        if (arr1[i] % 2 == 0) {
            sum += arr1[i] * 2;  /* Basic block A */
        } else {
            sum += arr1[i] / 2;  /* Basic block B */
        }
        /* Early exit possibility */
        if (sum > 10000) {
            return sum;  /* Creates exit block */
        }
    }
    
    /* Second loop - completely disjoint, no block intersection */
    for (int j = 0; j < m && j < 100; ++j) {
        if (arr2[j] > 50) {
            sum -= arr2[j];  /* Basic block C */
        } else {
            sum += arr2[j];  /* Basic block D */
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    int sum = 0;
    KEEP(n);
    KEEP(outer_iter);
    
    /* Outer wrapper loop */
    for (int k = 0; k < outer_iter; ++k) {
        /* Inner loop with switch creating many basic blocks */
        for (int i = 0; i < n; ++i) {
            switch (i % 5) {
                case 0:
                    sum += i * 1;
                    break;
                case 1:
                    sum += i * 2;
                    /* Fall through */
                case 2:
                    sum += i * 3;
                    break;
                case 3:
                    sum += i * 4;
                    /* Multiple statements in case */
                    if (sum % 2 == 0) {
                        sum += 10;
                    }
                    break;
                case 4:
                    sum += i * 5;
                    /* Nested if in case */
                    if (i > n/2) {
                        sum += 100;
                    } else {
                        sum += 50;
                    }
                    break;
                default:
                    sum += i;
            }
            
            /* Additional block after switch */
            if (i == n - 1) {
                sum += 1000;
            }
        }
        
        /* Outer loop body after inner */
        sum += k * 10000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint control flow paths) */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    int sum = 0;
    KEEP(n);
    KEEP(m);
    KEEP(flag);
    
    if (flag > 0) {
        /* First loop in true branch */
        for (int i = 0; i < n; ++i) {
            sum += i * i;
            /* Multiple blocks in loop */
            if (i % 3 == 0) {
                sum += 1;
                continue;
            }
            if (i % 7 == 0) {
                break;  /* Early exit */
            }
        }
        /* Extra code after loop in true branch */
        sum += 100;
    } else {
        /* Different loop in false branch - disjoint from first */
        for (int j = 0; j < m; ++j) {
            sum -= j * j;
            /* Different block structure */
            if (j % 4 == 0) {
                sum += 5;
                if (j > 10) {
                    sum += 10;
                }
            }
        }
        /* Extra code after loop in false branch */
        sum -= 100;
    }
    
    /* Common code after if-else */
    for (int k = 0; k < 5; ++k) {
        sum += k;  /* Small common loop */
    }
    
    return sum;
}

/* Function F: Deeply nested loops for complex hierarchy */
NOINLINE int test_deep_nesting(int n, int m, int p) {
    int sum = 0;
    KEEP(n);
    KEEP(m);
    KEEP(p);
    
    /* Level 1 */
    for (int i = 0; i < n; ++i) {
        sum += i;
        
        /* Level 2 */
        for (int j = 0; j < m; ++j) {
            sum += j;
            
            /* Level 3 */
            for (int k = 0; k < p; ++k) {
                sum += k;
                
                /* Innermost with multiple exits */
                if (k % 2 == 0) {
                    sum += 1;
                    if (k > p/2) {
                        break;  /* Break from innermost */
                    }
                }
            }
            
            /* Middle loop continuation */
            if (j % 3 == 0) {
                continue;
            }
            sum += 10;
        }
        
        /* Outer loop continuation with goto creating interesting flow */
        if (i == n - 1) {
            goto finish;
        }
        sum += 100;
    }
    
finish:
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc for some variability in loop bounds */
    int base = (argc > 1) ? atoi(argv[1]) : 10;
    if (base <= 0) base = 10;
    
    /* Run all test functions with different parameters */
    result += test_nested_simple(base, base * 2);
    printf("test_nested_simple: %d\n", result);
    
    result += test_nested_complex(base / 2, base);
    printf("test_nested_complex: %d\n", result);
    
    result += test_sequential_disjoint(base, base * 3);
    printf("test_sequential_disjoint: %d\n", result);
    
    result += test_switch_in_loop(base, 2);
    printf("test_switch_in_loop: %d\n", result);
    
    result += test_conditional_loops(base, base * 2, argc % 2);
    printf("test_conditional_loops: %d\n", result);
    
    result += test_deep_nesting(base / 3 + 1, base / 2 + 1, base / 4 + 1);
    printf("test_deep_nesting: %d\n", result);
    
    /* Final result check to prevent optimization */
    if (result == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
