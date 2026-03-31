/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop having multiple basic blocks */
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
                continue;       /* Creates back edge block */
            } else if (j % 5 == 0) {
                sum += j * 3;  /* Another basic block */
                break;         /* Creates exit block */
            } else {
                sum += i + j;  /* Default block */
            }
        }
        /* Additional block in outer loop body */
        if (i % 2 == 0) {
            sum -= 1;
        }
    }
    return sum;
}

/* Function B: Nested loops with shared header/complex control flow */
NOINLINE int test_nested_complex(int n, int m) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared conditional block - could be in both loops' bitmaps */
        if (i % 4 == 0) {
            sum += 100;
        }
        
        /* Inner for loop */
        for (int j = 0; j < vm; ++j) {
            if (j < vm / 2) {
                sum += i * j;
                if (j == 3) goto skip_point;  /* Creates additional exit edge */
            } else {
                sum -= j;
            }
        }
        skip_point:
        i++;
    } while (i < vn);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n) {
    int sum = 0;
    volatile int vn = n;
    int arr1[100], arr2[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < vn && i < 100; ++i) {
        if (i % 2 == 0) {
            sum += arr1[i];  /* Basic block A */
        } else {
            sum -= arr1[i];  /* Basic block B */
        }
    }
    
    /* Intermediate code to ensure disjointness */
    int temp = sum * 2;
    asm volatile("" : "+r"(temp));  /* Prevent optimization */
    
    /* Second loop - completely disjoint blocks */
    for (int i = 0; i < vn && i < 100; ++i) {
        if (i % 3 == 0) {
            sum += arr2[i] * 2;  /* Different basic blocks */
        } else if (i % 3 == 1) {
            sum += arr2[i] / 2;
        } else {
            sum += arr2[i];
        }
    }
    
    return sum + temp;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    int sum = 0;
    volatile int vn = n;
    volatile int vouter = outer_iter;
    
    /* Outer wrapper loop */
    for (int k = 0; k < vouter; ++k) {
        /* Inner loop with switch */
        for (int i = 0; i < vn; ++i) {
            switch (i % 5) {
                case 0:
                    sum += i * 10;  /* Case block 0 */
                    break;
                case 1:
                    sum += i * 20;  /* Case block 1 */
                    if (sum > 1000) return sum;  /* Early exit */
                    break;
                case 2:
                    sum += i * 30;  /* Case block 2 */
                    continue;  /* Skip to next iteration */
                case 3:
                    sum += i * 40;  /* Case block 3 */
                    /* Fall through */
                case 4:
                    sum += i * 50;  /* Case block 4 */
                    break;
                default:
                    sum += i;  /* Default block */
            }
            
            /* Additional block after switch */
            if (i % 7 == 0) {
                sum -= 5;
            }
        }
        
        /* Block between inner loop iterations */
        sum += k * 1000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint loops) */
NOINLINE int test_conditional_loops(int n, int flag) {
    int sum = 0;
    volatile int vn = n;
    
    if (flag > 0) {
        /* Loop in true branch */
        for (int i = 0; i < vn; ++i) {
            sum += i * i;
            if (i % 11 == 0) {
                sum += 100;  /* Additional block */
                continue;
            }
        }
    } else {
        /* Different loop in false branch - disjoint blocks */
        int j = vn;
        while (j-- > 0) {
            sum -= j * 2;
            if (j % 13 == 0) {
                sum += 50;  /* Different basic block pattern */
                break;
            }
        }
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE int test_complex_nesting(int n, int m) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    
    /* Outer loop */
    for (int i = 0; i < vn; ++i) {
        /* Middle loop */
        for (int j = 0; j < vm; ++j) {
            /* Innermost loop */
            for (int k = 0; k < 5; ++k) {
                sum += i + j + k;
                if (k == 2 && j == 3) {
                    goto middle_loop_continue;  /* Exit innermost */
                }
            }
            
            /* Block after innermost loop */
            sum += j * 10;
            
            middle_loop_continue:
            if (j % 4 == 0) {
                sum += 1;
            }
        }
        
        /* Multiple exit points from outer loop */
        if (sum > 10000) {
            return sum;  /* Early return */
        }
        if (i == vn / 2) {
            break;  /* Break from outer loop */
        }
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use argc for variability without complex control flow */
    int base = (argc > 1) ? atoi(argv[1]) : 10;
    if (base <= 0) base = 10;
    
    /* Run all test functions with different parameters */
    total += test_nested_simple(base, base + 5);
    total += test_nested_complex(base + 2, base + 3);
    total += test_sequential_disjoint(base + 1);
    total += test_switch_in_loop(base + 4, 2);
    total += test_conditional_loops(base + 3, argc % 2);
    total += test_complex_nesting(base / 2 + 1, base / 3 + 1);
    
    /* Output result to prevent dead code elimination */
    printf("Total checksum: %d\n", total);
    
    return 0;
}
