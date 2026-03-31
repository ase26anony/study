/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Force variable to register to prevent constant propagation */
#define KEEP(var) asm volatile("" : : "r"(var))

/* Function A: Simple nested loops with inner conditional */
NOINLINE int test_nested_simple(int n, int m) {
    int sum = 0;
    KEEP(n); KEEP(m);
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        sum += i;
        
        /* Inner loop with multiple basic blocks */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += j * 2;
                continue;  /* Creates separate basic block */
            }
            sum += j;
            
            if (j == m - 1) {
                sum += 100;  /* Another basic block */
            }
        }
        
        /* Basic block after inner loop */
        sum += i * 10;
    }
    
    return sum;
}

/* Function B: Nested loops with shared header complexity */
NOINLINE int test_nested_shared(int n, int m) {
    int sum = 0;
    int i = 0;
    KEEP(n); KEEP(m);
    
    /* do-while outer loop */
    do {
        /* Shared conditional block - could be in both loops' bitmaps */
        if (i % 3 == 0) {
            sum += 7;
        }
        
        /* for inner loop */
        for (int j = 0; j < m; ++j) {
            sum += i * j;
            
            /* Early exit from inner loop creates another block */
            if (j > m / 2) {
                sum += 50;
                break;
            }
            
            sum += j;
        }
        
        i++;
        /* Complex condition with multiple basic blocks */
        if (i < n) {
            sum += 1;
        } else {
            sum += 2;
        }
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n, int m) {
    int sum = 0;
    int arr1[100], arr2[100];
    KEEP(n); KEEP(m);
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = 100 - i;
    }
    
    /* First loop - processes arr1 */
    for (int i = 0; i < n && i < 100; ++i) {
        if (arr1[i] % 4 == 0) {  /* Creates multiple blocks */
            sum += arr1[i] * 3;
            continue;
        }
        sum += arr1[i];
    }
    
    /* Intermediate code to ensure disjointness */
    sum += 999;
    
    /* Second loop - processes arr2, completely disjoint */
    for (int j = 0; j < m && j < 100; ++j) {
        if (arr2[j] % 5 == 0) {
            sum += arr2[j] * 2;
            /* No continue here - different block structure */
        } else {
            sum += arr2[j];
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE int test_switch_nested(int n, int outer_iter) {
    int sum = 0;
    KEEP(n); KEEP(outer_iter);
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < outer_iter; ++outer) {
        sum += outer * 100;
        
        /* Inner loop with switch */
        for (int i = 0; i < n; ++i) {
            /* Switch creates many basic blocks */
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
                    /* Complex case with its own if */
                    if (i > n / 2) {
                        sum += 1000;
                    }
                    break;
                case 4:
                    sum += i * 5;
                    break;
                default:
                    sum += 9999;
            }
            
            /* Additional block after switch */
            if (i % 3 == 0) {
                sum += 7;
            }
        }
        
        /* Block after inner loop */
        sum += outer * 1000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint loops) */
NOINLINE int test_conditional_disjoint(int n, int m, int flag) {
    int sum = 0;
    KEEP(n); KEEP(m); KEEP(flag);
    
    if (flag > 0) {
        /* First loop in true branch */
        for (int i = 0; i < n; ++i) {
            sum += i * i;
            if (i % 7 == 0) {
                sum += 77;
                continue;
            }
            sum += i;
        }
        
        /* Additional code to ensure no intersection with else branch */
        sum += 1111;
    } else {
        /* Different loop in false branch */
        int k = 0;
        while (k < m) {  /* while loop for variety */
            sum += k * 3;
            k++;
            
            if (k == m / 2) {
                sum += 500;
                /* Early return creates exit block */
                if (m > 20) return sum;
            }
        }
        
        /* Different block structure */
        sum += 2222;
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE int test_complex_multi_exit(int n, int m) {
    int sum = 0;
    KEEP(n); KEEP(m);
    
    /* Outer loop with multiple exit points */
    for (int i = 0; i < n; ++i) {
        if (i == 0) {
            sum += 1;
            continue;
        }
        
        if (i > n * 3 / 4) {
            /* Early return from function */
            return sum + 999;
        }
        
        /* Middle loop */
        for (int j = 0; j < m; ++j) {
            sum += i * j;
            
            if (j == m - 1) {
                /* goto to outer loop's exit block */
                sum += 100;
                goto inner_done;
            }
            
            if (j % 2 == 0) {
                sum += j;
                continue;
            }
        }
        
        inner_done:
        if (i % 10 == 0) {
            sum += 1000;
        }
    }
    
    return sum;
}

/* Main driver */
int main(int argc, char *argv[]) {
    int total = 0;
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Vary iteration counts to create different block patterns */
    int n1 = 10 + (rand() % 20);
    int m1 = 5 + (rand() % 15);
    int n2 = 8 + (rand() % 12);
    int m2 = 6 + (rand() % 10);
    int outer_iter = 2 + (rand() % 3);
    int flag = rand() % 2;
    
    printf("Running hardware loop test patterns...\n");
    
    total += test_nested_simple(n1, m1);
    total += test_nested_shared(n2, m2);
    total += test_sequential_disjoint(n1, m2);
    total += test_switch_nested(n1, outer_iter);
    total += test_conditional_disjoint(n2, m1, flag);
    total += test_complex_multi_exit(n1, m1);
    
    /* Use result to prevent optimization */
    volatile int result = total;
    printf("Total checksum: %d\n", result);
    
    return 0;
}
