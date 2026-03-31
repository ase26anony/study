/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Force variable to register to prevent constant propagation */
#define KEEP(var) asm volatile("" : : "r"(var))

/* Function A: Simple nested loops with inner conditional */
NOINLINE int test_nested_simple(int n, int m) {
    int sum = 0;
    KEEP(n); KEEP(m);
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        sum += i;
        
        /* Inner loop with conditional continue creating multiple blocks */
        for (int j = 0; j < m; ++j) {
            if (j % 3 == 0) {
                sum += j * 2;
                continue;  /* Creates separate basic block */
            }
            sum += j;
            
            /* Additional block complexity */
            if (j % 5 == 0) {
                sum += 1;
            }
        }
        
        /* Block after inner loop but still in outer loop */
        if (i % 2 == 0) {
            sum -= 1;
        }
    }
    
    return sum;
}

/* Function B: Do-while outer with for inner, shared header complexity */
NOINLINE int test_do_while_nested(int n, int m) {
    int sum = 0;
    int i = 0;
    KEEP(n); KEEP(m);
    
    /* Shared header block that both loops conceptually share */
    if (n > 0 && m > 0) {
        sum = 1;
    }
    
    /* Do-while outer loop */
    do {
        /* Conditional before inner loop */
        if (i % 2 == 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        /* For inner loop */
        for (int j = 0; j < m; ++j) {
            sum += (i * j);
            
            /* Early exit from inner loop creates another block */
            if (j > m / 2) {
                break;
            }
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
        arr2[i] = 100 - i;
    }
    
    /* First loop - processes arr1 */
    for (int i = 0; i < n && i < 100; ++i) {
        if (arr1[i] % 2 == 0) {
            sum += arr1[i] * 2;
        } else {
            sum += arr1[i];
        }
        
        /* Additional block */
        switch (i % 3) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            default: sum += 3; break;
        }
    }
    
    /* Block between loops - ensures disjoint bitmaps */
    sum += 1000;
    
    /* Second loop - processes arr2 (completely disjoint blocks) */
    for (int i = 0; i < m && i < 100; ++i) {
        if (arr2[i] > 50) {
            sum += arr2[i] / 2;
        } else {
            sum += arr2[i];
        }
        
        /* Different control flow pattern */
        for (int k = 0; k < 2; ++k) {
            sum += k;
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    int sum = 0;
    KEEP(n); KEEP(outer_iter);
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < outer_iter; ++outer) {
        sum += outer * 100;
        
        /* Inner loop with switch statement */
        for (int i = 0; i < n; ++i) {
            /* Switch creates multiple basic blocks within the loop */
            switch (i % 5) {
                case 0:
                    sum += i;
                    /* Fall through */
                case 1:
                    sum += i * 2;
                    break;
                case 2:
                    sum += i * 3;
                    /* Nested if in case */
                    if (i % 3 == 0) {
                        sum += 10;
                    }
                    break;
                case 3:
                    sum += i * 4;
                    /* Small loop inside case */
                    for (int j = 0; j < 2; ++j) {
                        sum += j;
                    }
                    break;
                default: /* case 4 */
                    sum += i * 5;
                    /* Conditional return creates exit block */
                    if (sum > 10000) {
                        return sum;  /* Multiple exit points */
                    }
                    break;
            }
            
            /* Additional block after switch */
            if (i % 7 == 0) {
                sum -= 1;
            }
        }
        
        /* Block after inner loop but in outer loop */
        sum += outer;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint control flow paths) */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    int sum = 0;
    KEEP(n); KEEP(m); KEEP(flag);
    
    if (flag > 0) {
        /* First path: loop with complex body */
        for (int i = 0; i < n; ++i) {
            sum += i * i;
            
            /* Nested loop in true branch */
            for (int j = 0; j < 3; ++j) {
                sum += j;
                if (j == 1) {
                    continue;  /* Creates separate block */
                }
                sum += 1;
            }
            
            /* Conditional break */
            if (sum > 1000) {
                break;
            }
        }
    } else {
        /* Second path: completely different loop structure */
        int k = 0;
        while (k < m) {
            sum += k * 3;
            
            /* Do-while inside while */
            int l = 0;
            do {
                sum += l;
                l++;
            } while (l < 2);
            
            k++;
            
            /* Multiple exit points */
            if (k % 4 == 0) {
                goto early_exit;
            }
        }
        
        early_exit:
        sum += 500;
    }
    
    /* Common code after conditional */
    sum += flag * 10;
    
    return sum;
}

/* Function F: Complex nested loops with shared and disjoint regions */
NOINLINE int test_complex_nesting(int n, int m) {
    int sum = 0;
    KEEP(n); KEEP(m);
    
    /* Level 1 loop */
    for (int i = 0; i < n; ++i) {
        sum += i;
        
        /* Level 2 loop A */
        for (int j = 0; j < m; ++j) {
            sum += j;
            
            /* Shared block with Level 2 loop B */
            if (j % 2 == 0) {
                sum += 1;
            }
            
            /* Level 3 loop (deeply nested) */
            for (int k = 0; k < 2; ++k) {
                sum += k;
                
                /* Multiple blocks in innermost loop */
                switch (k) {
                    case 0: sum += 5; break;
                    case 1: sum += 10; 
                            if (sum % 3 == 0) {
                                continue;  /* Skip to next iteration */
                            }
                            break;
                }
            }
        }
        
        /* Level 2 loop B (sibling of loop A, shares some blocks) */
        for (int j = 0; j < i && j < m; ++j) {
            sum += j * 2;
            
            /* Shared block with Level 2 loop A */
            if (j % 2 == 0) {
                sum += 1;
            }
            
            /* Different block not shared with loop A */
            if (j % 3 == 0) {
                sum += 100;
            }
        }
    }
    
    return sum;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use command line arguments for variability, but keep small for fast execution */
    int base = (argc > 1) ? atoi(argv[1]) % 10 + 5 : 7;
    
    printf("Testing hardware loop patterns...\n");
    
    /* Call all test functions with different parameters */
    total += test_nested_simple(base, base + 2);
    total += test_do_while_nested(base + 1, base + 3);
    total += test_sequential_disjoint(base + 2, base + 4);
    total += test_switch_in_loop(base + 3, 2);  /* Outer iter fixed at 2 */
    total += test_conditional_loops(base + 4, base + 5, base % 2);
    total += test_complex_nesting(base + 1, base + 2);
    
    printf("Total checksum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
