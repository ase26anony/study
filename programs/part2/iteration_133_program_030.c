/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Force variable to stay in register */
#define KEEP(var) asm volatile("" : : "r"(var))

/* Function A: Simple nested loops with inner loop continue */
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
        }
        
        /* Another block in outer loop */
        if (i % 3 == 0) {
            sum -= 1;
        }
    }
    
    return sum;
}

/* Function B: Nested loops with shared header (do-while outer) */
NOINLINE int test_shared_header(int n, int m) {
    int sum = 0;
    int i = 0;
    KEEP(n); KEEP(m);
    
    /* Shared header block */
    if (n > 0 && m > 0) {
        sum = 1;
    }
    
    /* do-while outer loop */
    do {
        /* This block is shared conceptually */
        if (sum % 2 == 0) {
            sum += 2;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            if (j == i) {
                sum += 5;
                break;  /* Creates exit block */
            }
            sum += j;
        }
        
        i++;
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_disjoint_loops(int n, int m) {
    int sum = 0;
    int arr1[100], arr2[100];
    KEEP(n); KEEP(m);
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = 100 - i;
    }
    
    /* First loop - disjoint body */
    for (int i = 0; i < n; ++i) {
        if (i < 50) {
            sum += arr1[i];
        } else {
            sum += arr1[i] * 2;
        }
    }
    
    /* Intermediate computation (separate block) */
    sum = (sum * 3) / 2;
    
    /* Second loop - completely disjoint from first */
    for (int j = 0; j < m; ++j) {
        if (j % 4 == 0) {
            sum -= arr2[j];
            continue;
        }
        sum += arr2[j];
    }
    
    return sum;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE int test_switch_in_loop(int n, int m) {
    int sum = 0;
    KEEP(n); KEEP(m);
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < 2; ++outer) {
        sum += outer * 10;
        
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
                    sum += 3;
                    break;
                case 3:
                    if (i < m) {
                        sum += 7;
                        continue;  /* Continue inner loop */
                    }
                    sum += 4;
                    break;
                case 4:
                    sum += i / 2;
                    break;
                default:
                    sum -= 1;
            }
            
            /* Additional block in inner loop */
            if (sum > 1000) {
                sum = sum % 1000;
            }
        }
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint control flow) */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    int sum = 0;
    KEEP(n); KEEP(m); KEEP(flag);
    
    if (flag) {
        /* First loop in true branch */
        for (int i = 0; i < n; ++i) {
            if (i % 2 == 0) {
                for (int j = 0; j < 3; ++j) {
                    sum += j;
                    if (j == 1) {
                        break;
                    }
                }
            }
            sum += i;
        }
    } else {
        /* Second loop in false branch - disjoint from first */
        int k = 0;
        while (k < m) {
            sum += k * k;
            k++;
            if (k == m/2) {
                sum += 100;
                continue;
            }
        }
        
        /* Additional nested loop in false branch */
        for (int x = 0; x < 5; ++x) {
            sum -= x;
        }
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE int test_complex_nesting(int n, int m) {
    int sum = 0;
    KEEP(n); KEEP(m);
    
    /* Outer loop with multiple exit points */
    for (int i = 0; i < n; ++i) {
        if (i == 0) {
            sum += 1;
            /* Early continue creates separate block */
            continue;
        }
        
        /* Middle loop */
        for (int j = 0; j < m; ++j) {
            if (j == i) {
                sum += 10;
                goto inner_exit;  /* Unusual control flow */
            }
            
            /* Innermost loop */
            for (int k = 0; k < 3; ++k) {
                sum += k;
                if (sum > 500) {
                    return sum;  /* Early return from function */
                }
            }
            
            sum += j;
        }
        
    inner_exit:
        if (sum > 1000) {
            break;  /* Break outer loop */
        }
        
        sum += i;
    }
    
    return sum;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create some runtime variability */
    int n = (argc > 1) ? atoi(argv[1]) % 50 + 10 : 20;
    int m = (argc > 2) ? atoi(argv[2]) % 30 + 5 : 15;
    int flag = (argc > 3) ? atoi(argv[3]) % 2 : 0;
    
    /* Call all test functions */
    result += test_nested_simple(n, m);
    result += test_shared_header(n, m);
    result += test_disjoint_loops(n, m);
    result += test_switch_in_loop(n, m);
    result += test_conditional_loops(n, m, flag);
    result += test_complex_nesting(n, m);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
