/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Force variable to stay in register, prevent constant propagation */
#define KEEP(var) asm volatile("" : : "r"(var))

/* Function A: Simple nested loops with inner loop having multiple blocks */
NOINLINE int test_nested_simple(int n, int m) {
    int sum = 0;
    KEEP(n);
    KEEP(m);
    
    /* Outer loop - will have its own blocks */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with multiple basic blocks due to if/continue */
        for (int j = 0; j < m; ++j) {
            if (j % 3 == 0) {
                /* Creates separate basic block */
                sum += i * 2;
                continue;  /* Creates another edge */
            } else if (j % 5 == 0) {
                /* Another basic block */
                sum += j * 3;
                if (sum > 1000) {
                    /* Yet another block with early exit */
                    return sum;
                }
            } else {
                /* Default block */
                sum += i + j;
            }
        }
        
        /* Block after inner loop, still in outer loop */
        if (i % 7 == 0) {
            sum -= 1;
        }
    }
    
    return sum;
}

/* Function B: Nested loops with shared header/complex relationship */
NOINLINE int test_nested_complex(int n, int m) {
    int sum = 0;
    int i = 0;
    KEEP(n);
    KEEP(m);
    
    /* do-while outer loop */
    do {
        /* Block that might be shared in bitmap analysis */
        int temp = i * 2;
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            /* Multiple blocks in inner loop */
            switch (j % 4) {
                case 0:
                    sum += temp + j;
                    break;
                case 1:
                    sum += temp - j;
                    /* Fall through */
                case 2:
                    sum += j * j;
                    if (sum < 0) {
                        /* Unlikely, but creates block */
                        sum = 0;
                    }
                    break;
                default:
                    sum += 1;
                    break;
            }
            
            /* Another conditional in inner loop */
            if (j == m / 2) {
                sum += 100;
                /* Could break or continue */
                if (sum > 5000) continue;
            }
        }
        
        i++;
        /* Conditional at end of do-while */
        if (i % 2 == 0) {
            sum += 10;
        }
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
        if (i % 2 == 0) {
            sum += arr1[i] * 2;
        } else {
            sum += arr1[i] / 2;
            /* Early exit possibility */
            if (sum > 10000) {
                goto finish_first;
            }
        }
        
        /* Another block in first loop */
        if (arr1[i] > 50) {
            sum -= 5;
        }
    }
finish_first:
    
    /* Second loop - completely disjoint blocks */
    for (int j = 0; j < m && j < 100; ++j) {
        switch (j % 3) {
            case 0:
                sum += arr2[j] + 1;
                break;
            case 1:
                sum += arr2[j] * 2;
                /* Nested if creates block */
                if (j > 10) {
                    sum += 20;
                }
                break;
            case 2:
                sum += arr2[j] - 1;
                break;
        }
        
        /* Conditional continue */
        if (j % 7 == 0) {
            continue;
        }
        
        sum += 1;
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
        /* Inner loop with switch */
        for (int i = 0; i < n; ++i) {
            /* Switch creates multiple blocks */
            switch (i % 5) {
                case 0:
                    sum += i * 10;
                    /* Nested if in case */
                    if (sum % 2 == 0) {
                        sum += 2;
                    }
                    break;
                case 1:
                    sum += i * 20;
                    for (int x = 0; x < 3; ++x) {
                        /* Tiny inner loop */
                        sum += x;
                    }
                    break;
                case 2:
                    sum += i * 30;
                    /* goto creates interesting flow */
                    if (sum > 5000) {
                        goto case_2_done;
                    }
                    sum += 1;
                    break;
                case 3:
                    sum += i * 40;
                    /* continue in switch */
                    continue;
                default:
                    sum += i * 50;
                    /* break from loop */
                    if (i == n - 1) {
                        break;
                    }
                    break;
            }
            
            /* Common code after switch */
            sum += 1;
            
        case_2_done:
            /* Label target */
            if (i % 11 == 0) {
                sum += 11;
            }
        }
        
        /* Outer loop body continues */
        sum += k * 100;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint control flow) */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    int sum = 0;
    KEEP(n);
    KEEP(m);
    KEEP(flag);
    
    if (flag > 0) {
        /* First branch loop */
        for (int i = 0; i < n; ++i) {
            if (i % 2 == 0) {
                sum += i * 3;
                /* Nested if */
                if (sum > 1000) {
                    sum -= 500;
                }
            } else {
                sum += i * 7;
            }
            
            /* Loop with multiple exits */
            if (i == n - 1) {
                break;
            } else if (i == n - 2) {
                sum += 99;
                continue;
            }
            
            sum += 1;
        }
    } else {
        /* Second branch loop (disjoint from first) */
        int j = 0;
        while (j < m) {
            sum += j * 2;
            
            /* Multiple continue points */
            if (j % 3 == 0) {
                j++;
                continue;
            }
            
            if (j % 4 == 0) {
                sum += 4;
                /* Early return */
                if (sum > 2000) {
                    return sum;
                }
            }
            
            j++;
        }
        
        /* Another loop in same branch */
        for (int k = 0; k < 5; ++k) {
            sum += k * 10;
        }
    }
    
    return sum;
}

/* Function F: Complex nested loops with multiple levels */
NOINLINE int test_multi_level_nested(int n, int m, int p) {
    int sum = 0;
    KEEP(n);
    KEEP(m);
    KEEP(p);
    
    /* Level 1 */
    for (int i = 0; i < n; ++i) {
        /* Level 2 */
        for (int j = 0; j < m; ++j) {
            /* Level 3 */
            for (int k = 0; k < p; ++k) {
                /* Complex body with multiple blocks */
                if (k % 2 == 0) {
                    sum += i + j + k;
                    if (sum % 3 == 0) {
                        sum += 3;
                        /* continue at different level */
                        if (j % 2 == 0) continue;
                    }
                } else {
                    sum += i * j * k;
                }
                
                /* Another conditional */
                if (k == p / 2) {
                    sum += 100;
                    /* break from middle loop */
                    if (sum > 5000) {
                        goto break_middle;
                    }
                }
            }
            
            /* Code between level 2 and 3 loops */
            sum += j * 10;
        }
    break_middle:
        /* After breaking from middle loop */
        sum += i * 100;
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create some variability in iteration counts */
    int base = (argc > 1) ? atoi(argv[1]) : 10;
    if (base < 5) base = 5;
    if (base > 100) base = 100;
    
    printf("Testing hardware loop patterns with base=%d\n", base);
    
    /* Test 1: Simple nested loops */
    result += test_nested_simple(base, base/2);
    printf("Test 1 result: %d\n", result);
    
    /* Test 2: Complex nested loops */
    result += test_nested_complex(base, base/3);
    printf("Test 2 result: %d\n", result);
    
    /* Test 3: Sequential disjoint loops */
    result += test_sequential_disjoint(base, base/2);
    printf("Test 3 result: %d\n", result);
    
    /* Test 4: Loop with switch inside outer loop */
    result += test_switch_in_loop(base, 3);
    printf("Test 4 result: %d\n", result);
    
    /* Test 5: Conditional loops (disjoint paths) */
    result += test_conditional_loops(base, base/2, argc % 2);
    printf("Test 5 result: %d\n", result);
    
    /* Test 6: Multi-level nested loops */
    result += test_multi_level_nested(base/2, base/3, base/4);
    printf("Test 6 result: %d\n", result);
    
    printf("Final checksum: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    volatile int dummy = result;
    (void)dummy;
    
    return (result != 0) ? 0 : 1;
}
