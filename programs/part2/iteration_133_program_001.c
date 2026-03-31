/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int dummy;

/* Function A: Simple nested loops with inner loop having multiple basic blocks */
NOINLINE int test_nested_simple(int n, int m) {
    int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with multiple basic blocks */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += i * j;  /* Basic block A1 */
                continue;       /* Creates separate basic block for continue */
            }
            sum -= i + j;       /* Basic block A2 */
        }
        /* Additional block in outer loop */
        if (i % 3 == 0) {
            sum += 100;
        }
    }
    return sum;
}

/* Function B: Nested loops with shared header/complex control flow */
NOINLINE int test_nested_complex(int n, int m) {
    int sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared conditional block - could be part of both loops' bitmaps */
        if (i % 2 == 0) {
            sum += 5;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            if (j < i) {
                sum += j * 2;   /* Basic block B1 */
                if (j == i/2) {
                    break;      /* Creates exit block */
                }
            } else {
                sum += j;       /* Basic block B2 */
            }
        }
        
        i++;
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_disjoint_loops(int n, int m) {
    int sum = 0;
    int arr1[100], arr2[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = 100 - i;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n; ++i) {
        if (i % 4 == 0) {       /* Creates multiple blocks */
            sum += arr1[i % 100];
        } else {
            sum -= arr1[i % 100];
        }
    }
    
    /* Intermediate code to ensure disjointness */
    int temp = sum * 2;
    asm volatile("" : "+r"(temp) : :);
    
    /* Second loop - completely disjoint from first */
    for (int j = 0; j < m; ++j) {
        if (j % 3 == 0) {       /* Creates multiple blocks */
            sum += arr2[j % 100] * 2;
        } else if (j % 3 == 1) {
            sum += arr2[j % 100];
        } else {
            sum -= arr2[j % 100];
        }
    }
    
    return sum + temp;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    int sum = 0;
    
    /* Outer wrapper loop */
    for (int k = 0; k < outer_iter; ++k) {
        /* Inner loop with switch statement */
        for (int i = 0; i < n; ++i) {
            switch (i % 5) {
                case 0:
                    sum += i * 2;   /* Case block 0 */
                    break;
                case 1:
                    sum += i + 10;  /* Case block 1 */
                    if (i == n/2) {
                        continue;   /* Creates separate continue block */
                    }
                    break;
                case 2:
                    sum += i * i;   /* Case block 2 */
                    break;
                case 3:
                    sum -= i;       /* Case block 3 */
                    /* Fall through */
                case 4:
                    sum += 5;       /* Case block 4 */
                    break;
                default:
                    sum += 1;       /* Default block */
            }
            
            /* Additional block after switch */
            if (i % 7 == 0) {
                sum += 1000;
            }
        }
        
        /* Outer loop body continuation */
        sum += k * 10000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint control flow paths) */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    int sum = 0;
    
    if (flag > 0) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            if (i % 2 == 0) {
                sum += i * 3;
                continue;
            }
            sum += i;
            if (i == n - 1) {
                break;
            }
        }
    } else {
        /* Different loop in false branch - disjoint from true branch loop */
        int j = 0;
        while (j < m) {
            sum += j * j;
            j++;
            if (j % 4 == 0) {
                sum += 50;
                /* Early return creates exit block */
                if (sum > 1000) {
                    return sum;
                }
            }
        }
    }
    
    return sum;
}

/* Function F: Deeply nested loops for complex bitmap relationships */
NOINLINE int test_deep_nesting(int n, int m, int p) {
    int sum = 0;
    
    /* Level 1 loop */
    for (int i = 0; i < n; ++i) {
        /* Level 2 loop */
        for (int j = 0; j < m; ++j) {
            /* Level 3 loop */
            for (int k = 0; k < p; ++k) {
                if (k % 2 == 0) {
                    sum += i + j + k;
                    if (k == p/2) {
                        goto inner_exit;  /* Creates goto exit block */
                    }
                } else {
                    sum += i * j * k;
                }
            }
            inner_exit:
            sum += j * 10;
        }
        
        /* Additional control flow in outer loop */
        switch (i % 3) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 2: sum += 3; break;
        }
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments for variability, but provide defaults */
    int n = (argc > 1) ? atoi(argv[1]) % 100 + 10 : 50;
    int m = (argc > 2) ? atoi(argv[2]) % 100 + 10 : 40;
    int p = (argc > 3) ? atoi(argv[3]) % 50 + 5 : 20;
    int flag = (argc > 4) ? atoi(argv[4]) % 2 : 0;
    int outer_iter = (argc > 5) ? atoi(argv[5]) % 5 + 2 : 3;
    
    /* Force values into registers to prevent constant propagation */
    asm volatile("" : "+r"(n), "+r"(m), "+r"(p), "+r"(flag), "+r"(outer_iter));
    
    /* Execute all test functions */
    result += test_nested_simple(n, m);
    result += test_nested_complex(n, m);
    result += test_disjoint_loops(n, m);
    result += test_switch_in_loop(n, outer_iter);
    result += test_conditional_loops(n, m, flag);
    result += test_deep_nesting(n, m, p);
    
    /* Use result to prevent dead code elimination */
    dummy = result;
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
