/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Force variable to stay in register */
#define KEEP(var) asm volatile("" : : "r"(var))

/* Function A: Simple nested loops with inner conditional */
NOINLINE int test_nested_simple(int n, int m) {
    int sum = 0;
    KEEP(n); KEEP(m);
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        sum += i;
        
        /* Inner loop with conditional continue creating multiple blocks */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += j * 2;
                continue;  /* Creates separate basic block */
            }
            sum += j;
        }
        
        /* Another block in outer loop */
        if (i % 3 == 0) {
            sum += 100;
        }
    }
    
    return sum;
}

/* Function B: Nested loops with shared header complexity */
NOINLINE int test_nested_shared_header(int n, int m) {
    int sum = 0;
    int i = 0;
    KEEP(n); KEEP(m);
    
    /* do-while outer loop */
    do {
        /* Shared conditional block - could be considered part of both loops */
        if (i < n/2) {
            sum += i * 3;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            sum += (i + j);
            if (j == m-1) {
                break;  /* Creates exit block */
            }
        }
        
        i++;
        /* Multiple conditions in while */
    } while (i < n && sum < 1000000);
    
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
        arr2[i] = i * 2;
    }
    
    /* First loop - processes arr1 */
    for (int i = 0; i < n && i < 100; ++i) {
        if (arr1[i] % 4 == 0) {  /* Creates multiple blocks */
            sum += arr1[i] * 2;
        } else {
            sum += arr1[i];
        }
    }
    
    /* Intermediate code to ensure disjointness */
    int temp = sum % 256;
    sum += temp;
    
    /* Second loop - processes arr2, completely disjoint from first */
    for (int j = 0; j < m && j < 100; ++j) {
        if (arr2[j] > 50) {  /* Creates multiple blocks */
            sum -= arr2[j] / 2;
        } else {
            sum += arr2[j];
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    int sum = 0;
    KEEP(n); KEEP(outer_iter);
    
    /* Outer wrapper loop */
    for (int k = 0; k < outer_iter; ++k) {
        /* Inner loop with switch */
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
                    if (sum > 1000) {
                        sum -= 50;  /* Conditional creates new block */
                    }
                    break;
                case 4:
                    sum += i * 4;
                    /* Multiple statements in case */
                    for (int j = 0; j < 3; ++j) {
                        sum += j;
                    }
                    break;
                default:
                    sum += 1;
            }
            
            /* Additional block in loop */
            if (i == n-1) {
                sum += 999;
            }
        }
        
        /* Block in outer loop but not in inner */
        sum += k * 1000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint control flow) */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    int sum = 0;
    KEEP(n); KEEP(m); KEEP(flag);
    
    if (flag > 0) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            sum += i * i;
            if (i % 7 == 0) {
                continue;  /* Creates separate continue block */
            }
            sum += 1;
        }
        
        /* Additional code in true branch */
        sum += 10000;
    } else {
        /* Different loop in false branch - disjoint from first */
        int j = m;
        while (j-- > 0) {
            sum -= j;
            if (sum < 0) {
                sum = 0;  /* Creates conditional block */
            }
        }
        
        /* Additional code in false branch */
        sum += 20000;
    }
    
    /* Common code after if-else */
    for (int k = 0; k < 5; ++k) {
        sum += k;
    }
    
    return sum;
}

/* Function F: Complex nested structure with early returns */
NOINLINE int test_complex_nesting(int n, int m) {
    int sum = 0;
    KEEP(n); KEEP(m);
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        if (i == 0) {
            /* Special handling for first iteration */
            sum += 1000;
        }
        
        /* Middle loop */
        for (int j = 0; j < m; ++j) {
            /* Innermost loop */
            for (int k = 0; k < 3; ++k) {
                sum += (i + j + k);
                if (sum > 100000) {
                    return sum;  /* Early return from innermost */
                }
            }
            
            if (j % 2 == 0) {
                continue;  /* Continue to next j iteration */
            }
            
            sum += j * 10;
        }
        
        if (i % 3 == 0) {
            sum += i * 100;
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
    int m1 = 5 + (rand() % 15);
    int n2 = 8 + (rand() % 12);
    int m2 = 6 + (rand() % 10);
    int outer_iter = 2 + (rand() % 3);
    int flag = rand() % 2;
    
    printf("Testing hardware loop patterns...\n");
    
    result += test_nested_simple(n1, m1);
    printf("test_nested_simple: %d\n", result);
    
    result += test_nested_shared_header(n2, m2);
    printf("test_nested_shared_header: %d\n", result);
    
    result += test_sequential_disjoint(n1, m1);
    printf("test_sequential_disjoint: %d\n", result);
    
    result += test_switch_in_loop(n2, outer_iter);
    printf("test_switch_in_loop: %d\n", result);
    
    result += test_conditional_loops(n1, m2, flag);
    printf("test_conditional_loops: %d\n", result);
    
    result += test_complex_nesting(n2, m1);
    printf("test_complex_nesting: %d\n", result);
    
    printf("Final result: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    volatile int dummy = result;
    (void)dummy;
    
    return 0;
}
