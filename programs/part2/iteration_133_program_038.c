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
        sum += i;
        
        /* Inner loop with multiple basic blocks */
        for (int j = 0; j < vm; ++j) {
            if (j % 2 == 0) {
                sum += j * 2;
                continue;  /* Creates separate basic block */
            }
            sum += j;
            
            /* Additional basic block inside inner loop */
            if (j == vm - 1) {
                sum += 100;
            }
        }
        
        /* Basic block between inner and outer loop */
        if (i % 3 == 0) {
            sum -= 5;
        }
    }
    
    return sum;
}

/* Function B: Nested loops with shared header (do-while outer) */
NOINLINE int test_nested_shared_header(int n, int m) {
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
            
            /* Unreachable but creates another block */
            sum += 999;
        }
        
        /* Conditional that could be shared */
        if (sum > 1000) {
            sum /= 2;
        }
        
        i++;
    } while (i < vn);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n, int m) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    
    /* First loop - completely disjoint from second */
    for (int i = 0; i < vn; ++i) {
        if (i % 4 == 0) {
            sum += i * 3;
            /* Early continue creates separate block */
            continue;
        }
        sum += i;
        
        /* Nested if to create more blocks */
        if (i > vn / 2) {
            sum -= 10;
        }
    }
    
    /* Intermediate code to ensure disjointness */
    int intermediate = sum * 2;
    
    /* Second loop - no block intersection with first */
    for (int j = 0; j < vm; ++j) {
        switch (j % 3) {
            case 0:
                sum += intermediate + j;
                break;
            case 1:
                sum += intermediate - j;
                /* Fall through */
            case 2:
                sum += j * j;
                break;
            default:
                sum += 1;
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE int test_switch_in_loop(int n) {
    int sum = 0;
    volatile int vn = n;
    
    /* Outer wrapper loop */
    for (int wrap = 0; wrap < 2; ++wrap) {
        /* Inner loop with switch */
        for (int i = 0; i < vn; ++i) {
            /* Multiple case blocks within the loop */
            switch (i % 5) {
                case 0:
                    sum += i + wrap;
                    if (sum > 1000) {
                        sum = sum % 1000;
                    }
                    break;
                case 1:
                    sum += i * 2 + wrap;
                    for (int k = 0; k < 3; ++k) {
                        sum += k;  /* Tiny inner loop */
                    }
                    break;
                case 2:
                    sum += i * 3 + wrap;
                    /* Nested conditional */
                    if (wrap == 1) {
                        sum += 50;
                    }
                    break;
                case 3:
                    sum += i * 4 + wrap;
                    /* Small loop inside case */
                    int t = 0;
                    while (t < 2) {
                        sum += t;
                        t++;
                    }
                    break;
                case 4:
                    sum += i * 5 + wrap;
                    /* Another conditional */
                    sum += (i > vn/2) ? 25 : -25;
                    break;
                default:
                    sum += 1;
            }
            
            /* Common block after switch */
            if (i == vn - 1) {
                sum += 999;
            }
        }
        
        /* Outer loop increment block */
        sum += wrap * 1000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint loops) */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    
    if (flag) {
        /* Loop in true branch */
        for (int i = 0; i < vn; ++i) {
            sum += i * i;
            if (i % 7 == 0) {
                continue;  /* Extra block */
            }
            sum += i;
            
            /* Nested conditional loop */
            for (int j = 0; j < 3; ++j) {
                sum += j;
                if (j == 1) {
                    break;
                }
            }
        }
    } else {
        /* Different loop in false branch - disjoint from true branch loop */
        int k = 0;
        while (k < vm) {
            sum -= k;
            k++;
            
            if (k > vm / 2) {
                sum += 100;
                /* Early exit from while */
                if (sum > 500) {
                    break;
                }
            }
        }
        
        /* Another sequential loop in false branch */
        for (int p = 0; p < 5; ++p) {
            sum += p * p * p;
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
        /* First inner loop */
        for (int j = 0; j < vm; ++j) {
            sum += i * j;
            
            /* Conditional exit from inner loop */
            if (sum > 10000) {
                goto outer_continue;  /* Jump to outer loop */
            }
            
            if (j % 2 == 0) {
                continue;
            }
            
            sum += 1;
        }
        
        /* Second inner loop (sequential to first) */
        for (int k = 0; k < i; ++k) {
            sum -= k;
            
            /* Multiple exit points */
            if (k == 5) {
                return sum;  /* Early return from function */
            }
            
            if (sum < 0) {
                break;
            }
        }
        
    outer_continue:
        /* Label creates a basic block */
        sum += 10;
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to create some runtime variability */
    int base_iter = (argc > 1) ? atoi(argv[1]) : 10;
    if (base_iter <= 0) base_iter = 10;
    
    /* Run all test functions with different iteration counts */
    result += test_nested_simple(base_iter, base_iter / 2 + 1);
    result += test_nested_shared_header(base_iter + 2, base_iter / 3 + 1);
    result += test_sequential_disjoint(base_iter, base_iter + 1);
    result += test_switch_in_loop(base_iter + 3);
    result += test_conditional_loops(base_iter, base_iter + 2, argc % 2);
    result += test_complex_nesting(base_iter / 2 + 1, base_iter / 3 + 1);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
