/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner conditional */
NOINLINE int test_nested_simple(int n, int m) {
    int sum = 0;
    volatile int vn = n;  /* Prevent constant propagation */
    volatile int vm = m;
    
    /* Outer loop */
    for (int i = 0; i < vn; ++i) {
        /* Inner loop with multiple basic blocks */
        for (int j = 0; j < vm; ++j) {
            if (j % 3 == 0) {
                sum += i * 2;  /* Creates separate basic block */
                continue;       /* Creates back edge */
            } else if (j % 5 == 0) {
                sum += j * 3;   /* Another basic block */
                if (sum > 1000) break;  /* Early exit creates another block */
            } else {
                sum += i + j;   /* Default path */
            }
        }
        /* Additional block after inner loop */
        if (i % 7 == 0) {
            sum -= 1;
        }
    }
    return sum;
}

/* Function B: Nested loops with shared header complexity */
NOINLINE int test_nested_shared(int n, int m) {
    int sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared conditional block - could be in both loops' bitmaps */
        if (i % 2 == 0) {
            sum += 100;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            /* Complex body with multiple exits */
            switch (j % 4) {
                case 0: sum += i; break;
                case 1: sum += j; break;
                case 2: if (sum < 0) return sum; break;  /* Early return */
                case 3: sum += i * j; 
                        if (sum > 5000) goto cleanup;  /* Another exit */
                        break;
            }
        }
        
        /* Loop increment with condition */
        i++;
        if (i > n * 2) break;  /* Additional exit condition */
        
    } while (i < n);
    
cleanup:
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n) {
    int arr1[100], arr2[100];
    int sum = 0;
    
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
        /* No blocks shared with second loop */
    }
    
    /* Intermediate code to ensure disjointness */
    int temp = sum;
    sum = 0;
    
    /* Second loop - completely disjoint */
    for (int i = 0; i < n && i < 100; ++i) {
        if (arr2[i] % 4 == 0) {
            sum += arr2[i] * 3;  /* Different condition */
        } else if (arr2[i] % 4 == 1) {
            sum += arr2[i] * 5;  /* More blocks */
        } else {
            sum += arr2[i];
        }
    }
    
    return sum + temp;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE int test_switch_nested(int n) {
    int sum = 0;
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < 2; ++outer) {
        /* Inner loop with switch */
        for (int i = 0; i < n; ++i) {
            switch (i % 6) {  /* Multiple cases create many blocks */
                case 0: sum += 1; break;
                case 1: sum += i; break;
                case 2: sum += outer * 10; break;
                case 3: 
                    if (sum < 0) {
                        sum = 0;  /* Conditional block */
                    }
                    break;
                case 4:
                    for (int k = 0; k < 2; ++k) {  /* Even more nesting */
                        sum += k;
                    }
                    break;
                case 5: sum -= 1; break;
                default: sum += 100; break;
            }
            
            /* Additional condition in loop body */
            if (i % 10 == 0) {
                sum += 1000;
            }
        }
        
        /* Code between outer iterations */
        sum += outer * 10000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint paths) */
NOINLINE int test_conditional_disjoint(int n, int selector) {
    int sum = 0;
    
    if (selector > 0) {
        /* First loop path */
        for (int i = 0; i < n; ++i) {
            if (i % 2 == 0) {
                sum += i * 2;
                continue;  /* Creates back edge block */
            }
            sum += i;
            
            /* Nested loop in true branch */
            for (int j = 0; j < 3; ++j) {
                sum += j;
                if (j == 1) break;  /* Early exit */
            }
        }
    } else {
        /* Second loop path - completely disjoint */
        int k = n;
        while (k-- > 0) {  /* Different loop type */
            sum += k * 3;
            
            /* Multiple conditions */
            if (k % 3 == 0) {
                sum += 100;
                if (sum > 5000) return sum;  /* Early return */
            } else if (k % 3 == 1) {
                sum += 200;
            }
        }
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE int test_complex_multi_exit(int n) {
    int sum = 0;
    
    /* Outer loop with multiple exits */
    for (int i = 0; i < n; ++i) {
        if (i == 0) continue;  /* Skip first */
        
        /* Middle loop */
        for (int j = 0; j < i && j < 10; ++j) {
            /* Multiple exit points */
            if (sum > 10000) return sum;
            if (sum < -1000) goto exit_loop;
            
            /* Inner-most loop */
            for (int k = 0; k < 3; ++k) {
                sum += i + j + k;
                if (k == 1 && j == 5) break;  /* Break inner only */
            }
            
            /* Conditional in middle loop */
            if (j % 4 == 0) {
                sum += 7;
                continue;
            }
        }
        
        /* Another conditional exit */
        if (i % 7 == 0) {
            sum += 1000;
            if (sum > 5000) break;
        }
    }
    
exit_loop:
    /* Final processing loop */
    for (int i = 0; i < 5; ++i) {
        sum += i * 2;
    }
    
    return sum;
}

/* Main driver */
int main(int argc, char *argv[]) {
    int result = 0;
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Use seed to vary iteration counts */
    int n1 = (seed % 20) + 5;
    int n2 = (seed % 15) + 3;
    int n3 = (seed % 25) + 10;
    
    /* Call all test functions */
    result += test_nested_simple(n1, n2);
    result += test_nested_shared(n2, n3);
    result += test_sequential_disjoint(n3);
    result += test_switch_nested(n1);
    result += test_conditional_disjoint(n2, seed % 2);
    result += test_complex_multi_exit(n3);
    
    /* Prevent dead code elimination */
    volatile int output = result;
    printf("Result: %d\n", output);
    
    return 0;
}
