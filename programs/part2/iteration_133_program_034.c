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
        /* Inner loop with multiple basic blocks */
        for (int j = 0; j < m; ++j) {
            if (j % 3 == 0) {
                sum += i * 2;  /* Creates separate basic block */
                continue;       /* Creates another block for continue path */
            }
            if (j % 5 == 0) {
                sum += j * 3;  /* Another block */
                break;         /* Creates exit block */
            }
            sum += i + j;      /* Default path */
        }
        /* Block after inner loop */
        if (i % 7 == 0) {
            sum -= 1;          /* Creates block in outer loop only */
        }
    }
    return sum;
}

/* Function B: Nested loops with shared header complexity */
NOINLINE int test_nested_shared(int n, int m) {
    volatile int sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared block before inner loop */
        int temp = i * 2;
        if (temp % 3 == 0) {
            sum += temp;       /* Block potentially shared if inner loop jumps here */
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += temp + j;
                if (j % 4 == 0) {
                    goto inner_done;  /* Creates complex control flow */
                }
            } else {
                sum -= j;
            }
        }
    inner_done:
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
        arr2[i] = 100 - i;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n && i < 100; ++i) {
        if (arr1[i] % 2 == 0) {
            sum += arr1[i] * 2;  /* Creates block in first loop only */
        } else {
            sum += arr1[i];      /* Another block in first loop */
        }
    }
    
    /* Intermediate code to ensure disjointness */
    int intermediate = sum % 7;
    
    /* Second loop - completely disjoint blocks */
    for (int j = 0; j < m && j < 100; ++j) {
        if (arr2[j] % 3 == 0) {
            sum += arr2[j] * 3;  /* Block in second loop only */
        } else if (arr2[j] % 5 == 0) {
            sum += arr2[j] * 5;  /* Another block in second loop */
        } else {
            sum -= arr2[j];      /* Default block in second loop */
        }
    }
    
    return sum + intermediate;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE int test_switch_nested(int n, int outer_iter) {
    volatile int sum = 0;
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < outer_iter; ++outer) {
        /* Loop with switch statement - creates many basic blocks */
        for (int i = 0; i < n; ++i) {
            switch (i % 6) {
                case 0:
                    sum += i * 2;
                    break;
                case 1:
                    sum += i * 3;
                    if (sum % 11 == 0) break;  /* Conditional break */
                    /* fall through */
                case 2:
                    sum += i * 5;
                    break;
                case 3:
                    sum += i * 7;
                    continue;  /* Skip to next iteration */
                case 4:
                    sum += i * 11;
                    if (i % 3 == 0) {
                        goto switch_done;  /* Early exit */
                    }
                    break;
                case 5:
                    sum += i * 13;
                    break;
                default:
                    sum += i;
            }
        switch_done:
            /* Block after switch but still in loop */
            if (i % 8 == 0) {
                sum -= 1;
            }
        }
        
        /* Block in outer loop only */
        if (outer % 4 == 0) {
            sum += 1000;
        }
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint loops) */
NOINLINE int test_conditional_disjoint(int n, int m, int flag) {
    volatile int sum = 0;
    
    if (flag > 0) {
        /* First loop in true branch */
        for (int i = 0; i < n; ++i) {
            if (i % 2 == 0) {
                sum += i * i;
                if (i % 6 == 0) {
                    continue;  /* Creates continue block */
                }
            } else {
                sum += i;
            }
            /* Another block in this loop */
            sum += 1;
        }
    } else {
        /* Second loop in false branch - completely disjoint */
        int j = 0;
        while (j < m) {
            sum += j * 3;
            j++;
            if (j % 4 == 0) {
                sum -= 5;  /* Creates block in second loop only */
                break;     /* Creates exit block */
            }
        }
    }
    
    return sum;
}

/* Function F: Complex nested loops with multiple exits */
NOINLINE int test_complex_nesting(int n, int m) {
    volatile int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* First inner loop */
        for (int j = 0; j < m; ++j) {
            sum += i + j;
            if (sum % 13 == 0) {
                return sum;  /* Early return from both loops */
            }
            if (j % 7 == 0) {
                goto next_outer;  /* Continue outer loop */
            }
        }
        
        /* Second inner loop at same nesting level */
        for (int k = 0; k < i && k < 10; ++k) {
            sum += k * k;
            if (k % 3 == 0) {
                continue;  /* Creates continue block */
            }
            sum += 1;
        }
        
    next_outer:
        /* Block after inner loops but in outer loop */
        if (i % 5 == 0) {
            sum += 100;
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
    volatile int iter1 = 10 + (rand() % 20);
    volatile int iter2 = 5 + (rand() % 15);
    volatile int iter3 = 8 + (rand() % 12);
    volatile int flag = rand() % 2;
    
    /* Force values into registers to prevent optimization */
    asm volatile("" : : "r"(iter1), "r"(iter2), "r"(iter3), "r"(flag));
    
    /* Call all test functions */
    total += test_nested_simple(iter1, iter2);
    total += test_nested_shared(iter2, iter3);
    total += test_sequential_disjoint(iter1, iter3);
    total += test_switch_nested(iter2, 2);  /* Fixed outer iterations */
    total += test_conditional_disjoint(iter1, iter3, flag);
    total += test_complex_nesting(iter3, iter1);
    
    /* Output result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}
