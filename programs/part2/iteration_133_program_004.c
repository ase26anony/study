/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop having multiple blocks */
NOINLINE int test_nested_simple(int n, int m) {
    volatile int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with split body */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += i * j;  /* Basic block A1 */
            } else {
                sum += i + j;  /* Basic block A2 */
                continue;      /* Creates separate block for continue */
            }
            /* Additional block after if-else */
            sum += 1;
        }
        /* Block after inner loop */
        sum += i;
    }
    return sum;
}

/* Function B: Nested loops with shared header complexity */
NOINLINE int test_nested_shared_header(int n, int m) {
    volatile int sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared conditional block - could be considered part of both loops */
        if (i % 3 == 0) {
            sum += 100;  /* Block potentially in both loop bitmaps */
        }
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            if (j == m/2) {
                break;    /* Creates exit block */
            }
            sum += (i * j) % 7;
        }
        
        i++;
        /* Additional block with conditional continue */
        if (i % 2 == 0) {
            continue;
        }
        sum += 5;
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
        arr2[i] = 100 - i;
    }
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n && i < 100; ++i) {
        if (arr1[i] % 4 == 0) {  /* Creates multiple blocks */
            sum += arr1[i] * 2;
        } else {
            sum += arr1[i];
        }
        /* Additional exit point */
        if (sum > 1000) {
            break;
        }
    }
    
    /* Second loop - completely disjoint blocks */
    for (int j = 0; j < m && j < 100; ++j) {
        switch (arr2[j] % 3) {  /* Creates multiple case blocks */
            case 0: sum += arr2[j] * 3; break;
            case 1: sum += arr2[j] * 5; break;
            case 2: sum += arr2[j] * 7; break;
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch and outer wrapper */
NOINLINE int test_switch_in_loop(int n) {
    volatile int sum = 0;
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < 2; ++outer) {
        /* Inner loop with switch */
        for (int i = 0; i < n; ++i) {
            switch (i % 5) {  /* Creates 5 case blocks + default */
                case 0: sum += i * 1; break;
                case 1: sum += i * 2; 
                        if (sum % 3 == 0) break;
                        sum += 1;
                        break;
                case 2: sum += i * 3; 
                        continue;  /* Jumps to loop header */
                case 3: sum += i * 4; 
                        if (i == n-1) return sum;  /* Early exit */
                        break;
                case 4: sum += i * 5; break;
                default: sum += i; break;
            }
            /* Block after switch */
            sum += outer;
        }
        /* Block after inner loop */
        sum += 1000;
    }
    
    return sum;
}

/* Function E: Conditional loop nesting (disjoint control flow) */
NOINLINE int test_conditional_loops(int n, int flag) {
    volatile int sum = 0;
    
    if (flag) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            if (i % 2 == 0) {
                sum += i * 2;
                continue;
            }
            sum += i;
            /* Nested loop inside true branch */
            for (int j = 0; j < 3; ++j) {
                sum += j;
                if (j == 1) break;
            }
        }
    } else {
        /* Different loop in false branch - disjoint blocks */
        int k = n;
        while (k-- > 0) {
            sum += k * 3;
            /* Multiple exit points */
            if (sum > 5000) {
                goto done;
            }
            if (k % 4 == 0) {
                continue;
            }
            sum += 1;
        }
    }
    
done:
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE int test_complex_nesting(int n, int m) {
    volatile int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Middle loop */
        for (int j = 0; j < m; ++j) {
            /* Innermost loop */
            for (int k = 0; k < 3; ++k) {
                sum += i + j + k;
                if (k == 1 && j == m/2) {
                    goto next_outer;  /* Jump to outer loop */
                }
            }
            /* Block after innermost loop */
            sum += j * 10;
            
            if (j == m-1) {
                break;  /* Break middle loop */
            }
        }
        
        next_outer:
        sum += i * 100;
        
        if (i == n/2) {
            return sum;  /* Early return from outer loop */
        }
    }
    
    return sum;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    int total = 0;
    
    /* Use volatile iteration counts to prevent constant propagation */
    volatile int iter1 = 10 + (rand() % 20);
    volatile int iter2 = 5 + (rand() % 15);
    volatile int iter3 = 8 + (rand() % 12);
    
    total += test_nested_simple(iter1, iter2);
    total += test_nested_shared_header(iter2, iter3);
    total += test_sequential_disjoint(iter1, iter2);
    total += test_switch_in_loop(iter3);
    total += test_conditional_loops(iter1, seed % 2);
    total += test_complex_nesting(iter2, iter3);
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}
