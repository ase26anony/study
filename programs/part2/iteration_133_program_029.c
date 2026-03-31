/* test_hwdoloop.c - Test program to cover hardware loop optimization bitmap intersection checks */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop having multiple basic blocks */
NOINLINE int test_nested_simple(int n, int m) {
    int sum = 0;
    volatile int vn = n; /* Prevent constant propagation */
    volatile int vm = m;
    
    /* Outer loop */
    for (int i = 0; i < vn; ++i) {
        /* Inner loop with multiple basic blocks due to if statement */
        for (int j = 0; j < vm; ++j) {
            if (j % 3 == 0) {
                sum += i * 2;
                continue; /* Creates separate basic block */
            } else if (j % 5 == 0) {
                sum += j * 3;
                break; /* Creates another basic block */
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

/* Function B: Nested loops with shared header/complex control flow */
NOINLINE int test_nested_complex(int n, int m) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared conditional block before inner loop */
        if (i % 2 == 0) {
            sum += 100;
        }
        
        /* Inner for loop */
        for (int j = 0; j < vm; ++j) {
            if (j < vm / 2) {
                sum += i * j;
                continue;
            } else {
                sum += j - i;
                if (j == vm - 1) {
                    break;
                }
            }
            sum += 1;
        }
        
        i++;
        if (i > vn * 2) {
            break; /* Multiple exit points */
        }
    } while (i < vn);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n) {
    int sum = 0;
    volatile int vn = n;
    int arr1[100], arr2[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = 100 - i;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < vn && i < 100; ++i) {
        if (arr1[i] % 4 == 0) {
            sum += arr1[i] * 2;
        } else {
            sum += arr1[i];
        }
    }
    
    /* Intermediate code to ensure disjointness */
    int temp = sum % 256;
    sum = temp * 3;
    
    /* Second loop - completely disjoint from first */
    for (int i = 0; i < vn && i < 100; ++i) {
        if (arr2[i] % 3 == 0) {
            sum -= arr2[i] / 2;
            continue;
        }
        sum += arr2[i];
    }
    
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE int test_loop_with_switch(int n, int outer_iter) {
    int sum = 0;
    volatile int vn = n;
    volatile int vouter = outer_iter;
    
    /* Outer wrapper loop */
    for (int k = 0; k < vouter; ++k) {
        /* Inner loop with switch statement */
        for (int i = 0; i < vn; ++i) {
            switch (i % 5) {
                case 0:
                    sum += i * 10;
                    break;
                case 1:
                    sum += i * 20;
                    if (i % 3 == 0) break;
                    /* fall through */
                case 2:
                    sum += i * 30;
                    continue; /* Skip to next iteration */
                case 3:
                    sum += i * 40;
                    /* Multiple basic blocks within case */
                    if (sum > 1000) {
                        sum -= 500;
                    }
                    break;
                default: /* case 4 */
                    sum += i * 50;
                    break;
            }
            
            /* Additional block after switch */
            sum += 1;
        }
        
        /* Block in outer loop but not in inner */
        sum += k * 1000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint loops in branches) */
NOINLINE int test_conditional_loops(int n, int flag) {
    int sum = 0;
    volatile int vn = n;
    
    if (flag > 0) {
        /* Loop in true branch */
        for (int i = 0; i < vn; ++i) {
            if (i % 2 == 0) {
                sum += i * 3;
                continue;
            }
            sum += i * 7;
            
            /* Nested loop inside true branch */
            for (int j = 0; j < 3; ++j) {
                sum += j;
                if (j == 1) break;
            }
        }
    } else {
        /* Different loop in false branch - disjoint from true branch loop */
        int i = vn;
        while (i-- > 0) {
            sum += i * 2;
            if (i % 4 == 0) {
                sum -= 5;
                continue;
            }
            
            /* Early return creates additional exit block */
            if (sum > 10000) {
                return sum;
            }
        }
        
        /* Another sequential loop in false branch */
        for (int k = 0; k < vn / 2; ++k) {
            sum += k * k;
        }
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE int test_complex_nesting(int n, int m) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    
    /* Level 1 loop */
    for (int a = 0; a < vn; ++a) {
        /* Level 2 loop */
        for (int b = 0; b < vm; ++b) {
            /* Level 3 loop - innermost */
            for (int c = 0; c < 5; ++c) {
                sum += a + b + c;
                
                /* Multiple exit points from innermost loop */
                if (sum > 1000) {
                    goto level2_continue; /* Exit to level 2 */
                }
                
                if (c == 3 && b == vm - 1) {
                    break; /* Normal break */
                }
            }
            
            /* Block in level 2 but not in level 3 */
            sum += b * 10;
            
        level2_continue:
            if (b % 7 == 0) {
                continue;
            }
        }
        
        /* Block in level 1 but not in level 2 or 3 */
        if (a % 11 == 0) {
            sum += 10000;
        } else {
            sum += 5000;
        }
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use command line arguments for variability, but provide defaults */
    int base_iter = (argc > 1) ? atoi(argv[1]) : 50;
    int flag = (argc > 2) ? atoi(argv[2]) : 1;
    
    /* Call all test functions with different parameters */
    total += test_nested_simple(base_iter, base_iter / 2);
    total += test_nested_complex(base_iter, base_iter / 3);
    total += test_sequential_disjoint(base_iter);
    total += test_loop_with_switch(base_iter, 2);
    total += test_conditional_loops(base_iter, flag);
    total += test_complex_nesting(base_iter / 4, base_iter / 5);
    
    /* Output result to prevent dead code elimination */
    printf("Total checksum: %d\n", total);
    
    return 0;
}
