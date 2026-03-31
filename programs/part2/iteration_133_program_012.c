/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop having multiple basic blocks */
NOINLINE int test_nested_simple(int n, int m) {
    volatile int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with multiple basic blocks */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += i * j;
                continue;  /* Creates separate basic block for continue path */
            } else {
                sum += i + j;
                /* Another basic block for the else path */
                if (j % 3 == 0) {
                    sum += 1;  /* Additional basic block */
                }
            }
        }
        /* Basic block after inner loop */
        if (i % 5 == 0) {
            sum -= 1;
        }
    }
    return sum;
}

/* Function B: Nested loops with shared header/complex control flow */
NOINLINE int test_nested_complex(int n, int m) {
    volatile int sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared basic block before inner loop */
        if (i % 2 == 0) {
            sum += i;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            sum += i * j;
            if (j == m / 2) {
                break;  /* Creates exit block */
            }
            sum += j;
        }
        
        /* Another shared block */
        sum -= i;
        i++;
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
        } else {
            break;  /* Creates exit block */
        }
        /* Additional basic block */
        sum += i % 7;
    }
    
    /* Second loop - completely disjoint blocks */
    for (int j = 0; j < m; ++j) {
        if (j < 100) {
            sum += arr2[j];
        }
        /* Different basic block pattern */
        switch (j % 4) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 2: sum += 3; break;
            case 3: sum += 4; break;
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE int test_switch_nested(int n, int outer_iter) {
    volatile int sum = 0;
    
    /* Outer wrapper loop */
    for (int k = 0; k < outer_iter; ++k) {
        /* Inner loop with switch */
        for (int i = 0; i < n; ++i) {
            switch (i % 5) {
                case 0:
                    sum += i * 2;
                    break;
                case 1:
                    sum += i + 1;
                    if (i % 3 == 0) {
                        continue;  /* Creates continue block */
                    }
                    break;
                case 2:
                    sum += i * i;
                    break;
                case 3:
                    sum += i - 1;
                    /* Fall through */
                case 4:
                    sum += i / 2;
                    break;
                default:
                    sum += 0;
            }
            
            /* Additional block after switch */
            if (i % 7 == 0) {
                sum += 7;
            }
        }
        
        /* Outer loop body after inner loop */
        sum += k * 100;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint loops in branches) */
NOINLINE int test_conditional_disjoint(int n, int m, int flag) {
    volatile int sum = 0;
    
    if (flag) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            sum += i * 3;
            if (i % 4 == 0) {
                goto early_exit;  /* Creates exit to different block */
            }
            sum += 1;
        }
    early_exit:
        sum += 1000;
    } else {
        /* Different loop in false branch - completely disjoint blocks */
        int j = m;
        while (j-- > 0) {
            sum += j * 2;
            if (j % 5 == 0) {
                continue;  /* Creates continue block */
            }
            sum += j % 3;
        }
        sum += 2000;
    }
    
    return sum;
}

/* Function F: Complex nested loops with partial overlap */
NOINLINE int test_partial_overlap(int n, int m) {
    volatile int sum = 0;
    int i = 0;
    
    /* Outer loop */
    while (i < n) {
        /* Shared header block */
        int temp = i * 2;
        
        /* First inner loop - shares some blocks with second */
        for (int j = 0; j < m; ++j) {
            sum += temp + j;
            if (j % 2 == 0) {
                /* Shared block with second inner loop */
                sum += 1;
                continue;
            }
        }
        
        /* Middle block */
        sum += i;
        
        /* Second inner loop - overlaps with first but not identical */
        for (int j = m - 1; j >= 0; --j) {
            sum += temp - j;
            if (j % 2 == 0) {
                /* Shared block with first inner loop */
                sum += 1;
                continue;
            }
            /* Different block */
            sum += 2;
        }
        
        i++;
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int seed = 1;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Use seed to vary iteration counts */
    int n = 10 + (seed % 20);
    int m = 5 + (seed % 15);
    int outer_iter = 2 + (seed % 3);
    
    printf("Running hardware loop tests with n=%d, m=%d\n", n, m);
    
    int result = 0;
    
    result += test_nested_simple(n, m);
    result += test_nested_complex(n, m);
    result += test_sequential_disjoint(n, m);
    result += test_switch_nested(n, outer_iter);
    result += test_conditional_disjoint(n, m, seed % 2);
    result += test_partial_overlap(n, m);
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    return 0;
}
