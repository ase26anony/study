/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE_COLD __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop continue */
NOINLINE_COLD
int test_nested_simple(int n, int m) {
    volatile int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with multiple basic blocks due to continue */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += i * j;
                continue;  /* Creates separate basic block */
            }
            sum -= i + j;
        }
        /* Additional block in outer loop */
        if (i % 3 == 0) {
            sum += 100;
        }
    }
    return sum;
}

/* Function B: Nested loops with shared header (do-while outer) */
NOINLINE_COLD
int test_nested_shared_header(int n, int m) {
    volatile int sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared conditional block - could be considered part of both loops */
        if (i < n/2) {
            sum += i * 10;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            if (j == m/2) {
                sum += 50;
                break;  /* Creates exit block */
            }
            sum += i + j;
        }
        
        i++;
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE_COLD
int test_sequential_disjoint(int n, int m) {
    volatile int sum = 0;
    int arr1[100], arr2[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = 100 - i;
    }
    
    /* First loop - processes arr1 */
    for (int i = 0; i < n && i < 100; ++i) {
        if (arr1[i] % 3 == 0) {
            sum += arr1[i] * 2;
        } else {
            sum += arr1[i];
        }
    }
    
    /* Intermediate code to ensure disjoint blocks */
    int temp = sum * 2;
    
    /* Second loop - processes arr2 (disjoint from first) */
    for (int i = 0; i < m && i < 100; ++i) {
        if (arr2[i] % 4 == 0) {
            sum -= arr2[i];
        } else {
            sum += arr2[i] / 2;
        }
    }
    
    return sum + temp;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE_COLD
int test_switch_in_loop(int n, int outer_iter) {
    volatile int sum = 0;
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < outer_iter; ++outer) {
        /* Inner loop with switch creating multiple basic blocks */
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
                    /* Multiple statements in case */
                    if (sum > 1000) {
                        sum -= 500;
                    }
                    break;
                case 4:
                    sum += i * 50;
                    /* Early continue creates another block */
                    if (i == n-1) continue;
                    sum += 1;
                    break;
                default:
                    sum += i;
            }
            
            /* Additional block after switch */
            if (i % 7 == 0) {
                sum += 7;
            }
        }
        
        /* Block between inner loop iterations */
        sum += outer * 1000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint control flow) */
NOINLINE_COLD
int test_conditional_loops(int n, int m, int condition) {
    volatile int sum = 0;
    
    if (condition > 0) {
        /* First loop in true branch */
        for (int i = 0; i < n; ++i) {
            if (i % 2 == 0) {
                sum += i * i;
                /* Multiple returns create different exit blocks */
                if (sum > 10000) return sum;
            } else {
                sum -= i;
            }
        }
        
        /* Additional code in true branch */
        for (int j = 0; j < 10; ++j) {
            sum += j;
        }
    } else {
        /* Second loop in false branch (disjoint from first) */
        int k = 0;
        while (k < m) {
            sum += k * 3;
            k++;
            if (k == m/2) {
                /* goto creates another exit path */
                goto early_exit;
            }
        }
        
        early_exit:
        sum += 1000;
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE_COLD
int test_complex_nesting(int n, int m) {
    volatile int sum = 0;
    int i = 0, j = 0;
    
    /* Outer loop with multiple exit points */
    for (i = 0; i < n; ++i) {
        if (i == n-1) {
            /* Early return creates exit block */
            return sum + 999;
        }
        
        /* Middle loop */
        for (j = 0; j < m; ++j) {
            if (j == m-1) {
                sum += 100;
                break;  /* Exit middle loop */
            }
            
            /* Innermost loop */
            for (int k = 0; k < 5; ++k) {
                sum += i + j + k;
                if (sum > 5000) {
                    /* goto to outer scope */
                    goto outer_break;
                }
            }
            
            /* Conditional continue in middle loop */
            if (j % 3 == 0) continue;
            sum += 10;
        }
        
        /* Label for goto target */
        outer_break:
        if (i == n/2) break;
    }
    
    return sum;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create some runtime variability */
    int base = (argc > 1) ? atoi(argv[1]) : 10;
    if (base <= 0) base = 10;
    
    /* Force variables into registers to prevent constant propagation */
    int n1, n2, n3, n4, n5, n6;
    asm volatile("" : "=r"(n1) : "0"(base));
    asm volatile("" : "=r"(n2) : "0"(base + 1));
    asm volatile("" : "=r"(n3) : "0"(base + 2));
    asm volatile("" : "=r"(n4) : "0"(base + 3));
    asm volatile("" : "=r"(n5) : "0"(base + 4));
    asm volatile("" : "=r"(n6) : "0"(base + 5));
    
    /* Call all test functions with different parameters */
    result += test_nested_simple(n1, n2);
    result += test_nested_shared_header(n2, n3);
    result += test_sequential_disjoint(n3, n4);
    result += test_switch_in_loop(n4, 2);  /* Fixed outer iterations */
    result += test_conditional_loops(n5, n6, argc);
    result += test_complex_nesting(n6, n1);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
