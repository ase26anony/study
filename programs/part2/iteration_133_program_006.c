/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent aggressive optimization and inlining */
#define NOINLINE __attribute__((noinline, cold))
#define KEEP(var) asm volatile("" : : "r"(var))

/* Function A: Simple nested loops with inner loop continue */
NOINLINE int test_nested_simple(int n, int m) {
    int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with multiple basic blocks due to continue */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += i * j;
                continue;  /* Creates separate basic block */
            }
            sum += i + j;  /* Another basic block */
        }
        /* Additional block in outer loop */
        if (i % 3 == 0) {
            sum += 1;
        }
    }
    KEEP(sum);
    return sum;
}

/* Function B: Nested loops with shared header using do-while */
NOINLINE int test_nested_do_while(int n, int m) {
    int sum = 0;
    int i = 0;
    
    /* Outer do-while loop */
    do {
        /* Shared conditional block - could be part of both loops' bitmaps */
        if (i < n/2) {
            sum += 10;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            /* Multiple exit points from inner loop */
            if (j == m - 1) {
                sum += 100;
                break;  /* Creates exit block */
            }
            if (j % 4 == 0) {
                continue;  /* Another basic block */
            }
            sum += i * j;
        }
        
        i++;
        /* Complex condition with multiple basic blocks */
        if (i % 2 == 0) {
            sum += 5;
        }
    } while (i < n);
    
    KEEP(sum);
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n, int m) {
    int sum = 0;
    int arr1[100], arr2[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = 100 - i;
    }
    
    /* First loop - processes arr1 */
    for (int i = 0; i < n && i < 100; ++i) {
        if (arr1[i] % 3 == 0) {  /* Creates multiple blocks */
            sum += arr1[i] * 2;
        } else {
            sum += arr1[i];
        }
        /* Early return possibility */
        if (sum > 10000) {
            return sum;  /* Creates exit block */
        }
    }
    
    /* Second loop - completely disjoint from first, processes arr2 */
    for (int j = 0; j < m && j < 100; ++j) {
        switch (arr2[j] % 4) {  /* Switch creates multiple blocks */
            case 0: sum += arr2[j]; break;
            case 1: sum += arr2[j] * 2; break;
            case 2: sum += arr2[j] * 3; break;
            default: sum += arr2[j] * 4; break;
        }
    }
    
    KEEP(sum);
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    int sum = 0;
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < outer_iter; ++outer) {
        /* Inner loop with switch statement */
        for (int i = 0; i < n; ++i) {
            /* Switch creates many basic blocks */
            switch (i % 5) {
                case 0:
                    sum += i * 1;
                    if (sum % 2 == 0) continue;  /* Branch within case */
                    break;
                case 1:
                    sum += i * 2;
                    break;
                case 2:
                    sum += i * 3;
                    /* Nested if creates another block */
                    if (i > n/2) {
                        sum += 100;
                    }
                    break;
                case 3:
                    sum += i * 4;
                    /* goto creates interesting control flow */
                    if (i == n - 1) goto end_inner;
                    break;
                case 4:
                    sum += i * 5;
                    break;
                default:
                    sum += i;
            }
            
            /* Additional block after switch */
            sum += 1;
        }
        end_inner:
        /* Continue outer loop */
        sum += outer * 1000;
    }
    
    KEEP(sum);
    return sum;
}

/* Function E: Conditional loop nesting - disjoint loops in branches */
NOINLINE int test_conditional_loops(int n, int m, int condition) {
    int sum = 0;
    
    if (condition) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            /* Multiple basic blocks */
            if (i % 2 == 0) {
                for (int j = 0; j < 3; ++j) {  /* Small nested loop */
                    sum += i + j;
                }
            } else {
                sum += i * 2;
            }
            
            /* Early exit */
            if (sum > 5000) {
                return sum;
            }
        }
    } else {
        /* Different loop in false branch - completely disjoint */
        int k = 0;
        while (k < m) {  /* while loop for variety */
            sum += k * 3;
            k++;
            
            /* Nested if with continue */
            if (k % 4 == 0) {
                sum += 7;
                continue;
            }
            
            /* Another block */
            sum += 1;
        }
        
        /* Post-loop processing */
        for (int p = 0; p < 5; ++p) {
            sum += p;
        }
    }
    
    KEEP(sum);
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE int test_complex_nesting(int n, int m) {
    int sum = 0;
    
    /* Level 1 loop */
    for (int a = 0; a < n; ++a) {
        /* Level 2 loop */
        for (int b = 0; b < m; ++b) {
            /* Level 3 loop - creates deep nesting */
            for (int c = 0; c < 3; ++c) {
                sum += a + b + c;
                
                /* Multiple conditional exits */
                if (sum % 13 == 0) {
                    goto level2_continue;  /* Exit to level 2 */
                }
                
                if (sum % 17 == 0) {
                    continue;  /* Continue level 3 */
                }
            }
            
            /* Block between level 3 and level 2 */
            sum += b * 10;
            level2_continue:;
        }
        
        /* Conditional block in level 1 */
        if (a % 7 == 0) {
            /* Another small loop */
            for (int d = 0; d < 2; ++d) {
                sum += d * 100;
            }
        }
    }
    
    KEEP(sum);
    return sum;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Use seed to create variation in loop bounds */
    int n1 = (seed % 20) + 5;
    int n2 = (seed % 15) + 3;
    int n3 = (seed % 10) + 2;
    int n4 = (seed % 8) + 4;
    int n5 = (seed % 25) + 1;
    
    int total = 0;
    
    /* Call all test functions */
    total += test_nested_simple(n1, n2);
    total += test_nested_do_while(n2, n3);
    total += test_sequential_disjoint(n3, n4);
    total += test_switch_in_loop(n4, 2);  /* Fixed outer iterations */
    total += test_conditional_loops(n5, n3, seed % 2);
    total += test_complex_nesting(n2, n1);
    
    /* Ensure result is used */
    printf("Total checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
