/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop having multiple blocks */
NOINLINE int test_nested_simple(int n, int m) {
    volatile int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with multiple basic blocks due to if/continue */
        for (int j = 0; j < m; ++j) {
            if (j % 3 == 0) {
                sum += i * 2;
                continue;  /* Creates separate basic block */
            }
            if (j % 5 == 0) {
                sum += i * 3;
                break;     /* Creates another basic block */
            }
            sum += i + j;
        }
        /* Additional block in outer loop */
        if (i % 2 == 0) {
            sum += 100;
        }
    }
    return sum;
}

/* Function B: Nested loops with shared header complexity */
NOINLINE int test_nested_shared_header(int n, int m) {
    volatile int sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared conditional block - could be considered part of both loops */
        if (i < n/2) {
            sum += 50;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            if (j % 4 == 0) {
                sum += i * j;
                continue;
            }
            sum += j;
        }
        
        i++;
        if (i % 3 == 0) {
            sum += 200;
        }
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n, int m) {
    volatile int sum = 0;
    int arr1[100], arr2[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n && i < 100; ++i) {
        if (arr1[i] % 2 == 0) {
            sum += arr1[i];
        } else {
            sum -= arr1[i];
        }
    }
    
    /* Intermediate code to ensure disjoint block sets */
    int temp = sum * 2;
    sum = temp / 2;
    
    /* Second loop - completely disjoint blocks */
    for (int j = 0; j < m && j < 100; ++j) {
        if (arr2[j] % 3 == 0) {
            sum += arr2[j] * 2;
        } else {
            sum += arr2[j];
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    volatile int sum = 0;
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < outer_iter; ++outer) {
        /* Inner loop with switch creating multiple blocks */
        for (int i = 0; i < n; ++i) {
            switch (i % 5) {
                case 0:
                    sum += i * 10;
                    break;
                case 1:
                    sum += i * 20;
                    if (i % 3 == 0) break;
                    sum += 5;
                    break;
                case 2:
                    sum += i * 30;
                    continue;  /* Skip to next iteration */
                case 3:
                    sum += i * 40;
                    /* Fall through */
                case 4:
                    sum += i * 50;
                    break;
                default:
                    sum += i;
            }
            
            /* Additional block after switch */
            if (i % 7 == 0) {
                sum += 1000;
            }
        }
        
        /* Block between inner loop iterations */
        sum += outer * 10000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting - disjoint loops in branches */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    volatile int sum = 0;
    
    if (flag) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            if (i % 2 == 0) {
                sum += i * 2;
                continue;
            }
            sum += i;
            if (i % 7 == 0) {
                sum += 77;
            }
        }
    } else {
        /* Different loop in false branch - disjoint blocks */
        int j = 0;
        while (j < m) {
            sum += j * 3;
            j++;
            if (j % 4 == 0) {
                sum += 44;
                break;
            }
        }
        
        /* Additional loop in else branch */
        for (int k = 0; k < n/2; ++k) {
            sum += k * k;
        }
    }
    
    return sum;
}

/* Function F: Complex nested loops with early exits */
NOINLINE int test_complex_nesting(int n, int m) {
    volatile int sum = 0;
    
    /* Three-level nesting */
    for (int i = 0; i < n; ++i) {
        if (i == n-1) {
            break;  /* Early exit creates separate block */
        }
        
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                /* Innermost loop */
                for (int k = 0; k < 5; ++k) {
                    sum += i + j + k;
                    if (k == 3) {
                        goto inner_exit;  /* Complicates control flow */
                    }
                }
                inner_exit:
                sum += 1;
            } else {
                sum += i * j;
            }
            
            if (sum > 1000000) {
                return sum;  /* Early return from middle of loop */
            }
        }
        
        sum += i * 100;
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    int total = 0;
    
    /* Use varying iteration counts to prevent constant folding */
    int n1 = 50 + (rand() % 50);
    int m1 = 30 + (rand() % 30);
    int n2 = 40 + (rand() % 40);
    int m2 = 20 + (rand() % 20);
    int n3 = 60 + (rand() % 40);
    int outer = 2 + (rand() % 3);
    int flag = rand() % 2;
    
    /* Call all test functions */
    total += test_nested_simple(n1, m1);
    total += test_nested_shared_header(n2, m2);
    total += test_sequential_disjoint(n1, m2);
    total += test_switch_in_loop(n3, outer);
    total += test_conditional_loops(n1, m1, flag);
    total += test_complex_nesting(n2, m2);
    
    /* Prevent dead code elimination */
    volatile int result = total;
    
    printf("Result: %d\n", result);
    return 0;
}
