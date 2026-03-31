/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Force variable to register to prevent constant propagation */
#define KEEP_IN_REG(x) asm volatile("" : : "r"(x))

/* Function A: Simple nested loops with inner loop continue */
NOINLINE int test_nested_simple(int n, int m) {
    int sum = 0;
    KEEP_IN_REG(n);
    KEEP_IN_REG(m);
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with multiple basic blocks due to continue */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += i * j;
                continue;  /* Creates separate basic block */
            }
            sum += i + j;  /* Another basic block */
        }
        
        /* Additional block in outer loop */
        if (i % 3 == 0) {
            sum += 1;
        }
    }
    
    return sum;
}

/* Function B: Nested loops with shared header complexity */
NOINLINE int test_nested_shared_header(int n, int m) {
    int sum = 0;
    int i = 0;
    KEEP_IN_REG(n);
    KEEP_IN_REG(m);
    
    /* do-while outer loop */
    do {
        /* Shared conditional block - could be considered part of both loops */
        if (i < n/2) {
            sum += 10;
        }
        
        /* for inner loop */
        for (int j = 0; j < m; ++j) {
            if (j % 4 == 0) {
                sum += i * j * 2;
                if (j == m-1) break;  /* Early exit creates another block */
            } else {
                sum += i + j + 1;
            }
        }
        
        i++;
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n, int m) {
    int sum = 0;
    int arr1[100], arr2[100];
    KEEP_IN_REG(n);
    KEEP_IN_REG(m);
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = 100 - i;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n && i < 100; ++i) {
        if (arr1[i] % 3 == 0) {
            sum += arr1[i] * 2;  /* Creates separate block */
        } else {
            sum += arr1[i];      /* Another block */
        }
    }
    
    /* Intermediate code to ensure disjointness */
    int temp = sum % 7;
    sum += temp;
    
    /* Second loop - completely disjoint blocks */
    for (int j = 0; j < m && j < 100; ++j) {
        if (arr2[j] > 50) {
            sum -= arr2[j] / 2;
            continue;  /* Creates separate block */
        }
        sum += arr2[j] * 3;
    }
    
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    int sum = 0;
    KEEP_IN_REG(n);
    KEEP_IN_REG(outer_iter);
    
    /* Outer wrapper loop */
    for (int k = 0; k < outer_iter; ++k) {
        /* Inner loop with switch */
        for (int i = 0; i < n; ++i) {
            switch (i % 5) {
                case 0:
                    sum += i * 2;
                    break;
                case 1:
                    sum += i + 10;
                    /* Fall through */
                case 2:
                    sum += i * 3;
                    break;
                case 3:
                    if (sum > 1000) {
                        sum -= 50;  /* Conditional creates block */
                    }
                    sum += i * 4;
                    break;
                case 4:
                    sum += i * 5;
                    /* Multiple statements in case */
                    for (int j = 0; j < 3; ++j) {
                        sum += j;
                    }
                    break;
                default:
                    sum += 1;
            }
            
            /* Additional condition in loop */
            if (i == n-1) {
                sum += 100;
            }
        }
        
        /* Outer loop body continues */
        sum += k * 1000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint control flow) */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    int sum = 0;
    KEEP_IN_REG(n);
    KEEP_IN_REG(m);
    KEEP_IN_REG(flag);
    
    if (flag > 0) {
        /* First loop in true branch */
        for (int i = 0; i < n; ++i) {
            if (i % 2 == 0) {
                sum += i * i;
                continue;
            }
            sum += i;
            
            /* Nested loop inside */
            for (int j = 0; j < 3; ++j) {
                sum += j;
                if (j == 1) break;
            }
        }
    } else {
        /* Second loop in false branch - completely disjoint */
        for (int i = m; i > 0; --i) {
            sum += i * 3;
            if (i < m/2) {
                return sum;  /* Early return creates exit block */
            }
        }
        
        /* Additional loop after first in else branch */
        int count = 0;
        while (count < 5) {
            sum += count * 10;
            count++;
            if (count == 3) continue;  /* Creates block */
        }
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE int test_complex_nesting(int n, int m) {
    int sum = 0;
    KEEP_IN_REG(n);
    KEEP_IN_REG(m);
    
    /* Triple nesting */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            /* Innermost loop with multiple exits */
            for (int k = 0; k < 10; ++k) {
                sum += i + j + k;
                if (k == 5) {
                    goto inner_exit;  /* Non-local exit */
                }
                if (sum > 10000) {
                    return sum;  /* Early return */
                }
            }
            inner_exit:
            
            /* Middle loop continues */
            if (j % 3 == 0) {
                continue;
            }
            sum += j * 100;
        }
        
        /* Outer loop with switch */
        switch (i % 4) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 2: sum += 3; break;
            case 3: sum += 4; break;
        }
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use argc for variability without complex RNG */
    int n = (argc > 1) ? atoi(argv[1]) % 50 + 10 : 20;
    int m = (argc > 2) ? atoi(argv[2]) % 40 + 10 : 15;
    int flag = (argc > 3) ? atoi(argv[3]) % 2 : 1;
    int outer = (argc > 4) ? atoi(argv[4]) % 5 + 2 : 3;
    
    printf("Testing hardware loop patterns with n=%d, m=%d, flag=%d, outer=%d\n",
           n, m, flag, outer);
    
    total += test_nested_simple(n, m);
    total += test_nested_shared_header(n, m);
    total += test_sequential_disjoint(n, m);
    total += test_switch_in_loop(n, outer);
    total += test_conditional_loops(n, m, flag);
    total += test_complex_nesting(n, m);
    
    printf("Total checksum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
