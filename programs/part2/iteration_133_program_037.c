/* test_hwdoloop.c - Test program to cover hardware loop optimization bitmap intersection checks */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop having multiple basic blocks */
NOINLINE int test_nested_simple(int n, int m) {
    int sum = 0;
    volatile int vn = n;  /* Prevent constant propagation */
    volatile int vm = m;
    
    /* Outer loop - will have its own header and latch blocks */
    for (int i = 0; i < vn; ++i) {
        /* Inner loop with multiple basic blocks due to if/continue */
        for (int j = 0; j < vm; ++j) {
            /* This creates two basic blocks in the inner loop */
            if (j % 3 == 0) {
                continue;  /* Creates a back edge from a different block */
            }
            sum += i * j;
            
            /* Another conditional to create more blocks */
            if (j % 5 == 0) {
                sum += 1;  /* Creates another distinct block */
            }
        }
        
        /* Additional block in outer loop after inner loop */
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
        /* Block that might be shared in bitmap analysis */
        int temp = i * 2;
        
        /* Inner for loop */
        for (int j = 0; j < vm; ++j) {
            /* Multiple exit points from inner loop */
            if (j > 10 && temp > 20) {
                break;  /* Creates exit edge to outer loop */
            }
            
            sum += temp + j;
            
            /* Continue creates another back edge */
            if (j % 2 == 0) {
                continue;
            }
            
            sum += 1;
        }
        
        /* Another inner loop at same level */
        for (int k = 0; k < i; ++k) {
            sum -= k;
            if (k == 3) {
                goto skip;  /* Creates another exit path */
            }
        }
        skip:
        
        i++;
    } while (i < vn);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n, int m) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    
    /* First loop - operates on first part of array */
    int arr1[100];
    for (int i = 0; i < vn && i < 100; ++i) {
        arr1[i] = i * 2;
        /* Conditional creates multiple blocks */
        if (i % 4 == 0) {
            arr1[i] += 1;
        } else {
            arr1[i] -= 1;
        }
        sum += arr1[i];
    }
    
    /* Second loop - completely disjoint, different array */
    int arr2[100];
    for (int j = 0; j < vm && j < 100; ++j) {
        arr2[j] = j * 3;
        /* Different conditional structure */
        switch (j % 3) {
            case 0: arr2[j] += 2; break;
            case 1: arr2[j] -= 2; break;
            case 2: arr2[j] *= 2; break;
        }
        sum += arr2[j];
    }
    
    return sum;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    int sum = 0;
    volatile int vn = n;
    volatile int vouter = outer_iter;
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < vouter; ++outer) {
        /* Inner loop with switch statement */
        for (int i = 0; i < vn; ++i) {
            /* Switch creates multiple basic blocks */
            switch (i % 5) {
                case 0:
                    sum += i * 1;
                    break;
                case 1:
                    sum += i * 2;
                    /* Nested if inside case */
                    if (i > 10) sum += 5;
                    break;
                case 2:
                    sum += i * 3;
                    break;
                case 3:
                    sum += i * 4;
                    /* Early continue */
                    if (i % 2 == 0) continue;
                    break;
                case 4:
                    sum += i * 5;
                    /* Another control flow */
                    for (int k = 0; k < 2; ++k) {
                        sum += k;
                    }
                    break;
            }
            
            /* Additional block after switch */
            if (i % 7 == 0) {
                sum -= 1;
            }
        }
        
        /* Block in outer loop after inner loop */
        sum += outer * 100;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting - disjoint loops in branches */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    
    if (flag) {
        /* Loop in true branch */
        for (int i = 0; i < vn; ++i) {
            sum += i * 2;
            /* Multiple blocks */
            if (i % 3 == 0) {
                continue;
            }
            sum += 1;
            
            /* Nested loop in true branch */
            for (int j = 0; j < 3; ++j) {
                sum += j;
                if (j == 1) break;
            }
        }
    } else {
        /* Different loop in false branch - completely disjoint */
        for (int k = 0; k < vm; ++k) {
            sum -= k * 3;
            /* Different control flow */
            switch (k % 4) {
                case 0: sum += 10; break;
                case 1: sum += 20; break;
                default: sum += 30; break;
            }
        }
        
        /* Another sequential loop in false branch */
        int count = 0;
        while (count < 5) {
            sum += count * 100;
            count++;
            if (count == 3) {
                goto done;  /* Different exit path */
            }
        }
        done:;
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE int test_complex_nesting(int n, int m) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    
    /* Three-level nesting */
    for (int i = 0; i < vn; ++i) {
        /* Middle loop */
        int j = 0;
        while (j < vm) {
            /* Innermost loop */
            for (int k = 0; k < 3; ++k) {
                sum += i + j + k;
                
                /* Multiple exit points from innermost */
                if (k == 1 && i > 5) {
                    goto next_j;  /* Exit to middle loop */
                }
                
                if (k == 2 && j > 3) {
                    goto next_i;  /* Exit to outer loop */
                }
            }
            
            /* Block after innermost in middle loop */
            sum += j * 10;
            
            next_j:
            j++;
        }
        
        /* Block after middle loop in outer loop */
        if (i % 2 == 0) {
            sum += i * 100;
        } else {
            continue;  /* Skip the rest of outer loop body */
        }
        
        next_i:
        sum += 1;
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use argc for variability, but keep values small for reasonable runtime */
    int n = (argc > 1) ? atoi(argv[1]) % 20 + 5 : 10;
    int m = (argc > 2) ? atoi(argv[2]) % 15 + 5 : 8;
    int outer = (argc > 3) ? atoi(argv[3]) % 5 + 2 : 3;
    int flag = (argc > 4) ? atoi(argv[4]) % 2 : 1;
    
    printf("Testing hardware loop patterns with n=%d, m=%d, outer=%d, flag=%d\n", 
           n, m, outer, flag);
    
    /* Call all test functions */
    total += test_nested_simple(n, m);
    total += test_nested_complex(n, m);
    total += test_sequential_disjoint(n, m);
    total += test_switch_in_loop(n, outer);
    total += test_conditional_loops(n, m, flag);
    total += test_complex_nesting(n, m);
    
    printf("Total checksum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
