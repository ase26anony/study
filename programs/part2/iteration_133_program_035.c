/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
#define USE_VAR(var) asm volatile("" : : "r"(var))

/* Function A: Simple nested loops with inner loop having multiple blocks */
NOINLINE int func_nested_simple(int n, int m) {
    int sum = 0;
    USE_VAR(n);
    USE_VAR(m);
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with multiple basic blocks */
        for (int j = 0; j < m; ++j) {
            if (j % 3 == 0) {
                sum += i * 2;  /* Creates separate basic block */
                continue;       /* Creates another edge */
            } else if (j % 5 == 0) {
                sum += i * 3;  /* Another basic block */
                if (sum > 1000) {
                    break;      /* Early exit creates another block */
                }
            } else {
                sum += i + j;   /* Default block */
            }
        }
        
        /* Additional block in outer loop but not in inner */
        if (i % 7 == 0) {
            sum -= 1;
        }
    }
    
    return sum;
}

/* Function B: Nested loops with shared header/initialization */
NOINLINE int func_nested_shared_header(int n, int m) {
    int sum = 0;
    int count = 0;
    USE_VAR(n);
    USE_VAR(m);
    
    /* do-while outer loop */
    int i = 0;
    do {
        /* Shared initialization block - could be in both loop bitmaps */
        int temp = i * 2;
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            /* Switch creates multiple basic blocks */
            switch (j % 4) {
                case 0:
                    sum += temp;
                    break;
                case 1:
                    sum += temp + 1;
                    /* Fall through */
                case 2:
                    sum += temp + 2;
                    break;
                default:
                    sum += temp + 3;
                    if (sum > 500) {
                        goto early_exit;  /* Creates exit block */
                    }
            }
            count++;
        }
        
        /* Block only in outer loop */
        if (count > 100) {
            sum /= 2;
        }
        
        i++;
    } while (i < n);
    
early_exit:
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int func_sequential_disjoint(int n, int m) {
    int sum = 0;
    int arr1[100], arr2[100];
    USE_VAR(n);
    USE_VAR(m);
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = 100 - i;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n && i < 100; ++i) {
        if (i % 2 == 0) {
            sum += arr1[i] * 2;  /* Creates separate block */
        } else {
            sum += arr1[i];      /* Another block */
        }
        
        /* Early return creates exit block */
        if (sum > 10000) {
            return sum;
        }
    }
    
    /* Intermediate code - ensures loops are disjoint */
    int intermediate = sum * 2;
    
    /* Second loop - completely disjoint from first */
    for (int j = 0; j < m && j < 100; ++j) {
        switch (j % 3) {
            case 0:
                sum += arr2[j] + intermediate;
                break;
            case 1:
                sum += arr2[j] * 2 + intermediate;
                break;
            case 2:
                sum += arr2[j] * 3 + intermediate;
                /* Nested if creates another block */
                if (sum < 0) {
                    sum = 0;
                }
                break;
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch wrapped by outer loop */
NOINLINE int func_wrapped_switch(int n, int outer_iter) {
    int sum = 0;
    USE_VAR(n);
    USE_VAR(outer_iter);
    
    /* Outer wrapper loop */
    for (int wrap = 0; wrap < outer_iter; ++wrap) {
        /* Inner loop with switch */
        for (int i = 0; i < n; ++i) {
            /* Switch with multiple cases creates many blocks */
            switch (i % 6) {
                case 0:
                    sum += i * 1;
                    /* Conditional continue */
                    if (i % 12 == 0) continue;
                    break;
                case 1:
                    sum += i * 2;
                    break;
                case 2:
                    sum += i * 3;
                    /* Nested if */
                    if (sum > 1000) {
                        sum -= 500;
                    }
                    break;
                case 3:
                    sum += i * 4;
                    /* Another conditional */
                    if (i % 8 == 0) {
                        sum += 100;
                    }
                    break;
                case 4:
                    sum += i * 5;
                    /* Early break from inner loop */
                    if (sum > 2000) {
                        goto end_inner;
                    }
                    break;
                default:
                    sum += i * 6;
                    break;
            }
            
            /* Additional block in inner loop */
            if (i % 7 == 0) {
                sum += wrap;
            }
        }
    end_inner:
        
        /* Block in outer loop but not inner */
        sum += wrap * 10;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting - disjoint loops in branches */
NOINLINE int func_conditional_disjoint(int n, int m, int flag) {
    int sum = 0;
    USE_VAR(n);
    USE_VAR(m);
    USE_VAR(flag);
    
    if (flag > 0) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            if (i % 2 == 0) {
                sum += i * 3;
                continue;
            }
            sum += i;
            
            /* Nested loop in true branch */
            for (int j = 0; j < 5; ++j) {
                sum += j;
                if (j == 3) break;
            }
        }
    } else {
        /* Different loop in false branch - disjoint from true branch loop */
        int k = 0;
        while (k < m) {
            sum += k * 2;
            k++;
            
            /* Multiple exit points */
            if (sum > 500) {
                return sum;
            }
            
            if (k % 4 == 0) {
                sum -= 10;
                continue;
            }
        }
    }
    
    /* Common code after if-else */
    sum += flag * 100;
    
    /* Another loop that might intersect with previous ones */
    for (int x = 0; x < 10; ++x) {
        sum += x;
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE int func_complex_nesting(int n, int m) {
    int sum = 0;
    USE_VAR(n);
    USE_VAR(m);
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* First inner loop */
        for (int j = 0; j < m; ++j) {
            sum += i + j;
            
            /* Conditional exit from inner loop */
            if (sum > 1000 && j > m/2) {
                goto next_outer_iter;
            }
            
            /* Another conditional creating block */
            if (j % 3 == 0) {
                sum += 5;
            }
        }
        
        /* Second inner loop at same level as first */
        for (int k = 0; k < i && k < 20; ++k) {
            sum += k * 2;
            
            /* Switch inside */
            switch (k % 4) {
                case 0: sum += 1; break;
                case 1: sum += 2; break;
                case 2: sum += 3; break;
                case 3: sum += 4; 
                        if (sum > 500) goto early_return;
                        break;
            }
        }
        
    next_outer_iter:
        /* Continue outer loop */
        if (i % 5 == 0) {
            sum -= 10;
        }
    }
    
    return sum;

early_return:
    return sum * 2;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Use different iteration counts to create varied control flow */
    int n1 = 10 + (seed % 20);
    int m1 = 5 + (seed % 15);
    int n2 = 8 + (seed % 12);
    int m2 = 6 + (seed % 10);
    int outer_iter = 2 + (seed % 3);
    int flag = seed % 2;
    
    printf("Testing hardware loop patterns with seed=%d\n", seed);
    
    /* Call all test functions */
    result += func_nested_simple(n1, m1);
    printf("func_nested_simple: %d\n", result);
    
    result += func_nested_shared_header(n2, m2);
    printf("func_nested_shared_header: %d\n", result);
    
    result += func_sequential_disjoint(n1, m2);
    printf("func_sequential_disjoint: %d\n", result);
    
    result += func_wrapped_switch(n2, outer_iter);
    printf("func_wrapped_switch: %d\n", result);
    
    result += func_conditional_disjoint(n1, m1, flag);
    printf("func_conditional_disjoint: %d\n", result);
    
    result += func_complex_nesting(n2, m2);
    printf("func_complex_nesting: %d\n", result);
    
    printf("Final result: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    volatile int dummy = result;
    (void)dummy;
    
    return 0;
}
