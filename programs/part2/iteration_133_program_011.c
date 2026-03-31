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
            if (j % 2 == 0) {
                sum += i * j;
                continue;  /* Creates separate basic block */
            }
            sum -= i + j;
        }
        
        /* Additional block in outer loop */
        if (i % 3 == 0) {
            sum += 100;
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(sum));
    return sum;
}

/* Function B: Nested loops with shared header */
NOINLINE int test_nested_shared_header(int n, int m) {
    int sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared block before inner loop */
        if (i < n/2) {
            sum += 5;
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            if (j == i) {
                sum += j * 2;
                break;  /* Creates exit block */
            }
            sum += j;
        }
        
        i++;
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_sequential_disjoint(int n, int m) {
    int sum = 0;
    int arr1[100], arr2[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n && i < 100; ++i) {
        if (arr1[i] % 4 == 0) {
            sum += arr1[i] * 3;
            continue;
        }
        sum += arr1[i];
    }
    
    /* Intermediate computation */
    sum += 1000;
    
    /* Second loop - disjoint from first */
    for (int j = 0; j < m && j < 100; ++j) {
        if (arr2[j] > 50) {
            sum -= arr2[j] / 2;
        } else {
            sum += arr2[j];
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE int test_switch_in_loop(int n, int outer_iter) {
    int sum = 0;
    
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
                        sum -= 50;
                    }
                    break;
                case 4:
                    sum += i / 2;
                    break;
                default:
                    sum += 1;
            }
            
            /* Additional block in inner loop */
            if (i == n - 1) {
                sum += 999;
            }
        }
        
        /* Block in outer loop but not in inner */
        sum += k * 100;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting */
NOINLINE int test_conditional_loops(int n, int flag) {
    int sum = 0;
    
    if (flag) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            sum += i * i;
            if (i % 7 == 0) {
                sum += 7;
                continue;
            }
            sum -= i;
        }
    } else {
        /* Different loop in false branch */
        int j = n;
        while (j > 0) {
            sum += j;
            if (j % 3 == 0) {
                sum *= 2;
            }
            j--;
        }
    }
    
    /* Common code after conditional */
    for (int k = 0; k < 10; ++k) {
        sum += k;
    }
    
    return sum;
}

/* Function F: Complex nested structure with early exits */
NOINLINE int test_complex_nesting(int n, int m) {
    int sum = 0;
    
    /* Outer loop with multiple exit points */
    for (int i = 0; i < n; ++i) {
        if (i == 0) {
            sum += 100;
            continue;
        }
        
        if (i == n - 1) {
            sum += 1000;
            break;
        }
        
        /* Middle loop */
        for (int j = 0; j < m; ++j) {
            if (j == i) {
                sum += j * 10;
                goto inner_done;  /* Creates another exit path */
            }
            
            /* Innermost loop */
            for (int k = 0; k < 3; ++k) {
                sum += k;
                if (sum > 5000) {
                    return sum;  /* Early return from function */
                }
            }
            
            sum += j;
        }
        inner_done:
        
        /* Another inner loop at same level */
        for (int j = m; j > 0; --j) {
            sum -= j;
            if (j < i) {
                break;
            }
        }
    }
    
    return sum;
}

/* Main driver */
int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    int total = 0;
    
    /* Call each test function with varying parameters */
    total += test_nested_simple(rand() % 50 + 10, rand() % 20 + 5);
    total += test_nested_shared_header(rand() % 40 + 10, rand() % 30 + 5);
    total += test_sequential_disjoint(rand() % 60 + 10, rand() % 40 + 10);
    total += test_switch_in_loop(rand() % 30 + 10, rand() % 5 + 2);
    total += test_conditional_loops(rand() % 50 + 10, rand() % 2);
    total += test_complex_nesting(rand() % 40 + 10, rand() % 25 + 5);
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
