/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop having multiple basic blocks */
NOINLINE int test_nested_simple(int n, int m) {
    volatile int sum = 0;
    /* Outer loop - creates one set of basic blocks */
    for (int i = 0; i < n; ++i) {
        sum += i;
        /* Inner loop - creates another set of basic blocks */
        for (int j = 0; j < m; ++j) {
            /* Create multiple basic blocks in inner loop */
            if (j % 3 == 0) {
                sum += j * 2;
                continue;  /* Creates separate basic block for continue path */
            } else if (j % 5 == 0) {
                sum += j * 3;
                break;     /* Creates separate basic block for break path */
            } else {
                sum += j;
            }
            /* Another basic block after the if-else chain */
            sum += 1;
        }
        /* Basic block after inner loop */
        sum += 100;
    }
    return sum;
}

/* Function B: Nested loops with shared header/complex control flow */
NOINLINE int test_nested_complex(int n, int m) {
    volatile int sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared header block - could be part of both loops' bitmaps */
        if (i % 2 == 0) {
            sum += i * 2;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            /* Multiple basic blocks in inner loop */
            switch (j % 4) {
                case 0:
                    sum += j;
                    break;
                case 1:
                    sum += j * 2;
                    continue;  /* Different continue path */
                case 2:
                    sum += j * 3;
                    /* Fall through */
                default:
                    sum += j * 4;
                    if (sum > 1000) {
                        goto outer_loop_end;  /* Early exit creates new block */
                    }
            }
            /* Another basic block */
            sum += 5;
        }
        
        /* Label for goto target */
        outer_loop_end:
        sum += 10;
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
        arr2[k] = k * 2;
    }
    
    /* First loop - completely disjoint from second loop */
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            sum += arr1[i % 100] * 2;
        } else {
            sum += arr1[i % 100];
            if (sum > 500) {
                sum -= 100;  /* Creates another basic block */
            }
        }
        /* Basic block after if-else */
        sum += 1;
    }
    
    /* Completely separate second loop */
    for (int j = 0; j < m; ++j) {
        switch (j % 3) {
            case 0:
                sum += arr2[j % 100];
                break;
            case 1:
                sum += arr2[j % 100] * 2;
                /* No break - fall through */
            case 2:
                sum += arr2[j % 100] * 3;
                break;
        }
        /* Another basic block */
        sum += 2;
    }
    
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    volatile int sum = 0;
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < outer_iter; ++outer) {
        sum += outer * 100;
        
        /* Inner loop with switch */
        for (int i = 0; i < n; ++i) {
            /* Switch creates multiple basic blocks */
            switch (i % 5) {
                case 0:
                    sum += i;
                    if (sum % 7 == 0) {
                        continue;  /* Continue from within switch case */
                    }
                    break;
                case 1:
                    sum += i * 2;
                    break;
                case 2:
                    sum += i * 3;
                    /* Early return creates exit block */
                    if (sum > 10000) {
                        return sum;
                    }
                    break;
                case 3:
                    sum += i * 4;
                    /* Nested if for more blocks */
                    if (i % 2 == 0) {
                        sum += 10;
                    } else {
                        sum += 20;
                    }
                    break;
                default:  /* case 4 */
                    sum += i * 5;
                    for (int k = 0; k < 2; ++k) {
                        sum += k;  /* Tiny nested loop inside switch case */
                    }
                    break;
            }
            /* Basic block after switch */
            sum += 1;
        }
        
        /* Basic block after inner loop */
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
            if (i % 3 == 0) {
                sum += i * 2;
                continue;
            }
            sum += i;
            /* Nested tiny loop */
            for (int j = 0; j < 2; ++j) {
                sum += j;
            }
        }
    } else {
        /* Different loop in false branch - completely disjoint */
        int k = 0;
        while (k < m) {
            sum += k * 3;
            k++;
            if (k % 4 == 0) {
                sum += 100;
                break;  /* Early break creates exit block */
            }
        }
        
        /* Another loop after while */
        for (int p = 0; p < m / 2; ++p) {
            sum += p;
            if (p % 5 == 0) {
                continue;
            }
            sum += p * 2;
        }
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple exit points */
NOINLINE int test_complex_nesting(int n, int m) {
    volatile int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        sum += i;
        
        /* Middle loop */
        for (int j = 0; j < m; ++j) {
            /* Multiple conditions creating many basic blocks */
            if (j % 2 == 0) {
                if (j % 3 == 0) {
                    sum += j * 2;
                    continue;
                } else {
                    sum += j * 3;
                }
            } else {
                sum += j;
            }
            
            /* Innermost tiny loop */
            for (int k = 0; k < 3; ++k) {
                sum += k;
                if (k == 1 && sum > 500) {
                    goto middle_loop_end;  /* Jump to middle loop end */
                }
            }
            
            /* Another basic block */
            sum += 10;
        }
        middle_loop_end:
        sum += 100;
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create some runtime variability */
    int n = (argc > 1) ? atoi(argv[1]) % 50 + 10 : 20;
    int m = (argc > 2) ? atoi(argv[2]) % 30 + 5 : 15;
    int outer_iter = (argc > 3) ? atoi(argv[3]) % 5 + 2 : 3;
    int flag = (argc > 4) ? atoi(argv[4]) % 2 : 1;
    
    /* Force variables into registers to prevent constant propagation */
    asm volatile("" : : "r"(n), "r"(m), "r"(outer_iter), "r"(flag));
    
    printf("Testing hardware loop patterns...\n");
    
    result += test_nested_simple(n, m);
    printf("test_nested_simple: %d\n", result);
    
    result += test_nested_complex(n, m);
    printf("test_nested_complex: %d\n", result);
    
    result += test_sequential_disjoint(n, m);
    printf("test_sequential_disjoint: %d\n", result);
    
    result += test_switch_in_loop(n, outer_iter);
    printf("test_switch_in_loop: %d\n", result);
    
    result += test_conditional_loops(n, m, flag);
    printf("test_conditional_loops: %d\n", result);
    
    result += test_complex_nesting(n, m);
    printf("test_complex_nesting: %d\n", result);
    
    printf("Final result: %d\n", result);
    
    return result != 0 ? 0 : 1;  /* Return non-zero if all tests returned 0 */
}
