/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent aggressive optimization and inlining */
#define NO_INLINE __attribute__((noinline, cold))
#define KEEP_ALIVE(x) asm volatile("" : : "r"(x))

/* Function A: Simple nested loops with inner loop having multiple basic blocks */
NO_INLINE int test_nested_simple(int n, int m) {
    int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        KEEP_ALIVE(i);
        /* Inner loop with multiple basic blocks due to if-else */
        for (int j = 0; j < m; ++j) {
            KEEP_ALIVE(j);
            if (j % 2 == 0) {
                sum += i * 2;
                /* Continue creates another basic block */
                continue;
            } else {
                sum += j * 3;
                /* Break could create exit block */
                if (j == m - 1) break;
            }
            /* Additional block after if-else */
            sum += 1;
        }
        /* Block after inner loop */
        if (i % 3 == 0) {
            sum += 5;
        }
    }
    return sum;
}

/* Function B: Nested loops with shared header (do-while outer, for inner) */
NO_INLINE int test_nested_shared_header(int n, int m) {
    int sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared conditional block - both loops might share this in analysis */
        if (i < n/2) {
            sum += 10;
        }
        
        /* for inner loop */
        for (int j = 0; j < m; ++j) {
            KEEP_ALIVE(j);
            /* Multiple exit points */
            if (sum > 1000) {
                return sum;  /* Early return creates exit block */
            }
            if (j % 4 == 0) {
                sum += i + j;
                goto inner_label;  /* goto creates another block */
            }
            sum += j;
            inner_label:
            /* Empty label block */
            ;
        }
        
        i++;
        /* Conditional continue in outer loop */
        if (i % 2 == 0) {
            continue;
        }
        sum += 7;
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NO_INLINE int test_sequential_disjoint(int n, int m) {
    int sum = 0;
    int arr1[100], arr2[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n; ++i) {
        KEEP_ALIVE(i);
        /* Multiple basic blocks */
        if (i % 3 == 0) {
            sum += arr1[i % 100];
            /* Nested if creates more blocks */
            if (sum < 0) {
                sum = 0;
            }
        } else {
            sum -= arr1[i % 100];
        }
    }
    
    /* Intermediate code to ensure disjointness */
    int temp = sum * 2;
    KEEP_ALIVE(temp);
    
    /* Second loop - completely disjoint blocks */
    for (int j = 0; j < m; ++j) {
        KEEP_ALIVE(j);
        /* Different control flow pattern */
        switch (j % 4) {
            case 0: sum += arr2[j % 100]; break;
            case 1: sum -= arr2[j % 100]; break;
            case 2: sum *= 2; break;
            case 3: sum /= 2; if (sum < 0) sum = 0; break;
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NO_INLINE int test_switch_in_loop(int n, int outer_iter) {
    int sum = 0;
    
    /* Outer wrapper loop */
    for (int k = 0; k < outer_iter; ++k) {
        KEEP_ALIVE(k);
        
        /* Inner loop with switch */
        for (int i = 0; i < n; ++i) {
            KEEP_ALIVE(i);
            
            /* Switch creates multiple basic blocks */
            switch (i % 5) {
                case 0:
                    sum += i * 2;
                    /* break from switch, not loop */
                    break;
                case 1:
                    sum += i * 3;
                    if (sum > 1000) {
                        /* Early continue creates exit edge */
                        continue;
                    }
                    break;
                case 2:
                    sum += i * 4;
                    /* Nested loop inside case */
                    for (int j = 0; j < 2; ++j) {
                        sum += j;
                    }
                    break;
                case 3:
                    sum += i * 5;
                    /* goto within loop */
                    goto switch_label;
                case 4:
                    sum += i * 6;
                    break;
                default:
                    sum += i;
            }
            
            switch_label:
            /* Additional block after switch */
            sum += 1;
            
            /* Conditional break from loop */
            if (sum > 5000 && i > n/2) {
                break;
            }
        }
        
        /* Block after inner loop but inside outer */
        sum += k * 100;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting */
NO_INLINE int test_conditional_nesting(int n, int m, int flag) {
    int sum = 0;
    
    /* Outer conditional */
    if (flag) {
        /* First loop in true branch */
        for (int i = 0; i < n; ++i) {
            KEEP_ALIVE(i);
            /* Complex body with multiple blocks */
            if (i % 2 == 0) {
                for (int j = 0; j < 3; ++j) {
                    sum += i + j;
                    /* Nested conditional */
                    if (j == 1) {
                        sum += 10;
                        continue;
                    }
                }
            } else {
                sum += i * 2;
            }
            
            /* Another level of nesting */
            if (i > n/2) {
                for (int k = 0; k < 2; ++k) {
                    sum -= k;
                }
            }
        }
    } else {
        /* Different loop in false branch - disjoint from true branch loop */
        int count = 0;
        while (count < m) {
            KEEP_ALIVE(count);
            sum += count * 3;
            
            /* Multiple exit points */
            if (sum < -1000) {
                return sum;
            }
            
            /* Nested for loop */
            for (int x = 0; x < 2; ++x) {
                sum += x;
                if (x == 1 && count % 3 == 0) {
                    break;
                }
            }
            
            count++;
            
            /* Conditional continue */
            if (count % 4 == 0) {
                continue;
            }
            
            sum += 1;
        }
    }
    
    return sum;
}

/* Function F: Complex nested structure with overlapping but not fully nested loops */
NO_INLINE int test_partial_overlap(int n) {
    int sum = 0;
    int i = 0;
    
    /* Loop 1 */
    for (i = 0; i < n; ++i) {
        KEEP_ALIVE(i);
        sum += i;
        
        /* Loop 2 starts here but extends beyond Loop 1 */
        for (int j = 0; j < 3; ++j) {
            sum += j;
            
            /* This if block is shared between loops */
            if (j == 1 && i < n/2) {
                sum += 5;
                /* Early exit from inner loop only */
                if (sum > 100) break;
            }
        }
        /* Loop 2 ends here */
        
        /* More code in Loop 1 after Loop 2 ends */
        if (i % 2 == 0) {
            sum += 2;
        }
    }
    /* Loop 1 ends here */
    
    /* Loop 3: Shares some blocks with Loop 2 but not Loop 1 */
    for (int k = 0; k < 2; ++k) {
        sum += k * 10;
        /* This mimics part of Loop 2's body but is actually disjoint */
        if (k == 1) {
            sum += 5;
        }
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use command line args for variability, or defaults */
    int n = (argc > 1) ? atoi(argv[1]) % 50 + 10 : 20;
    int m = (argc > 2) ? atoi(argv[2]) % 40 + 10 : 15;
    int outer = (argc > 3) ? atoi(argv[3]) % 5 + 2 : 3;
    int flag = (argc > 4) ? atoi(argv[4]) % 2 : 1;
    
    printf("Testing hardware loop patterns with n=%d, m=%d, outer=%d, flag=%d\n", 
           n, m, outer, flag);
    
    /* Call all test functions */
    total += test_nested_simple(n, m);
    printf("test_nested_simple: %d\n", total);
    
    total += test_nested_shared_header(n, m);
    printf("test_nested_shared_header: %d\n", total);
    
    total += test_sequential_disjoint(n, m);
    printf("test_sequential_disjoint: %d\n", total);
    
    total += test_switch_in_loop(n, outer);
    printf("test_switch_in_loop: %d\n", total);
    
    total += test_conditional_nesting(n, m, flag);
    printf("test_conditional_nesting: %d\n", total);
    
    total += test_partial_overlap(n);
    printf("test_partial_overlap: %d\n", total);
    
    printf("Final total: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 1000000) {
        printf("Unexpected large result\n");
    }
    
    return total % 256;
}
