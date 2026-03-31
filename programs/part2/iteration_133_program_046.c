/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Force variable to register to prevent constant propagation */
#define KEEP(var) asm volatile("" : : "r"(var))

/* Function A: Simple nested loops with inner loop continue */
NOINLINE int test_nested_simple(int n, int m) {
    int sum = 0;
    KEEP(n);
    KEEP(m);
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        sum += i;
        
        /* Inner loop with multiple basic blocks due to continue */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += j * 2;
                continue;  /* Creates separate basic block */
            }
            sum += j;
        }
        
        /* Additional block in outer loop */
        if (i % 3 == 0) {
            sum += 100;
        }
    }
    
    return sum;
}

/* Function B: Nested loops with shared header (do-while outer) */
NOINLINE int test_shared_header(int n, int m) {
    int sum = 0;
    int i = 0;
    KEEP(n);
    KEEP(m);
    
    /* Shared header block */
    if (n > 0 && m > 0) {
        sum += 5;
    }
    
    /* do-while outer loop */
    do {
        /* This block is shared by both loops conceptually */
        if (i < n/2) {
            sum += i * 2;
        }
        
        /* for inner loop */
        for (int j = 0; j < m; ++j) {
            if (j == m/2) {
                break;  /* Creates exit block */
            }
            sum += j;
            
            /* Nested if creates another block */
            if (j % 4 == 0) {
                sum += 1;
            }
        }
        
        i++;
        if (i % 5 == 0) {
            sum -= 10;  /* Another block in outer loop */
        }
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_disjoint_loops(int n, int m) {
    int sum = 0;
    int arr1[100], arr2[100];
    KEEP(n);
    KEEP(m);
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    /* First loop - disjoint body */
    for (int i = 0; i < n; ++i) {
        if (i < 50) {
            sum += arr1[i];
        } else {
            sum += arr1[i] * 2;  /* Different block */
        }
        
        /* Early return possibility */
        if (sum > 10000) {
            return sum;
        }
    }
    
    /* Intermediate code between loops */
    sum += 999;
    
    /* Second loop - completely disjoint blocks */
    for (int j = 0; j < m; ++j) {
        if (j % 3 == 0) {
            sum += arr2[j];
            continue;
        }
        sum += arr2[j] / 2;
    }
    
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE int test_switch_in_loop(int n, int m) {
    int sum = 0;
    KEEP(n);
    KEEP(m);
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < 2; ++outer) {
        sum += outer * 1000;
        
        /* Inner loop with switch */
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
                    sum += i * 4;
                    /* Complex block with nested if */
                    if (i > m) {
                        sum -= 5;
                    }
                    break;
                default:
                    sum += i * 5;
                    break;
            }
            
            /* Additional block after switch */
            if (i == n - 1) {
                sum += 100;
            }
        }
    }
    
    return sum;
}

/* Function E: Conditional loop nesting */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    int sum = 0;
    KEEP(n);
    KEEP(m);
    KEEP(flag);
    
    if (flag) {
        /* First loop in true branch */
        for (int i = 0; i < n; ++i) {
            sum += i * i;
            if (i % 7 == 0) {
                continue;  /* Creates block */
            }
            sum += 1;
        }
        
        /* Additional loop in same branch */
        for (int j = 0; j < m; ++j) {
            sum -= j;
        }
    } else {
        /* Different loop in false branch - disjoint */
        int k = 0;
        while (k < n) {
            sum += k * 3;
            k++;
            if (k == m) {
                break;  /* Exit block */
            }
        }
        
        /* Nested loop in false branch */
        for (int x = 0; x < m; ++x) {
            for (int y = 0; y < 3; ++y) {
                sum += x + y;
                if (x + y > 10) {
                    goto done;  /* Complex control flow */
                }
            }
        }
    done:
        sum += 500;
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE int test_complex_nesting(int n, int m) {
    int sum = 0;
    KEEP(n);
    KEEP(m);
    
    /* Outer loop with multiple exits */
    for (int i = 0; i < n; ++i) {
        if (i == 0) {
            sum += 10;
        }
        
        /* Middle loop */
        for (int j = 0; j < m; ++j) {
            if (j == 0) {
                continue;
            }
            
            /* Innermost loop */
            for (int k = 0; k < 5; ++k) {
                sum += i + j + k;
                
                /* Multiple exit points */
                if (sum > 1000) {
                    return sum;  /* Exit from innermost scope */
                }
                
                if (k == 3 && j == m - 1) {
                    goto next_iter;  /* Jump to outer loop */
                }
            }
            
            sum += j * 10;
        }
        
    next_iter:
        if (i == n - 1) {
            break;  /* Another exit */
        }
        sum += i * 100;
    }
    
    return sum;
}

/* Main driver */
int main(int argc, char **argv) {
    int result = 0;
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Vary iteration counts to create different block patterns */
    int n1 = 10 + (rand() % 20);
    int n2 = 15 + (rand() % 25);
    int n3 = 5 + (rand() % 15);
    int n4 = 20 + (rand() % 30);
    int n5 = 8 + (rand() % 12);
    int n6 = 12 + (rand() % 18);
    
    /* Call all test functions */
    result += test_nested_simple(n1, n2);
    result += test_shared_header(n2, n3);
    result += test_disjoint_loops(n3, n4);
    result += test_switch_in_loop(n4, n5);
    result += test_conditional_loops(n5, n6, seed % 2);
    result += test_complex_nesting(n6, n1);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional volatile write */
    volatile int dummy = result;
    
    return 0;
}
