/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Force variable to stay in register */
#define KEEP(var) asm volatile("" : : "r"(var))

/* Function A: Simple nested loops with inner conditional */
NOINLINE int test_nested_simple(int n, int m) {
    int sum = 0;
    KEEP(n); KEEP(m);
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with multiple basic blocks */
        for (int j = 0; j < m; ++j) {
            if (j % 3 == 0) {
                sum += i * 2;
                continue;  /* Creates separate basic block */
            } else if (j % 5 == 0) {
                sum += j * 3;
                if (sum > 1000) break;  /* Another basic block */
            } else {
                sum += i + j;
            }
        }
        
        /* Additional block in outer loop */
        if (i % 7 == 0) {
            sum -= 5;
        }
    }
    
    return sum;
}

/* Function B: Nested loops with shared header */
NOINLINE int test_shared_header(int n, int m) {
    int sum = 0;
    int i = 0;
    KEEP(n); KEEP(m);
    
    /* do-while outer loop */
    do {
        /* Shared conditional block - could be considered part of both loops */
        if (i % 2 == 0) {
            sum += 10;
        }
        
        /* for inner loop */
        for (int j = 0; j < m; ++j) {
            sum += (i * j) % 17;
            if (j == m/2) {
                sum += 100;  /* Creates separate block */
            }
        }
        
        i++;
        if (i > n) break;  /* Multiple exit points */
        
        /* Another conditional in outer loop */
        switch (i % 4) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 2: sum += 3; break;
            case 3: sum += 4; break;
        }
    } while (i < n * 2);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_disjoint_loops(int n, int m) {
    int sum = 0;
    int arr1[100], arr2[100];
    KEEP(n); KEEP(m);
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i * 2;
        arr2[i] = i * 3;
    }
    
    /* First loop - processes arr1 */
    for (int i = 0; i < n && i < 100; ++i) {
        if (arr1[i] % 4 == 0) {
            sum += arr1[i];
            continue;  /* Creates separate block */
        }
        sum += arr1[i] / 2;
    }
    
    /* Intermediate computation (separate basic blocks) */
    sum = (sum * 3) % 1000;
    
    /* Second loop - processes arr2 (disjoint from first) */
    for (int j = 0; j < m && j < 100; ++j) {
        switch (arr2[j] % 5) {
            case 0: sum += arr2[j]; break;
            case 1: sum += arr2[j] * 2; break;
            case 2: sum += arr2[j] * 3; break;
            default: sum += arr2[j] / 2; break;
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    int sum = 0;
    KEEP(n); KEEP(outer_iter);
    
    /* Outer wrapper loop */
    for (int k = 0; k < outer_iter; ++k) {
        /* Inner loop with switch */
        for (int i = 0; i < n; ++i) {
            switch (i % 6) {
                case 0:
                    sum += i * 2;
                    if (sum % 7 == 0) break;  /* break from switch only */
                    /* fall through */
                case 1:
                    sum += i + 1;
                    break;
                case 2:
                    sum += i * i;
                    /* Multiple statements in case */
                    if (i > 10) {
                        sum -= 5;
                    }
                    break;
                case 3:
                    sum += i / 2;
                    break;
                case 4:
                    sum += i % 3;
                    /* Nested if */
                    if (i % 2 == 0) {
                        sum += 10;
                    } else {
                        sum += 20;
                    }
                    break;
                default:  /* case 5 */
                    sum += 100;
                    break;
            }
            
            /* Additional block after switch */
            if (i == n - 1) {
                sum += 1000;
            }
        }
        
        /* Block in outer loop but not inner */
        sum = (sum * 13) % 10000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    int sum = 0;
    KEEP(n); KEEP(m); KEEP(flag);
    
    if (flag > 0) {
        /* First loop in true branch */
        for (int i = 0; i < n; ++i) {
            sum += i * i;
            if (i % 3 == 0) {
                continue;  /* Creates separate block */
            }
            sum += i;
            
            /* Early exit possibility */
            if (sum > 5000) {
                return sum;  /* Multiple exit points */
            }
        }
        
        /* Additional computation after loop */
        sum = sum % 1000;
    } else {
        /* Different loop in false branch (disjoint from first) */
        int j = m;
        while (j-- > 0) {
            sum += j * 3;
            if (j % 4 == 0) {
                sum += 7;
                /* Nested if */
                if (sum % 11 == 0) {
                    sum += 11;
                }
            }
        }
        
        /* Loop with goto creating complex control flow */
        for (int k = 0; k < n; ++k) {
            if (k % 5 == 0) goto add_special;
            sum += k;
            continue;
            
        add_special:
            sum += 50;
        }
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple levels */
NOINLINE int test_multi_level_nesting(int n, int m, int p) {
    int sum = 0;
    KEEP(n); KEEP(m); KEEP(p);
    
    /* Level 1: outermost loop */
    for (int i = 0; i < n; ++i) {
        sum += i;
        
        /* Level 2: middle loop */
        for (int j = 0; j < m; ++j) {
            sum += j * 2;
            
            /* Level 3: innermost loop */
            for (int k = 0; k < p; ++k) {
                if (k % 2 == 0) {
                    sum += i + j + k;
                    continue;
                }
                
                if (k % 3 == 0) {
                    sum += (i * j * k) % 17;
                    if (sum > 10000) {
                        goto exit_inner;  /* Complex control flow */
                    }
                }
                
                sum += 1;
            exit_inner:
                ; /* Label target */
            }
            
            /* Block in middle loop but not innermost */
            if (j % 7 == 0) {
                sum += 7;
            }
        }
        
        /* Block in outer loop only */
        switch (i % 5) {
            case 0: case 1: case 2:
                sum += 10;
                break;
            case 3:
                sum += 20;
                /* fall through */
            default:
                sum += 5;
                break;
        }
    }
    
    return sum;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use command line args for variability, but keep small for fast execution */
    int n = argc > 1 ? atoi(argv[1]) % 50 + 10 : 20;
    int m = argc > 2 ? atoi(argv[2]) % 40 + 10 : 15;
    int p = argc > 3 ? atoi(argv[3]) % 30 + 5 : 10;
    int flag = argc > 4 ? atoi(argv[4]) % 2 : 0;
    
    /* Call all test functions */
    total += test_nested_simple(n, m);
    total += test_shared_header(n, m);
    total += test_disjoint_loops(n, m);
    total += test_switch_in_loop(n, 3);  /* Fixed outer iterations */
    total += test_conditional_loops(n, m, flag);
    total += test_multi_level_nesting(n / 2, m / 2, p);
    
    /* Prevent dead code elimination */
    volatile int result = total % 10000;
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;  /* Ensure non-zero exit for some inputs */
}
