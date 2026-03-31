/* test_hwdoloop.c - Test program to cover bitmap intersection checks in GCC's hw-doloop optimization */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop having multiple basic blocks */
NOINLINE int func_nested_simple(int n, int m) {
    volatile int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        sum += i;
        /* Inner loop with multiple basic blocks due to if-else */
        for (int j = 0; j < m; ++j) {
            if (j % 3 == 0) {
                sum += j * 2;
                continue;  /* Creates separate basic block */
            } else if (j % 5 == 0) {
                sum -= j;
                break;     /* Creates exit block */
            } else {
                sum += j;
            }
        }
    }
    return sum;
}

/* Function B: Nested loops with shared header/complex control flow */
NOINLINE int func_nested_complex(int n, int m) {
    volatile int sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared block before inner loop */
        if (i % 2 == 0) {
            sum += 100;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            sum += i * j;
            if (sum > 1000) {
                sum -= 500;  /* Creates another basic block */
            }
        }
        
        i++;
        if (i > n) break;  /* Additional exit condition */
    } while (1);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int func_sequential_disjoint(int n, int m) {
    volatile int sum = 0;
    int arr1[100], arr2[100];
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n && i < 100; ++i) {
        arr1[i] = i * 2;
        if (i % 4 == 0) {
            sum += arr1[i];  /* Conditional creates extra block */
        } else {
            sum -= arr1[i];
        }
    }
    
    /* Second loop - completely separate basic blocks */
    for (int j = 0; j < m && j < 100; ++j) {
        arr2[j] = j * 3;
        if (j % 3 == 0) {
            sum += arr2[j] * 2;
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE int func_switch_in_loop(int n, int outer_iter) {
    volatile int sum = 0;
    
    /* Outer wrapper loop */
    for (int k = 0; k < outer_iter; ++k) {
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
                    if (sum > 1000) {
                        sum -= 100;  /* Conditional in case */
                    }
                    break;
                case 4:
                default:
                    sum += i * 4;
                    break;
            }
            
            /* Additional control flow in loop body */
            if (i % 7 == 0) {
                continue;  /* Creates continue block */
            }
        }
        
        /* Code between inner and outer loops */
        sum += k * 100;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint control flow paths) */
NOINLINE int func_conditional_loops(int n, int m, int flag) {
    volatile int sum = 0;
    
    if (flag > 0) {
        /* First loop path */
        for (int i = 0; i < n; ++i) {
            sum += i * i;
            if (i % 2 == 0) {
                for (int j = 0; j < 3; ++j) {
                    sum += j;  /* Small nested loop */
                }
            }
        }
    } else {
        /* Second loop path - disjoint from first */
        int k = 0;
        while (k < m) {
            sum -= k * 3;
            k++;
            if (k % 4 == 0) {
                break;  /* Early exit creates extra block */
            }
        }
        
        /* Additional loop in else branch */
        for (int p = 0; p < n / 2; ++p) {
            sum += p * p;
        }
    }
    
    return sum;
}

/* Function F: Complex nested loops with multiple exits */
NOINLINE int func_complex_nesting(int n, int m) {
    volatile int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Middle loop */
        for (int j = 0; j < m; ++j) {
            /* Innermost loop */
            for (int k = 0; k < 5; ++k) {
                sum += i + j + k;
                if (sum > 10000) {
                    goto exit_inner;  /* Non-local exit */
                }
            }
            
            if (j % 3 == 0) {
                continue;  /* Middle loop continue */
            }
            
            sum += j * 100;
        }
        
        exit_inner:
        if (i % 2 == 0) {
            break;  /* Outer loop break */
        }
    }
    
    return sum;
}

/* Main driver that calls all functions */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create some runtime variability */
    int n = argc > 1 ? atoi(argv[1]) % 50 + 10 : 20;
    int m = argc > 2 ? atoi(argv[2]) % 30 + 5 : 15;
    int flag = argc > 3 ? atoi(argv[3]) % 2 : 0;
    
    /* Call all test functions */
    result += func_nested_simple(n, m);
    result += func_nested_complex(n, m);
    result += func_sequential_disjoint(n, m);
    result += func_switch_in_loop(n, 2);
    result += func_conditional_loops(n, m, flag);
    result += func_complex_nesting(n, m);
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
