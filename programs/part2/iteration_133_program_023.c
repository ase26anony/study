/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop having multiple blocks */
NOINLINE int test_nested_simple(int n, int m) {
    volatile int sum = 0;
    int i, j;
    
    /* Outer loop */
    for (i = 0; i < n; ++i) {
        sum += i;
        
        /* Inner loop with if statement creating multiple blocks */
        for (j = 0; j < m; ++j) {
            if (j % 3 == 0) {
                sum += j * 2;
                continue;  /* Creates separate basic block */
            }
            sum += j;
        }
        
        /* Another statement in outer loop */
        if (i % 2 == 0) {
            sum -= 1;
        }
    }
    
    return sum;
}

/* Function B: Nested loops with shared header complexity */
NOINLINE int test_nested_complex(int n, int m) {
    volatile int sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared header block - could be considered part of both loops */
        int temp = i * 2;
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            if (temp > j) {
                sum += temp - j;
                /* Early exit from inner loop */
                if (sum > 1000) break;
            } else {
                sum += j;
            }
        }
        
        i++;
        if (i % 5 == 0) {
            sum += 10;  /* Additional block in outer loop */
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
    for (int i = 0; i < n; ++i) {
        if (i < 100) {
            sum += arr1[i];
            if (arr1[i] % 4 == 0) {
                sum += 1;  /* Extra block */
            }
        }
    }
    
    /* Intermediate code between loops */
    int intermediate = sum / 2;
    
    /* Second loop - completely disjoint blocks */
    for (int j = 0; j < m; ++j) {
        if (j < 100) {
            sum += arr2[j];
            if (arr2[j] % 3 == 0) {
                sum -= 1;  /* Extra block */
            }
        }
    }
    
    return sum + intermediate;
}

/* Function D: Loop with switch inside, wrapped by outer loop */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    volatile int sum = 0;
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < outer_iter; ++outer) {
        
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
                    sum += i * 4;
                    if (sum > 500) {
                        sum = 500;  /* Extra block */
                    }
                    break;
                default:
                    sum += i * 5;
                    break;
            }
            
            /* Additional block in inner loop */
            if (i % 7 == 0) {
                sum += 7;
            }
        }
        
        /* Block in outer loop but not in inner */
        sum += outer * 100;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting - disjoint loops in branches */
NOINLINE int test_conditional_loops(int n, int m, int condition) {
    volatile int sum = 0;
    
    if (condition > 0) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            sum += i * 2;
            if (i % 3 == 0) {
                continue;  /* Creates separate block */
            }
            sum += 1;
        }
        
        /* Additional code in true branch */
        sum += 1000;
    } else {
        /* Different loop in false branch */
        int j = 0;
        while (j < m) {
            sum += j * 3;
            j++;
            if (j % 4 == 0) {
                sum += 5;  /* Extra block */
                /* Early exit possibility */
                if (sum > 2000) break;
            }
        }
        
        /* Additional code in false branch */
        sum += 2000;
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE int test_complex_nesting(int n, int m) {
    volatile int sum = 0;
    
    /* Outer loop with multiple exit points */
    for (int i = 0; i < n; ++i) {
        if (i == 0) {
            sum += 10;
            continue;
        }
        
        /* First inner loop */
        for (int j = 0; j < m; ++j) {
            sum += i + j;
            if (sum > 10000) {
                return sum;  /* Early return from function */
            }
        }
        
        /* Second inner loop at same nesting level */
        for (int k = 0; k < i; ++k) {
            sum -= k;
            if (k % 2 == 0) {
                goto loop_end;  /* Jump to outer loop end */
            }
        }
        
        /* Middle block between inner loops */
        sum += i * 100;
        
        loop_end:
        if (i % 10 == 0) {
            break;  /* Early exit from outer loop */
        }
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create some variability in iteration counts */
    int base_iter = (argc > 1) ? atoi(argv[1]) : 10;
    if (base_iter < 5) base_iter = 5;
    if (base_iter > 100) base_iter = 100;
    
    /* Test all patterns */
    result += test_nested_simple(base_iter, base_iter / 2);
    result += test_nested_complex(base_iter, base_iter / 3);
    result += test_sequential_disjoint(base_iter, base_iter / 2);
    result += test_switch_in_loop(base_iter, 2);
    result += test_conditional_loops(base_iter, base_iter / 2, argc);
    result += test_complex_nesting(base_iter, base_iter / 3);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
