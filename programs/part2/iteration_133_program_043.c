/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE_COLD __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner conditional */
NOINLINE_COLD
int test_nested_simple(int n, int m) {
    int sum = 0;
    volatile int vn = n;  /* Prevent constant propagation */
    volatile int vm = m;
    
    /* Outer loop */
    for (int i = 0; i < vn; ++i) {
        sum += i;
        
        /* Inner loop with conditional continue */
        for (int j = 0; j < vm; ++j) {
            if (j % 2 == 0) {
                sum += j * 2;
                continue;  /* Creates additional basic block */
            }
            sum += j;
        }
        
        /* Additional block in outer loop */
        if (i % 3 == 0) {
            sum += 100;
        }
    }
    
    return sum;
}

/* Function B: Nested loops with shared header (do-while outer) */
NOINLINE_COLD
int test_nested_shared_header(int n, int m) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared header block - could be considered part of both loops */
        int temp = i * 2;
        
        /* Inner for loop */
        for (int j = 0; j < vm; ++j) {
            if (temp > j) {
                sum += temp - j;
                break;  /* Creates exit block */
            } else {
                sum += j + temp;
                continue;
            }
        }
        
        i++;
        if (i > vn) {
            break;  /* Another exit point */
        }
    } while (1);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE_COLD
int test_sequential_disjoint(int n, int m) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    
    /* First loop - processes array A */
    int array_a[100];
    for (int i = 0; i < vn && i < 100; ++i) {
        array_a[i] = i * 3;
        if (i % 4 == 0) {
            sum += array_a[i] + 1;
        } else {
            sum += array_a[i] - 1;
        }
    }
    
    /* Second loop - completely disjoint, processes array B */
    int array_b[100];
    for (int j = 0; j < vm && j < 100; ++j) {
        array_b[j] = j * 7;
        if (j % 5 == 0) {
            sum += array_b[j] * 2;
            continue;
        }
        sum += array_b[j] / 2;
    }
    
    return sum;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE_COLD
int test_switch_in_loop(int n, int outer_iter) {
    int sum = 0;
    volatile int vn = n;
    volatile int vouter = outer_iter;
    
    /* Outer wrapper loop */
    for (int k = 0; k < vouter; ++k) {
        sum += k * 1000;
        
        /* Inner loop with switch statement */
        for (int i = 0; i < vn; ++i) {
            switch (i % 5) {
                case 0:
                    sum += i;
                    break;
                case 1:
                    sum += i * 2;
                    if (sum > 1000) {
                        sum -= 500;  /* Additional block */
                    }
                    break;
                case 2:
                    sum += i * 3;
                    continue;  /* Continue the loop */
                case 3:
                    sum += i * 4;
                    /* Fall through */
                case 4:
                    sum += i * 5;
                    break;
                default:
                    sum += 999;
                    break;
            }
            
            /* Additional computation after switch */
            if (i % 7 == 0) {
                sum += 7;
            }
        }
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint control flow) */
NOINLINE_COLD
int test_conditional_loops(int n, int m, int flag) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    volatile int vflag = flag;
    
    if (vflag > 0) {
        /* First loop in true branch */
        for (int i = 0; i < vn; ++i) {
            sum += i * i;
            if (i % 11 == 0) {
                goto early_exit;  /* Multiple exit points */
            }
            if (i % 13 == 0) {
                return sum;  /* Another exit point */
            }
        }
    } else {
        /* Different loop in false branch - disjoint from first */
        for (int j = 0; j < vm; ++j) {
            sum -= j * 3;
            if (j % 17 == 0) {
                break;  /* Exit point */
            }
            if (j % 19 == 0) {
                continue;
            }
        }
    }
    
early_exit:
    return sum;
}

/* Function F: Complex nested loops with multiple levels */
NOINLINE_COLD
int test_multi_level_nested(int n, int m, int p) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    volatile int vp = p;
    
    /* Level 1: Outer loop */
    for (int i = 0; i < vn; ++i) {
        sum += i;
        
        /* Level 2: Middle loop */
        for (int j = 0; j < vm; ++j) {
            sum += j * 10;
            
            /* Level 3: Innermost loop */
            for (int k = 0; k < vp; ++k) {
                sum += k * 100;
                
                /* Conditional in innermost loop */
                if (k % 2 == 0) {
                    sum += 1;
                    if (k % 3 == 0) {
                        sum += 2;
                        continue;
                    }
                }
            }
            
            /* Conditional in middle loop */
            if (j % 5 == 0) {
                sum += 1000;
            }
        }
        
        /* Conditional in outer loop with goto */
        if (i % 7 == 0) {
            sum += 10000;
            goto outer_continue;
        }
        
        sum += 5000;
        
    outer_continue:
        ;
    }
    
    return sum;
}

/* Function G: Loop with computed goto-like dispatch */
NOINLINE_COLD
int test_computed_dispatch(int n) {
    int sum = 0;
    volatile int vn = n;
    
    for (int i = 0; i < vn; ++i) {
        /* Multiple labels creating many basic blocks */
        if (i % 2 == 0) {
            goto block_a;
        } else if (i % 3 == 0) {
            goto block_b;
        } else if (i % 5 == 0) {
            goto block_c;
        } else {
            goto block_d;
        }
        
    block_a:
        sum += i * 2;
        continue;
        
    block_b:
        sum += i * 3;
        if (sum > 100) {
            sum -= 50;
        }
        continue;
        
    block_c:
        sum += i * 4;
        continue;
        
    block_d:
        sum += i * 5;
        continue;
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use command line arguments for variability */
    int base = (argc > 1) ? atoi(argv[1]) : 10;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    srand(seed);
    
    /* Call each test function with different parameters */
    total += test_nested_simple(base, base + 2);
    total += test_nested_shared_header(base + 1, base + 3);
    total += test_sequential_disjoint(base + 2, base + 4);
    total += test_switch_in_loop(base + 3, 2);  /* Outer iter fixed at 2 */
    total += test_conditional_loops(base + 4, base + 5, rand() % 2);
    total += test_multi_level_nested(base + 1, base + 2, base + 3);
    total += test_computed_dispatch(base + 6);
    
    /* Output result to prevent dead code elimination */
    printf("Total checksum: %d\n", total);
    
    return 0;
}
