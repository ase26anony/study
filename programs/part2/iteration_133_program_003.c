/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent aggressive optimization */
#define NOINLINE __attribute__((noinline, cold))
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
                break;     /* Creates another basic block */
            }
            sum += i + j;
        }
        
        /* Additional block in outer loop */
        if (i % 7 == 0) {
            sum -= 1;
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
        if (i < n/2) {
            sum += 10;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += i * j;
                continue;
            }
            sum += j;
            
            /* Early exit from inner loop */
            if (sum > 1000) {
                break;
            }
        }
        
        i++;
        /* Complex condition with multiple basic blocks */
        if (i % 3 == 0) {
            sum += 5;
        } else if (i % 4 == 0) {
            sum += 7;
        }
    } while (i < n);
    
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
            sum += arr1[i] * 2;
            /* Early continue creates separate block */
            continue;
        }
        sum += arr1[i];
        
        /* Nested if for more blocks */
        if (i % 5 == 0) {
            sum -= 1;
        }
    }
    
    /* Intermediate code to ensure disjointness */
    int temp = sum * 2;
    KEEP(temp);
    
    /* Second loop - processes arr2, completely disjoint from first */
    for (int j = 0; j < m && j < 100; ++j) {
        switch (arr2[j] % 6) {
            case 0:
                sum += arr2[j] * 3;
                break;
            case 1:
                sum += arr2[j] + 1;
                /* Fall through */
            case 2:
                sum += 5;
                break;
            default:
                sum += arr2[j];
                break;
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
            /* Switch creates multiple basic blocks */
            switch (i % 7) {
                case 0:
                    sum += i * 2;
                    /* Additional block in case */
                    if (sum % 3 == 0) {
                        sum += 10;
                    }
                    break;
                case 1:
                    sum += i + 1;
                    break;
                case 2:
                    sum += i * i;
                    /* Nested if */
                    if (i > 10) {
                        sum -= 5;
                    }
                    break;
                case 3:
                    sum += 3;
                    /* Continue creates separate flow */
                    continue;
                case 4:
                    sum += i / 2;
                    break;
                case 5:
                    sum += 7;
                    /* Early break from loop */
                    if (sum > 10000) {
                        goto end_inner;
                    }
                    break;
                default:
                    sum += 1;
                    break;
            }
            
            /* Additional code after switch */
            if (i % 11 == 0) {
                sum += 13;
            }
        }
        end_inner:
        
        /* Outer loop body continues */
        sum += k * 100;
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
            sum += i * 2;
            if (i % 3 == 0) {
                sum += 1;
                continue;
            }
            
            /* Nested loop in true branch */
            for (int j = 0; j < 5; ++j) {
                sum += j;
                if (j == 3) {
                    break;
                }
            }
        }
    } else {
        /* Different loop in false branch - disjoint from first */
        for (int i = m; i > 0; --i) {
            sum += i * 3;
            
            /* Multiple exit points */
            if (sum > 500) {
                return sum;  /* Early return creates exit block */
            }
            
            if (i % 4 == 0) {
                sum += 2;
                goto add_more;  /* goto creates another block */
            }
            
            sum += 1;
            
            add_more:
            sum += 5;
        }
        
        /* Additional loop after first in false branch */
        int count = 0;
        while (count < 10) {
            sum += count * 7;
            count++;
            
            /* Complex condition */
            if (count == 5) {
                sum += 20;
                continue;
            }
        }
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE int test_complex_nesting(int n, int m) {
    int sum = 0;
    KEEP(n); KEEP(m);
    
    /* Triple nesting */
    for (int i = 0; i < n; ++i) {
        sum += i;
        
        for (int j = 0; j < m; ++j) {
            sum += j;
            
            /* Innermost loop with multiple conditions */
            for (int k = 0; k < 8; ++k) {
                if (k % 2 == 0) {
                    sum += k * 2;
                    if (sum > 1000) {
                        goto middle_loop;  /* Jump to middle loop */
                    }
                    continue;
                }
                
                sum += k;
                
                if (k == 5) {
                    goto outer_loop;  /* Jump to outer loop */
                }
            }
            
            middle_loop:
            if (j % 3 == 0) {
                sum += 10;
            }
        }
        
        outer_loop:
        if (i % 4 == 0) {
            sum += 100;
        }
    }
    
    return sum;
}

/* Main driver */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use command line args for variability */
    int base = argc > 1 ? atoi(argv[1]) % 20 + 5 : 10;
    
    /* Call all test functions with different parameters */
    total += test_nested_simple(base, base + 3);
    total += test_shared_header(base + 1, base + 2);
    total += test_disjoint_loops(base + 2, base + 4);
    total += test_switch_in_loop(base + 5, 3);
    total += test_conditional_loops(base + 1, base + 3, argc % 2);
    total += test_complex_nesting(base + 2, base + 1);
    
    /* Ensure result is used */
    printf("Total checksum: %d\n", total);
    
    /* Additional compilation to force analysis */
    volatile int dummy = total;
    KEEP(dummy);
    
    return total > 0 ? 0 : 1;
}
