/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and aggressive optimization */
#define NOINLINE_COLD __attribute__((noinline, cold))

/* Force variable to stay in register */
#define KEEP(var) asm volatile("" : : "r"(var))

/* Function A: Simple nested loops with inner loop continue */
NOINLINE_COLD
int test_nested_simple(int n, int m) {
    int sum = 0;
    KEEP(n);
    KEEP(m);
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with multiple basic blocks due to continue */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += i * 2;
                continue;  /* Creates separate basic block */
            }
            sum += j;
        }
        
        /* Additional block in outer loop */
        if (i % 3 == 0) {
            sum -= 1;
        }
    }
    return sum;
}

/* Function B: Nested loops with shared header (do-while outer) */
NOINLINE_COLD
int test_nested_shared_header(int n, int m) {
    int sum = 0;
    int i = 0;
    KEEP(n);
    KEEP(m);
    
    /* Shared header block */
    if (n > 0 && m > 0) {
        sum = 1;
    }
    
    /* do-while outer loop */
    do {
        /* for inner loop */
        for (int j = 0; j < m; ++j) {
            /* Multiple exit points from inner loop */
            if (sum > 1000) {
                return sum;  /* Early return creates exit block */
            }
            if (j == m - 1) {
                break;  /* Break creates another exit block */
            }
            sum += i + j;
        }
        
        /* Conditional block that could be shared */
        if (i % 2 == 0 && n > 10) {
            sum *= 2;
        }
        
        i++;
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE_COLD
int test_disjoint_loops(int n, int m) {
    int sum = 0;
    int arr1[100], arr2[100];
    KEEP(n);
    KEEP(m);
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = 100 - i;
    }
    
    /* First loop - disjoint body */
    for (int i = 0; i < n && i < 100; ++i) {
        if (arr1[i] % 3 == 0) {  /* Creates multiple blocks */
            sum += arr1[i] * 2;
        } else {
            sum += arr1[i];
        }
    }
    
    /* Intermediate code to ensure disjointness */
    int temp = sum % 7;
    KEEP(temp);
    
    /* Second loop - completely disjoint from first */
    for (int j = 0; j < m && j < 100; ++j) {
        if (arr2[j] > 50) {  /* Creates multiple blocks */
            sum -= arr2[j];
        } else {
            sum += arr2[j] / 2;
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE_COLD
int test_switch_in_loop(int n, int m) {
    int sum = 0;
    KEEP(n);
    KEEP(m);
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < 2; ++outer) {
        /* Inner loop with switch statement */
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
                    if (sum > 100) {
                        sum -= 50;
                    }
                    break;
                case 4:
                    sum += i * 4;
                    /* Early continue */
                    if (i == n - 1) {
                        continue;
                    }
                    break;
                default:
                    sum += 1;
            }
            
            /* Additional block after switch */
            if (i % 10 == 0) {
                sum += outer;
            }
        }
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint control flow) */
NOINLINE_COLD
int test_conditional_loops(int n, int m, int flag) {
    int sum = 0;
    KEEP(n);
    KEEP(m);
    KEEP(flag);
    
    if (flag > 0) {
        /* First loop in true branch */
        for (int i = 0; i < n; ++i) {
            /* Multiple basic blocks */
            if (i % 2 == 0) {
                sum += i * i;
                goto skip_odd;  /* goto creates another block */
            }
            sum += i;
            skip_odd:
            if (i == n - 1) {
                sum += 1000;
            }
        }
    } else {
        /* Second loop in false branch - disjoint from first */
        for (int j = 0; j < m; ++j) {
            /* Loop with multiple exits */
            if (sum < -10000) {
                return -1;  /* Early return */
            }
            if (j % 3 == 0) {
                continue;  /* Continue creates separate block */
            }
            sum -= j * j;
        }
        
        /* Additional loop at same level */
        for (int k = 0; k < m / 2; ++k) {
            sum += k * 3;
        }
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple levels */
NOINLINE_COLD
int test_multi_level_nesting(int n, int m, int p) {
    int sum = 0;
    KEEP(n);
    KEEP(m);
    KEEP(p);
    
    /* Level 1: outermost loop */
    for (int i = 0; i < n; ++i) {
        /* Level 2: middle loop */
        for (int j = 0; j < m; ++j) {
            /* Level 3: innermost loop */
            for (int k = 0; k < p; ++k) {
                /* Complex body with multiple blocks */
                if (k % 2 == 0) {
                    if (j % 2 == 0) {
                        sum += i + j + k;
                    } else {
                        sum += i * j * k;
                    }
                } else {
                    sum -= 1;
                }
                
                /* Early break from innermost */
                if (sum > 1000000) {
                    break;
                }
            }
            
            /* Conditional between middle and inner */
            if (j == m / 2) {
                sum += 777;
            }
        }
        
        /* Another inner loop at level 2 (sibling to the previous) */
        for (int j2 = 0; j2 < i && j2 < 10; ++j2) {
            sum += j2 * 11;
        }
    }
    
    return sum;
}

/* Main driver */
int main(int argc, char *argv[]) {
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
    result += test_nested_shared_header(n2, n3);
    result += test_disjoint_loops(n3, n4);
    result += test_switch_in_loop(n4, n5);
    result += test_conditional_loops(n5, n6, seed % 2);
    result += test_multi_level_nesting(n6, n1, n2);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
