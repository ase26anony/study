/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop having multiple basic blocks */
NOINLINE int test_nested_simple(int n, int m) {
    volatile int sum = 0;
    int i, j;
    
    /* Outer loop */
    for (i = 0; i < n; ++i) {
        sum += i;
        
        /* Inner loop with multiple basic blocks due to if/continue */
        for (j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                /* Creates separate basic block */
                sum += j * 2;
                continue;
            }
            /* Another basic block */
            sum += j;
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
    volatile int sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Block that might be shared in intersection analysis */
        int temp = i * 2;
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            /* Multiple exit points from inner loop */
            if (j == m/2 && i == n/2) {
                sum += 999;
                break;  /* Creates separate exit block */
            }
            
            if (j % 4 == 0) {
                continue;  /* Another basic block */
            }
            
            sum += temp + j;
        }
        
        /* Conditional return inside outer loop */
        if (sum > 1000000) {
            return sum;  /* Early exit creates separate block */
        }
        
        i++;
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n) {
    volatile int sum = 0;
    int arr1[100], arr2[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n && i < 100; ++i) {
        if (i % 2 == 0) {
            sum += arr1[i];
        } else {
            sum -= arr1[i];
        }
    }
    
    /* Intermediate code to ensure disjoint bitmaps */
    int intermediate = sum * 2;
    
    /* Second loop - completely disjoint from first */
    for (int i = 0; i < n && i < 100; ++i) {
        if (i % 3 == 0) {
            sum += arr2[i] + intermediate;
        } else {
            sum += arr2[i];
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE int test_loop_with_switch(int n, int outer_iter) {
    volatile int sum = 0;
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < outer_iter; ++outer) {
        
        /* Inner loop with switch statement */
        for (int i = 0; i < n; ++i) {
            /* Switch creates multiple basic blocks */
            switch (i % 5) {
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
                    /* Nested if inside case */
                    if (sum > 1000) {
                        sum -= 100;
                    }
                    sum += i * 4;
                    break;
                case 4:
                default:
                    sum += i * 5;
                    /* Conditional continue */
                    if (i == n-1) {
                        continue;
                    }
                    break;
            }
            
            /* Additional block after switch */
            sum += outer;
        }
        
        /* Block only in outer loop */
        if (outer % 2 == 0) {
            sum += 777;
        }
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint control flow paths) */
NOINLINE int test_conditional_loops(int n, int flag) {
    volatile int sum = 0;
    
    if (flag > 0) {
        /* First loop in true branch */
        for (int i = 0; i < n; ++i) {
            if (i % 2 == 0) {
                sum += i * i;
                /* Nested if creates more blocks */
                if (i > n/2) {
                    sum += 1000;
                }
            } else {
                sum += i;
            }
        }
        
        /* Additional code in true branch */
        sum += 1111;
    } else {
        /* Different loop in false branch - disjoint from true branch loop */
        int j = n;
        while (j-- > 0) {
            sum += j * 3;
            /* Multiple exit points */
            if (sum > 5000) {
                break;
            }
            if (j == n/3) {
                sum += 2222;
                continue;
            }
        }
        
        /* Additional code in false branch */
        sum += 3333;
    }
    
    return sum;
}

/* Function F: Deeply nested loops for complex hierarchy */
NOINLINE int test_deep_nesting(int n) {
    volatile int sum = 0;
    
    /* Level 1 */
    for (int i = 0; i < n; ++i) {
        sum += i;
        
        /* Level 2 */
        for (int j = 0; j < i; ++j) {
            sum += j;
            
            /* Level 3 */
            for (int k = 0; k < j; ++k) {
                /* Conditional with multiple blocks */
                if (k % 3 == 0) {
                    sum += k * 2;
                    if (sum > 10000) {
                        goto early_exit;  /* Creates unique exit block */
                    }
                } else if (k % 3 == 1) {
                    sum += k * 3;
                } else {
                    sum += k;
                }
            }
            
            /* Block only in level 2 */
            sum += 10;
        }
        
        /* Another level 2 loop (sibling) */
        for (int j = 0; j < 5; ++j) {
            sum += i * j;
        }
    }
    
early_exit:
    return sum;
}

/* Function G: Loop with irregular control flow and gotos */
NOINLINE int test_irregular_flow(int n) {
    volatile int sum = 0;
    int i = 0;
    
loop_start:
    if (i >= n) goto loop_end;
    
    sum += i;
    
    if (i % 4 == 0) {
        i++;
        goto loop_start;  /* Creates backedge block */
    }
    
    if (i % 5 == 0) {
        sum += 100;
        goto skip_increment;  /* Creates another block */
    }
    
    sum += i * 2;
    
skip_increment:
    i++;
    goto loop_start;
    
loop_end:
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use command line args for variability, but provide defaults */
    int n1 = (argc > 1) ? atoi(argv[1]) % 50 + 10 : 20;
    int n2 = (argc > 2) ? atoi(argv[2]) % 40 + 10 : 15;
    int n3 = (argc > 3) ? atoi(argv[3]) % 30 + 10 : 25;
    
    printf("Running hardware loop pattern tests...\n");
    
    /* Run all test functions */
    total += test_nested_simple(n1, n2);
    total += test_nested_complex(n2, n3);
    total += test_sequential_disjoint(n1);
    total += test_loop_with_switch(n3, 3);
    total += test_conditional_loops(n1, argc);
    total += test_deep_nesting(8);  /* Smaller for deep nesting */
    total += test_irregular_flow(n2);
    
    printf("Total checksum: %d\n", total);
    
    /* Prevent dead code elimination */
    volatile int result = total;
    
    return (result > 0) ? 0 : 1;
}
