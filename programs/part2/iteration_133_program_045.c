/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent aggressive optimization and inlining */
#define NOINLINE __attribute__((noinline, cold))
#define KEEP(var) asm volatile("" : : "r"(var))

/* Function A: Simple nested loops with inner loop continue */
NOINLINE int test_nested_simple(int n, int m) {
    int sum = 0;
    KEEP(n); KEEP(m);
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        sum += i;
        
        /* Inner loop with multiple basic blocks due to continue */
        for (int j = 0; j < m; ++j) {
            if (j % 3 == 0) {
                sum += j * 2;
                continue;  /* Creates separate basic block */
            }
            sum += j;
            
            if (j % 7 == 0) {
                sum -= 1;  /* Another basic block */
            }
        }
        
        /* Additional block in outer loop */
        if (i % 5 == 0) {
            sum *= 2;
        }
    }
    
    return sum;
}

/* Function B: Nested loops with shared header using do-while */
NOINLINE int test_nested_shared_header(int n, int m) {
    int sum = 0;
    int i = 0;
    KEEP(n); KEEP(m);
    
    /* Shared header block */
    if (n > 0 && m > 0) {
        sum = 1;
    }
    
    /* Outer do-while loop */
    do {
        /* This block is shared by both loops conceptually */
        if (i % 2 == 0) {
            sum += i * 3;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            sum += j;
            
            /* Early exit from inner loop creates another block */
            if (j > m/2) {
                sum += 100;
                break;
            }
            
            /* Continue creates another block */
            if (j % 4 == 0) {
                continue;
            }
            
            sum += j * 2;
        }
        
        i++;
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n, int m) {
    int sum = 0;
    int arr1[100], arr2[100];
    KEEP(n); KEEP(m);
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    /* First loop - disjoint body */
    for (int i = 0; i < n; ++i) {
        if (i % 3 == 0) {
            sum += arr1[i % 100];
        } else {
            sum -= arr1[i % 100];
        }
        
        /* Additional block */
        switch (i % 4) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 2: sum += 3; break;
            default: sum += 4; break;
        }
    }
    
    /* Intermediate code to ensure disjointness */
    int temp = sum * 2;
    KEEP(temp);
    
    /* Second loop - completely disjoint from first */
    for (int j = 0; j < m; ++j) {
        if (j % 5 == 0) {
            sum += arr2[j % 100] * 3;
        }
        
        /* Nested if for more blocks */
        if (j > m/2) {
            sum -= 50;
        } else {
            sum += 10;
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
        sum += k * 100;
        
        /* Inner loop with switch */
        for (int i = 0; i < n; ++i) {
            /* Switch creates multiple basic blocks */
            switch (i % 6) {
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
                    sum += i * 4;
                    /* Complex block with condition */
                    if (sum > 1000) {
                        sum -= 500;
                    }
                    break;
                case 4:
                    sum += i * 5;
                    /* Nested if */
                    if (i % 3 == 0) {
                        sum += 7;
                    }
                    break;
                default:  /* case 5 */
                    sum += i * 6;
                    /* Loop with early exit */
                    for (int j = 0; j < 3; ++j) {
                        if (j == 2) break;
                        sum += j;
                    }
                    break;
            }
            
            /* Additional condition after switch */
            if (i % 7 == 0) {
                continue;
            }
            
            sum += 1;
        }
        
        /* Block only in outer loop */
        if (k % 2 == 0) {
            sum *= 2;
        }
    }
    
    return sum;
}

/* Function E: Conditional loop nesting - disjoint loops in branches */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    int sum = 0;
    KEEP(n); KEEP(m); KEEP(flag);
    
    if (flag) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            sum += i * 2;
            
            /* Nested loop in true branch */
            for (int j = 0; j < 3; ++j) {
                sum += j;
                if (j == 1) {
                    continue;
                }
                sum += 5;
            }
            
            if (i % 4 == 0) {
                sum -= 2;
            }
        }
    } else {
        /* Different loop in false branch - disjoint from true branch */
        for (int i = 0; i < m; ++i) {
            sum += i * 3;
            
            /* Different control flow */
            switch (i % 5) {
                case 0: sum += 10; break;
                case 1: sum += 20; break;
                case 2: sum += 30; break;
                case 3: sum += 40; break;
                case 4: sum += 50; break;
            }
            
            /* Early return possibility */
            if (sum > 10000) {
                return sum;
            }
        }
        
        /* Additional loop in false branch */
        for (int k = 0; k < n/2; ++k) {
            sum += k * 7;
            if (k % 3 == 0) {
                sum += 100;
                continue;
            }
        }
    }
    
    return sum;
}

/* Function F: Complex nested loops with multiple exits */
NOINLINE int test_complex_nesting(int n, int m) {
    int sum = 0;
    KEEP(n); KEEP(m);
    
    /* Triple nesting */
    for (int i = 0; i < n; ++i) {
        sum += i;
        
        for (int j = 0; j < m; ++j) {
            sum += j;
            
            /* Innermost loop with multiple exits */
            for (int k = 0; k < 5; ++k) {
                sum += k;
                
                if (k == 3) {
                    goto inner_exit;  /* Creates exit edge */
                }
                
                if (k == 4) {
                    break;  /* Another exit */
                }
                
                sum += 1;
            }
            
            inner_exit:
            if (j % 2 == 0) {
                continue;
            }
            
            sum += 10;
        }
        
        /* Exit from middle loop */
        if (sum > 5000) {
            break;
        }
        
        sum += 100;
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use argc for variability without complex control flow */
    int base = (argc > 1) ? atoi(argv[1]) : 10;
    if (base <= 0) base = 10;
    
    /* Run all test functions with different parameters */
    total += test_nested_simple(base, base/2 + 1);
    total += test_nested_shared_header(base, base/3 + 1);
    total += test_sequential_disjoint(base, base/2 + 2);
    total += test_switch_in_loop(base/2 + 3, 2);
    total += test_conditional_loops(base, base/2, base % 2);
    total += test_complex_nesting(base/3 + 1, base/4 + 1);
    
    /* Output result to prevent elimination */
    printf("Total checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
