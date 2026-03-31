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
        KEEP(i);
        /* Inner loop with multiple basic blocks due to continue */
        for (int j = 0; j < m; ++j) {
            KEEP(j);
            if (j % 2 == 0) {
                sum += i * j;
                continue;  /* Creates separate basic block */
            }
            sum += i + j;  /* Another basic block */
        }
        /* Additional block after inner loop */
        if (i % 3 == 0) {
            sum -= 1;
        }
    }
    return sum;
}

/* Function B: Nested loops with shared header (do-while outer) */
NOINLINE int test_nested_shared(int n, int m) {
    int sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        KEEP(i);
        /* Shared conditional block - could be part of both loops' bitmaps */
        if (i < n/2) {
            sum += i * 2;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            KEEP(j);
            /* Complex inner body with early exit */
            if (j == m-1) {
                break;  /* Creates exit block */
            }
            sum += (i * j) % 7;
            
            /* Nested if for more blocks */
            if (sum > 1000) {
                sum = sum % 1000;
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
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = 100 - i;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n; ++i) {
        KEEP(i);
        if (i % 4 == 0) {
            sum += arr1[i % 100] * 2;  /* Basic block A */
        } else {
            sum += arr1[i % 100] / 2;  /* Basic block B */
        }
        
        /* Early return possibility */
        if (sum > 10000) {
            return sum;  /* Creates exit edge */
        }
    }
    
    /* Completely separate second loop */
    for (int j = 0; j < m; ++j) {
        KEEP(j);
        /* Different array, different computation */
        sum -= arr2[j % 100];
        
        /* Nested condition for block complexity */
        if (j % 5 == 0) {
            sum += 1;
            if (sum < 0) {
                sum = 0;  /* Another nested block */
            }
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    int sum = 0;
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < outer_iter; ++outer) {
        KEEP(outer);
        
        /* Inner loop with switch statement */
        for (int i = 0; i < n; ++i) {
            KEEP(i);
            
            /* Switch creates multiple basic blocks */
            switch (i % 5) {
                case 0:
                    sum += i * 2;
                    break;
                case 1:
                    sum += i + 10;
                    /* Fall through */
                case 2:
                    sum += i * i;
                    break;
                case 3:
                    sum -= i;
                    /* Complex case with nested if */
                    if (sum < 0) {
                        sum = -sum;
                        break;
                    }
                    /* Continue to default */
                default:
                    sum += 1;
                    /* Nested loop inside switch case */
                    for (int k = 0; k < 2; ++k) {
                        sum += k;
                    }
                    break;
            }
            
            /* Additional condition after switch */
            if (i == n-1) {
                sum += 1000;
            }
        }
        
        /* Outer loop body after inner loop */
        sum = sum % 5000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint control flow) */
NOINLINE int test_conditional_loops(int n, int flag) {
    int sum = 0;
    
    if (flag > 0) {
        /* First loop in true branch */
        for (int i = 0; i < n; ++i) {
            KEEP(i);
            sum += i * 3;
            
            /* Nested if with continue */
            if (i % 7 == 0) {
                continue;
            }
            sum += 1;
            
            /* Early break */
            if (sum > 5000) {
                break;
            }
        }
        
        /* Additional computation */
        sum = sum * 2;
    } else {
        /* Different loop in false branch - disjoint from first */
        int j = n;
        while (j > 0) {
            KEEP(j);
            sum += j * 2;
            j--;
            
            /* Multiple exit points */
            if (sum < -1000) {
                return sum;  /* Early return */
            }
            
            /* Nested condition */
            if (j % 3 == 0) {
                sum += 5;
                if (j % 6 == 0) {
                    sum += 10;  /* Another block */
                }
            }
        }
        
        /* Post-loop computation */
        sum = sum / 2;
    }
    
    return sum;
}

/* Function F: Complex nested loops with multiple levels */
NOINLINE int test_multi_level_nesting(int n, int m, int p) {
    int sum = 0;
    
    /* Level 1: Outer loop */
    for (int i = 0; i < n; ++i) {
        KEEP(i);
        
        /* Level 2: Middle loop */
        for (int j = 0; j < m; ++j) {
            KEEP(j);
            
            /* Shared conditional - part of both middle and inner loops */
            if (j % 2 == 0) {
                sum += i + j;
            }
            
            /* Level 3: Innermost loop */
            for (int k = 0; k < p; ++k) {
                KEEP(k);
                
                /* Complex innermost body */
                if (k % 3 == 0) {
                    sum += i * j * k;
                    continue;
                }
                
                if (k % 5 == 0) {
                    sum -= (i + j);
                    break;  /* Exits innermost only */
                }
                
                sum += 1;
            }
            
            /* Middle loop continuation */
            if (sum > 10000) {
                sum = 10000;
            }
        }
        
        /* Outer loop tail */
        sum = sum % 7777;
    }
    
    return sum;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use argc for variability, but keep values small for fast execution */
    int n = (argc > 1) ? atoi(argv[1]) % 50 + 10 : 20;
    int m = (argc > 2) ? atoi(argv[2]) % 40 + 8 : 15;
    int p = (argc > 3) ? atoi(argv[3]) % 30 + 5 : 10;
    int flag = (argc > 4) ? atoi(argv[4]) % 2 : 1;
    int outer_iter = (argc > 5) ? atoi(argv[5]) % 5 + 2 : 3;
    
    /* Call all test functions */
    total += test_nested_simple(n, m);
    total += test_nested_shared(n, m);
    total += test_sequential_disjoint(n, m);
    total += test_switch_in_loop(n, outer_iter);
    total += test_conditional_loops(n, flag);
    total += test_multi_level_nesting(n, m, p);
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %d\n", total);
    
    /* Additional volatile store to ensure all computations are kept */
    volatile int result = total;
    (void)result;
    
    return 0;
}
