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
        /* Inner loop with multiple basic blocks due to if/continue */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += i * j;
                continue;  /* Creates separate basic block */
            }
            sum += i + j;  /* Another basic block */
        }
        /* Additional block in outer loop */
        if (i % 3 == 0) {
            sum -= 1;
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
        if (i < n/2) {
            sum += 10;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            if (j % 4 == 0) {
                sum += i * j * 2;
                if (j == m-1) break;  /* Early exit creates another block */
            } else {
                sum += i + j + 1;
            }
        }
        
        i++;
        if (i % 5 == 0) {
            sum -= 5;  /* Another block in outer loop */
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
        arr2[i] = 100 - i;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n; ++i) {
        if (i < 50) {
            sum += arr1[i];  /* Multiple blocks */
        } else {
            sum += arr1[i] * 2;
        }
    }
    
    /* Intermediate code to ensure disjointness */
    int temp = sum % 100;
    
    /* Second loop - completely disjoint blocks */
    for (int j = 0; j < m; ++j) {
        if (j % 3 == 0) {
            sum += arr2[j];  /* Different blocks from first loop */
        } else {
            sum += arr2[j] / 2;
        }
    }
    
    return sum + temp;
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
                    sum += i * 1;
                    break;
                case 1:
                    sum += i * 2;
                    if (i == n-1) return sum;  /* Early return creates exit block */
                    break;
                case 2:
                    sum += i * 3;
                    continue;  /* Skip to next iteration */
                case 3:
                    sum += i * 4;
                    /* Fall through */
                case 4:
                    sum += i * 5;
                    break;
                default:
                    sum += 0;
            }
            
            /* Additional block after switch */
            if (sum > 1000) {
                sum %= 1000;
            }
        }
        
        /* Block between inner loop iterations */
        sum += k * 100;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting - two disjoint loops */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    volatile int sum = 0;
    
    if (flag) {
        /* First loop in true branch */
        for (int i = 0; i < n; ++i) {
            if (i % 2 == 0) {
                sum += i * i;
                continue;
            }
            sum += i;
            if (i == n-1) {
                sum += 1000;  /* Additional block */
            }
        }
    } else {
        /* Second loop in false branch - completely disjoint blocks */
        for (int j = 0; j < m; ++j) {
            switch (j % 3) {
                case 0: sum += j * 10; break;
                case 1: sum += j * 20; break;
                case 2: sum += j * 30; break;
            }
            
            if (j > m/2) {
                sum -= 5;  /* Another block */
            }
        }
    }
    
    return sum;
}

/* Function F: Complex nested loops with early exits */
NOINLINE int test_complex_nesting(int n, int m) {
    volatile int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Middle loop */
        for (int j = 0; j < m; ++j) {
            /* Innermost loop */
            for (int k = 0; k < 5; ++k) {
                if (k == 3) {
                    sum += i + j + k;
                    goto inner_exit;  /* Creates complex control flow */
                }
                sum += (i * j * k);
            }
            inner_exit:
            
            if (j % 2 == 0) {
                continue;  /* Skip to next j iteration */
            }
            sum += 1;
        }
        
        if (i == n-1) {
            break;  /* Early exit from outer loop */
        }
        sum += 100;
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
    volatile int iter2 = 30 + (rand() % 40);
    volatile int iter3 = 20 + (rand() % 30);
    
    /* Test all patterns */
    total += test_nested_simple(iter1, iter2);
    total += test_nested_shared_header(iter2, iter3);
    total += test_sequential_disjoint(iter1, iter2);
    total += test_switch_in_loop(iter3, 2);
    total += test_conditional_loops(iter1, iter2, rand() % 2);
    total += test_complex_nesting(iter1 / 2, iter2 / 2);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(total));
    
    printf("Result: %d\n", total % 1000);
    return 0;
}
