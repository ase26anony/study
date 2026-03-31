/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop having multiple basic blocks */
NOINLINE int test_nested_simple(int n, int m) {
    volatile int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with multiple basic blocks due to if-continue */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                /* This creates a separate basic block */
                sum += i * j;
                continue;  /* Creates another control flow edge */
            }
            /* Another basic block in the inner loop */
            sum -= i;
        }
        /* Basic block after inner loop but still in outer loop */
        if (i % 3 == 0) {
            sum += 1;
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
        /* Shared header block that both loops might intersect with */
        if (i % 2 == 0) {
            sum += 100;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            if (j < i) {
                sum += j * 2;
                /* Early exit from inner loop creates more blocks */
                if (sum > 1000) break;
            } else {
                sum -= j;
            }
        }
        
        i++;
        /* Conditional continue in outer loop */
        if (i % 5 == 0) continue;
        
        sum += 10;
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n, int m) {
    volatile int sum = 0;
    int arr1[100], arr2[100];
    
    /* Initialize arrays to prevent optimization */
    for (int k = 0; k < 100; ++k) {
        arr1[k] = k;
        arr2[k] = 100 - k;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n; ++i) {
        if (i % 4 == 0) {
            sum += arr1[i % 100] * 2;  /* Multiple basic blocks */
        } else {
            sum += arr1[i % 100];
        }
    }
    
    /* Intermediate code to ensure disjointness */
    int temp = sum % 100;
    
    /* Second loop - completely disjoint from first */
    for (int j = 0; j < m; ++j) {
        if (j % 3 == 0) {
            sum -= arr2[j % 100] * 3;  /* Multiple basic blocks */
        } else {
            sum -= arr2[j % 100];
        }
    }
    
    return sum + temp;
}

/* Function D: Loop with internal switch wrapped in outer loop */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    volatile int sum = 0;
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < outer_iter; ++outer) {
        /* Inner loop with switch statement */
        for (int i = 0; i < n; ++i) {
            switch (i % 5) {
                case 0:
                    sum += i * 2;
                    break;
                case 1:
                    sum += i * 3;
                    /* Fall through to next case */
                case 2:
                    sum += 10;
                    break;
                case 3:
                    sum -= i;
                    /* Conditional break creates more blocks */
                    if (sum < 0) break;
                    sum += 5;
                    break;
                case 4:
                    sum += 100;
                    /* Nested if for more blocks */
                    if (i % 2 == 0) {
                        sum += 50;
                    }
                    break;
                default:
                    sum += 1;
            }
            
            /* Additional block in the loop */
            if (i % 7 == 0) {
                sum += 7;
            }
        }
        
        /* Block after inner loop but still in outer loop */
        sum += outer * 1000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint loops in branches) */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    volatile int sum = 0;
    
    if (flag > 0) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            if (i % 2 == 0) {
                sum += i * i;
                continue;
            }
            sum -= i;
            /* Early return creates exit block */
            if (sum > 10000) return sum;
        }
        
        /* Additional code in true branch */
        for (int j = 0; j < 10; ++j) {
            sum += j;
        }
    } else {
        /* Different loop in false branch - disjoint from true branch loop */
        int k = 0;
        while (k < m) {
            sum += k * 3;
            k++;
            if (k % 4 == 0) {
                sum += 100;
                /* Nested loop inside while */
                for (int p = 0; p < 3; ++p) {
                    sum += p;
                }
            }
        }
        
        /* Another disjoint loop in false branch */
        for (int q = 0; q < 5; ++q) {
            sum -= q * 2;
        }
    }
    
    return sum;
}

/* Function F: Complex nested loops with multiple levels */
NOINLINE int test_multi_level_nested(int n, int m, int p) {
    volatile int sum = 0;
    
    /* Level 1: Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Level 2: Middle loop */
        for (int j = 0; j < m; ++j) {
            /* Shared block between middle and inner loops */
            int temp = i + j;
            
            /* Level 3: Inner loop */
            for (int k = 0; k < p; ++k) {
                if (k % 2 == 0) {
                    sum += temp * k;
                    /* Continue creates another block */
                    if (k % 3 == 0) continue;
                } else {
                    sum -= k;
                }
                
                /* Another block in innermost loop */
                sum += 1;
            }
            
            /* Block after inner loop but in middle loop */
            if (j % 2 == 0) {
                sum += 10;
            }
        }
        
        /* Block after middle loop but in outer loop */
        sum += i * 100;
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments or defaults for iteration counts */
    int n = (argc > 1) ? atoi(argv[1]) % 50 + 10 : 20;
    int m = (argc > 2) ? atoi(argv[2]) % 40 + 10 : 15;
    int p = (argc > 3) ? atoi(argv[3]) % 30 + 5 : 10;
    int flag = (argc > 4) ? atoi(argv[4]) % 2 : 1;
    
    /* Force variables into registers to prevent constant propagation */
    asm volatile("" : : "r"(n), "r"(m), "r"(p), "r"(flag));
    
    printf("Testing hardware loop patterns with n=%d, m=%d, p=%d, flag=%d\n", 
           n, m, p, flag);
    
    /* Run all test functions */
    result += test_nested_simple(n, m);
    printf("test_nested_simple: %d\n", result);
    
    result += test_nested_complex(n, m);
    printf("test_nested_complex: %d\n", result);
    
    result += test_sequential_disjoint(n, m);
    printf("test_sequential_disjoint: %d\n", result);
    
    result += test_switch_in_loop(n, 2);
    printf("test_switch_in_loop: %d\n", result);
    
    result += test_conditional_loops(n, m, flag);
    printf("test_conditional_loops: %d\n", result);
    
    result += test_multi_level_nested(n, m, p);
    printf("test_multi_level_nested: %d\n", result);
    
    printf("Final result: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    if (result == 0) {
        printf("Zero result - unlikely but possible\n");
    }
    
    return result != 0 ? 0 : 1;
}
