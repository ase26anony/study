/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner conditional */
NOINLINE uint32_t nested_loops_simple(int n, int m) {
    uint32_t sum = 0;
    volatile int vn = n;  /* Prevent constant propagation */
    volatile int vm = m;
    
    /* Outer loop */
    for (int i = 0; i < vn; ++i) {
        /* Inner loop with multiple basic blocks */
        for (int j = 0; j < vm; ++j) {
            if (j % 2 == 0) {
                sum += i * j;
                /* Continue creates separate basic block */
                continue;
            }
            /* Another basic block in inner loop */
            sum += i + j;
        }
        /* Basic block after inner loop */
        sum += i;
    }
    return sum;
}

/* Function B: Nested loops with shared header complexity */
NOINLINE uint32_t nested_loops_shared_header(int n, int m) {
    uint32_t sum = 0;
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
                /* Early continue creates new block */
                if (j % 3 == 0) continue;
            }
            sum += j;
        }
        
        i++;
        /* Conditional break in outer loop */
        if (i > vn / 2) {
            sum += 1000;
            break;
        }
    } while (i < vn);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE uint32_t sequential_disjoint_loops(int n, int m) {
    uint32_t sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    int arr1[100], arr2[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = 100 - i;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < vn; ++i) {
        if (i % 4 == 0) {
            sum += arr1[i % 100] * 2;
        } else {
            sum += arr1[i % 100];
        }
    }
    
    /* Second loop - completely separate basic blocks */
    for (int j = 0; j < vm; ++j) {
        if (j % 5 == 0) {
            sum -= arr2[j % 100];
            /* Nested if creates another block */
            if (j % 10 == 0) {
                sum += 50;
            }
        } else {
            sum += arr2[j % 100];
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE uint32_t loop_with_switch(int n, int outer_iter) {
    uint32_t sum = 0;
    volatile int vn = n;
    volatile int vouter = outer_iter;
    
    /* Outer wrapper loop */
    for (int k = 0; k < vouter; ++k) {
        /* Inner loop with switch */
        for (int i = 0; i < vn; ++i) {
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
                    sum += i - 5;
                    /* Multiple statements in case */
                    if (i % 3 == 0) {
                        sum += 7;
                    }
                    break;
                case 4:
                    sum += i / 2;
                    /* Early continue */
                    if (i == vn - 1) continue;
                    sum += 1;
                    break;
                default:
                    sum += 999;
            }
        }
        /* Block after inner loop but inside outer */
        sum += k * 100;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint control flow) */
NOINLINE uint32_t conditional_disjoint_loops(int n, int m, int flag) {
    uint32_t sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    
    if (flag > 0) {
        /* Loop in true branch */
        for (int i = 0; i < vn; ++i) {
            if (i % 2 == 0) {
                sum += i * i;
                /* Multiple exit points */
                if (i > vn / 2) return sum;
            } else {
                sum += i;
            }
        }
    } else {
        /* Different loop in false branch - disjoint blocks */
        int j = vm;
        while (j-- > 0) {
            sum += j * 3;
            /* Complex control flow */
            if (j % 4 == 0) {
                sum += 1;
                goto skip;  /* Creates another block */
            }
            sum += 2;
            skip:
            if (j == 1) break;
        }
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE uint32_t complex_nested_exits(int n, int m) {
    uint32_t sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    
    for (int i = 0; i < vn; ++i) {
        /* Middle loop */
        for (int j = 0; j < vm; ++j) {
            /* Innermost loop */
            for (int k = 0; k < 5; ++k) {
                sum += i + j + k;
                /* Conditional return from innermost */
                if (sum > 1000000) {
                    return sum;
                }
            }
            /* Conditional break from middle */
            if (j > vm / 2) {
                sum += 100;
                break;
            }
        }
        /* Continue creates separate block */
        if (i % 3 == 0) continue;
        sum += i * 10;
    }
    
    return sum;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    uint32_t total = 0;
    
    /* Use command line args for variability, fallback to defaults */
    int n = (argc > 1) ? atoi(argv[1]) % 100 + 10 : 25;
    int m = (argc > 2) ? atoi(argv[2]) % 50 + 5 : 15;
    int flag = (argc > 3) ? atoi(argv[3]) % 2 : 0;
    int outer = (argc > 4) ? atoi(argv[4]) % 3 + 2 : 3;
    
    /* Call all test functions */
    total += nested_loops_simple(n, m);
    total += nested_loops_shared_header(n, m);
    total += sequential_disjoint_loops(n, m);
    total += loop_with_switch(n, outer);
    total += conditional_disjoint_loops(n, m, flag);
    total += complex_nested_exits(n, m);
    
    /* Output result to prevent optimization */
    printf("Result: %u\n", total);
    
    return 0;
}
