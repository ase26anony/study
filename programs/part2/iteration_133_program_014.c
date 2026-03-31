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
        sum += i;
        /* Inner loop with multiple basic blocks due to if/continue */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += j * 2;
                continue;  /* Creates separate basic block for continue path */
            }
            sum += j;
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
        /* Shared conditional block - could be considered part of both loops' bitmaps */
        if (i % 3 == 0) {
            sum += 100;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            if (j < m/2) {
                sum += j * 3;
            } else {
                sum += j * 5;
                break;  /* Creates exit block */
            }
        }
        
        i++;
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
    for (int i = 0; i < n; ++i) {
        if (i < 50) {
            sum += arr1[i];
        } else {
            sum += arr1[i] * 2;
        }
    }
    
    /* Second loop - completely separate basic blocks */
    for (int j = 0; j < m; ++j) {
        if (j % 4 == 0) {
            sum -= arr2[j];
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
    for (int k = 0; k < outer_iter; ++k) {
        /* Inner loop with switch statement */
        for (int i = 0; i < n; ++i) {
            switch (i % 5) {
                case 0:
                    sum += i * 10;
                    break;
                case 1:
                    sum += i * 20;
                    /* Fall through */
                case 2:
                    sum += i * 30;
                    break;
                case 3:
                    sum += i * 40;
                    continue;  /* Continue to next iteration */
                case 4:
                    sum += i * 50;
                    break;
                default:
                    sum += i;
            }
            
            /* Additional block after switch */
            if (i % 3 == 0) {
                sum += 1000;
            }
        }
    }
    
    return sum;
}

/* Function E: Conditional loop nesting - two disjoint loops in different branches */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    volatile int sum = 0;
    
    if (flag) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            if (i % 2 == 0) {
                for (int j = 0; j < 3; ++j) {
                    sum += i + j;
                }
            } else {
                sum += i * 2;
            }
        }
    } else {
        /* Different loop in false branch - disjoint from first */
        int k = 0;
        while (k < m) {
            sum += k * k;
            if (k % 5 == 0) {
                k += 2;
                continue;
            }
            k++;
        }
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE int test_complex_nested(int n, int m) {
    volatile int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        if (i == 0) {
            /* Early continue creates separate block */
            sum += 1;
            continue;
        }
        
        /* Middle loop */
        for (int j = 0; j < m; ++j) {
            if (j == m - 1) {
                /* Break to outer loop */
                sum += 100;
                break;
            }
            
            /* Innermost loop */
            for (int k = 0; k < 3; ++k) {
                if (k == 1) {
                    /* Return from innermost position */
                    if (sum > 10000) return sum;
                    sum += k * 1000;
                } else {
                    sum += k;
                }
            }
            
            /* Conditional goto creating additional block */
            if (j % 7 == 0) {
                sum += 7;
                goto add_special;
            }
            
            sum += j;
            continue;
            
        add_special:
            sum += 777;
        }
        
        if (i % 11 == 0) {
            /* Early return from outer loop */
            return sum * 2;
        }
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    int total = 0;
    
    /* Use volatile iteration counts to prevent constant propagation */
    volatile int iter1 = 50 + (rand() % 50);
    volatile int iter2 = 30 + (rand() % 30);
    volatile int iter3 = 20 + (rand() % 20);
    volatile int iter4 = 10 + (rand() % 10);
    
    /* Call all test functions with different parameters */
    total += test_nested_simple(iter1, iter2);
    total += test_nested_shared_header(iter2, iter3);
    total += test_sequential_disjoint(iter3, iter4);
    total += test_switch_in_loop(iter4, 3);
    total += test_conditional_loops(iter1, iter2, rand() % 2);
    total += test_complex_nested(iter3, iter4);
    
    /* Use result to prevent dead code elimination */
    printf("Total checksum: %d\n", total);
    
    return total > 0 ? 0 : 1;
}
