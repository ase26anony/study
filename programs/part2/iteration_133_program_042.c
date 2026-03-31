/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Force variable to register to prevent constant propagation */
#define KEEP(var) asm volatile("" : : "r"(var))

/* Function A: Simple nested loops with inner loop continue */
NOINLINE int func_nested_simple(int n, int m) {
    int sum = 0;
    KEEP(n); KEEP(m);
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        sum += i;
        
        /* Inner loop with multiple basic blocks due to continue */
        for (int j = 0; j < m; ++j) {
            if (j % 3 == 0) {
                sum += j * 2;
                continue;  /* Creates separate basic block */
            }
            if (j % 5 == 0) {
                sum -= j;
                break;     /* Creates another basic block */
            }
            sum += j;
        }
        
        /* Additional block in outer loop */
        if (i % 2 == 0) {
            sum *= 2;
        }
    }
    
    return sum;
}

/* Function B: Nested loops with shared header (do-while outer) */
NOINLINE int func_nested_shared(int n, int m) {
    int sum = 0;
    int i = 0;
    KEEP(n); KEEP(m);
    
    /* do-while outer loop */
    do {
        /* Shared block before inner loop */
        if (i < n/2) {
            sum += 100;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            sum += i * j;
            if (j == m/2) {
                sum += 500;  /* Creates separate block */
            }
        }
        
        /* Another shared block */
        if (sum > 1000) {
            sum -= 50;
        }
        
        i++;
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int func_sequential_disjoint(int n, int m) {
    int sum = 0;
    int arr1[100], arr2[100];
    KEEP(n); KEEP(m);
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = 100 - i;
    }
    
    /* First loop - disjoint body */
    for (int i = 0; i < n && i < 100; ++i) {
        sum += arr1[i];
        if (i % 4 == 0) {
            sum += 10;  /* Creates separate block */
        }
    }
    
    /* Intermediate computation (separates loops) */
    sum = (sum * 3) / 2;
    
    /* Second loop - completely disjoint from first */
    for (int j = 0; j < m && j < 100; ++j) {
        sum += arr2[j];
        if (j % 3 == 0) {
            sum -= 5;   /* Creates separate block */
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE int func_switch_nested(int n, int m) {
    int sum = 0;
    KEEP(n); KEEP(m);
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < 2; ++outer) {
        sum += outer * 1000;
        
        /* Inner loop with switch */
        for (int i = 0; i < n; ++i) {
            switch (i % 5) {
                case 0:
                    sum += i;
                    break;
                case 1:
                    sum += i * 2;
                    if (i > m/2) break;  /* Early exit from case */
                    sum += 10;
                    break;
                case 2:
                    sum += i * 3;
                    /* Fall through */
                case 3:
                    sum += i * 4;
                    for (int k = 0; k < 3; ++k) {
                        sum += k;  /* Tiny nested loop inside case */
                    }
                    break;
                case 4:
                    sum += i * 5;
                    break;
                default:
                    sum += 999;
                    break;
            }
            
            /* Additional block after switch */
            if (sum % 2 == 0) {
                sum /= 2;
            }
        }
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint control flow) */
NOINLINE int func_conditional_disjoint(int n, int m, int flag) {
    int sum = 0;
    KEEP(n); KEEP(m); KEEP(flag);
    
    if (flag > 0) {
        /* First loop in true branch */
        for (int i = 0; i < n; ++i) {
            sum += i * i;
            if (i % 7 == 0) {
                sum += 77;  /* Separate block */
                continue;
            }
            sum += 1;
        }
        
        /* Additional computation in true branch */
        sum += 10000;
    } else {
        /* Second loop in false branch - completely disjoint */
        for (int j = 0; j < m; ++j) {
            sum -= j * 3;
            if (j % 11 == 0) {
                sum += 111;  /* Separate block */
                break;       /* Creates exit block */
            }
            sum += 2;
        }
        
        /* Additional computation in false branch */
        sum -= 5000;
    }
    
    /* Common post-if code */
    sum = abs(sum);
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE int func_complex_multi_exit(int n, int m) {
    int sum = 0;
    KEEP(n); KEEP(m);
    
    /* Outer loop with multiple exit points */
    for (int i = 0; i < n; ++i) {
        if (i == 0) {
            sum += 999;
            continue;
        }
        
        /* Middle loop */
        for (int j = 0; j < m; ++j) {
            sum += i * j;
            
            if (sum > 1000000) {
                return sum;  /* Early exit from function */
            }
            
            if (j == m - 1) {
                sum += 100;
                goto inner_done;  /* Jump to label */
            }
            
            sum += 1;
        }
        
        inner_done:
        if (i % 10 == 0) {
            break;  /* Exit outer loop early */
        }
        
        sum += 10;
    }
    
    return sum;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use argc for variability, but keep values small */
    int n = (argc > 1) ? atoi(argv[1]) % 50 + 10 : 20;
    int m = (argc > 2) ? atoi(argv[2]) % 40 + 10 : 15;
    int flag = (argc > 3) ? atoi(argv[3]) % 2 : 0;
    
    printf("Testing hardware loop patterns with n=%d, m=%d, flag=%d\n", n, m, flag);
    
    total += func_nested_simple(n, m);
    total += func_nested_shared(n, m);
    total += func_sequential_disjoint(n, m);
    total += func_switch_nested(n, m);
    total += func_conditional_disjoint(n, m, flag);
    total += func_complex_multi_exit(n, m);
    
    /* Use result to prevent dead code elimination */
    printf("Total checksum: %d\n", total);
    
    /* Additional volatile store to ensure all loops execute */
    volatile int dummy = total;
    
    return (total > 0) ? 0 : 1;
}
