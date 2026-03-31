/* test_hwdoloop.c - Test program to cover hardware loop optimization bitmap intersection checks */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Prevent loop unrolling */
#define NO_UNROLL __attribute__((optimize("no-unroll-loops")))

/* Force variable to stay in register */
#define KEEP_IN_REG(var) asm volatile("" : : "r"(var))

/* Function A: Simple nested loops with inner loop having multiple basic blocks */
NOINLINE NO_UNROLL
int test_nested_simple(int n, int m) {
    int sum = 0;
    KEEP_IN_REG(n);
    KEEP_IN_REG(m);
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with multiple basic blocks due to continue */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                /* Creates separate basic block */
                sum += i * 2;
                continue;  /* Creates another basic block for the continue path */
            }
            sum += j;
        }
        
        /* Additional block in outer loop */
        if (i % 3 == 0) {
            sum += 1;
        }
    }
    
    return sum;
}

/* Function B: Nested loops with shared header/initialization block */
NOINLINE NO_UNROLL
int test_nested_shared_header(int n, int m) {
    int sum = 0;
    int i = 0;
    KEEP_IN_REG(n);
    KEEP_IN_REG(m);
    
    /* do-while outer loop */
    do {
        /* Shared initialization block that both loops conceptually share */
        int temp = i * 2;
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            /* Multiple exit points from inner loop */
            if (j == m - 1) {
                break;  /* Creates separate basic block */
            }
            
            if (temp > 100) {
                return sum;  /* Early return creates another exit block */
            }
            
            sum += temp + j;
        }
        
        i++;
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE NO_UNROLL
int test_sequential_disjoint(int n, int m) {
    int sum = 0;
    int arr1[100];
    int arr2[100];
    KEEP_IN_REG(n);
    KEEP_IN_REG(m);
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            sum += arr1[i % 100];
        } else {
            sum -= arr1[i % 100];
        }
    }
    
    /* Some code between loops to ensure disjoint block sets */
    int intermediate = sum * 2;
    
    /* Second loop - completely disjoint blocks from first */
    for (int j = 0; j < m; ++j) {
        if (j % 3 == 0) {
            sum += arr2[j % 100] + intermediate;
        } else {
            sum += arr2[j % 100];
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE NO_UNROLL
int test_switch_in_loop(int n, int outer_iter) {
    int sum = 0;
    KEEP_IN_REG(n);
    KEEP_IN_REG(outer_iter);
    
    /* Outer wrapper loop */
    for (int k = 0; k < outer_iter; ++k) {
        /* Inner loop with switch creating multiple basic blocks */
        for (int i = 0; i < n; ++i) {
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
                    } else {
                        sum += 50;
                    }
                    break;
                case 4:
                    sum += i * 4;
                    /* Early continue */
                    continue;
                default:
                    sum += 1;
            }
            
            /* Additional code after switch */
            if (i % 7 == 0) {
                sum += 7;
            }
        }
        
        /* Code between outer loop iterations */
        sum += k * 10;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting - two disjoint loops in different branches */
NOINLINE NO_UNROLL
int test_conditional_loops(int n, int m, int flag) {
    int sum = 0;
    KEEP_IN_REG(n);
    KEEP_IN_REG(m);
    KEEP_IN_REG(flag);
    
    if (flag) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            /* Complex body with multiple blocks */
            for (int j = 0; j < 3; ++j) {
                sum += i + j;
                if (j == 1) {
                    break;
                }
            }
            
            if (i % 4 == 0) {
                goto skip_rest;  /* Creates another exit block */
            }
            sum += i * 2;
        }
    } else {
        /* Different loop in false branch - completely disjoint blocks */
        int i = m;
        while (i > 0) {
            sum += i;
            i--;
            
            /* Multiple continue points */
            if (i % 2 == 0) {
                continue;
            }
            
            if (i < m / 2) {
                break;
            }
        }
    }
    
skip_rest:
    return sum;
}

/* Function F: Complex nested structure with partial overlap */
NOINLINE NO_UNROLL
int test_partial_overlap(int n, int m) {
    int sum = 0;
    KEEP_IN_REG(n);
    KEEP_IN_REG(m);
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Middle loop - shares some but not all blocks with inner */
        int j = 0;
        while (j < m) {
            /* Shared block between middle and inner */
            int base = i + j;
            
            /* Inner loop - partially overlaps with middle */
            for (int k = 0; k < 3; ++k) {
                if (k == 1) {
                    sum += base + k;
                    /* This block is only in inner loop */
                    continue;
                }
                sum += base - k;
            }
            
            /* This block is only in middle loop */
            if (j % 2 == 0) {
                sum += 100;
            }
            
            j++;
        }
        
        /* Block only in outer loop */
        sum += i * 1000;
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use command line arguments for variability, but provide defaults */
    int n = argc > 1 ? atoi(argv[1]) % 100 + 10 : 50;
    int m = argc > 2 ? atoi(argv[2]) % 100 + 10 : 40;
    int outer = argc > 3 ? atoi(argv[3]) % 5 + 2 : 3;
    int flag = argc > 4 ? atoi(argv[4]) % 2 : 1;
    
    printf("Testing hardware loop patterns with n=%d, m=%d, outer=%d, flag=%d\n", 
           n, m, outer, flag);
    
    total += test_nested_simple(n, m);
    total += test_nested_shared_header(n, m);
    total += test_sequential_disjoint(n, m);
    total += test_switch_in_loop(n, outer);
    total += test_conditional_loops(n, m, flag);
    total += test_partial_overlap(n, m);
    
    /* Use result to prevent dead code elimination */
    printf("Total checksum: %d\n", total);
    
    /* Additional volatile write to ensure all loops execute */
    volatile int sink = total;
    
    return total != 0 ? 0 : 1;
}
