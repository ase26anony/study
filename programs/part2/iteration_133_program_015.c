/* test_hwdoloop.c - Test program for hardware loop optimization coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to preserve loop structures */
#define NOINLINE __attribute__((noinline, cold))

/* Function A: Simple nested loops with inner loop continue */
NOINLINE static uint64_t func_a(int n, int m) {
    volatile uint64_t sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        sum += i;
        /* Inner loop with multiple basic blocks */
        for (int j = 0; j < m; ++j) {
            if (j % 2 == 0) {
                sum += j * 2;
                continue;  /* Creates separate basic block */
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

/* Function B: Nested loops with shared header using do-while */
NOINLINE static uint64_t func_b(int n, int m) {
    volatile uint64_t sum = 0;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* Shared header block - both loops might share this in analysis */
        sum += i * 2;
        
        /* Inner for loop */
        for (int j = 0; j < m; ++j) {
            if (j < m/2) {
                sum += j;
                if (j == 3) break;  /* Early exit creates another block */
            } else {
                sum += j * 3;
            }
        }
        
        i++;
        /* Conditional continue in outer loop */
        if (i % 4 == 0) {
            sum += 50;
            continue;
        }
    } while (i < n);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE static uint64_t func_c(int n, int m) {
    volatile uint64_t sum = 0;
    volatile int arr1[100], arr2[100];
    
    /* First loop - disjoint from second */
    for (int i = 0; i < n && i < 100; ++i) {
        arr1[i] = i * 2;
        if (i % 5 == 0) {
            sum += arr1[i];
            /* Early return possibility */
            if (sum > 1000) return sum;
        } else {
            sum -= arr1[i];
        }
    }
    
    /* Second loop - completely disjoint blocks */
    for (int j = 0; j < m && j < 100; ++j) {
        arr2[j] = j * 3;
        switch (j % 3) {
            case 0: sum += arr2[j] + 1; break;
            case 1: sum += arr2[j] * 2; break;
            case 2: sum += arr2[j] / 2; break;
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE static uint64_t func_d(int n, int outer) {
    volatile uint64_t sum = 0;
    
    /* Outer wrapper loop */
    for (int k = 0; k < outer; ++k) {
        sum += k * 1000;
        
        /* Inner loop with switch - creates many basic blocks */
        for (int i = 0; i < n; ++i) {
            switch (i % 5) {
                case 0:
                    sum += i;
                    if (sum % 2) continue;  /* Continue creates new block */
                    break;
                case 1:
                    sum += i * 2;
                    break;
                case 2:
                    sum += i * 3;
                    /* Nested if inside case */
                    if (i > 10) {
                        sum += 100;
                        goto skip_rest;  /* goto creates exit edge */
                    }
                    break;
                case 3:
                    sum += i * 4;
                    for (int x = 0; x < 2; ++x) {
                        sum += x;  /* Tiny inner loop */
                    }
                    break;
                case 4:
                    sum += i * 5;
                    break;
                default:
                    sum += 999;
            }
            
            skip_rest:
            /* Common tail block */
            if (i == n-1) {
                sum += 777;
            }
        }
        
        /* Another block in outer loop */
        if (k % 2 == 0) {
            sum += 888;
        }
    }
    
    return sum;
}

/* Function E: Conditional loop nesting - disjoint loops in branches */
NOINLINE static uint64_t func_e(int n, int m, int flag) {
    volatile uint64_t sum = 0;
    
    if (flag) {
        /* Loop in true branch */
        for (int i = 0; i < n; ++i) {
            sum += i * i;
            if (i % 7 == 0) {
                sum += 111;
                continue;
            }
            sum -= 10;
        }
    } else {
        /* Different loop in false branch - disjoint blocks */
        int j = m;
        while (j-- > 0) {
            sum += j * 3;
            /* Multiple exit points */
            if (sum > 5000) return sum;
            if (j == 1) break;
        }
        
        /* Another small loop in same branch */
        for (int k = 0; k < 5; ++k) {
            sum += k * 7;
        }
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple exits */
NOINLINE static uint64_t func_f(int n) {
    volatile uint64_t sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Middle loop */
        for (int j = 0; j < i && j < 10; ++j) {
            sum += j;
            
            /* Innermost loop */
            for (int k = 0; k < 3; ++k) {
                sum += k;
                if (k == 1 && sum % 2) {
                    goto middle_loop_continue;  /* Jump to middle loop */
                }
            }
            
            middle_loop_continue:
            if (j == 5) {
                sum += 555;
                continue;  /* Continue in middle loop */
            }
        }
        
        /* Multiple exit conditions from outer loop */
        if (sum > 10000) {
            return sum;
        }
        if (i == n-1) {
            break;
        }
    }
    
    return sum;
}

/* Main driver that calls all functions */
int main(int argc, char *argv[]) {
    uint64_t total = 0;
    
    /* Use argc for variability, but keep values small */
    int n = (argc > 1) ? atoi(argv[1]) % 20 + 5 : 10;
    int m = (argc > 2) ? atoi(argv[2]) % 15 + 3 : 8;
    int outer = (argc > 3) ? atoi(argv[3]) % 5 + 2 : 3;
    int flag = (argc > 4) ? atoi(argv[4]) % 2 : 1;
    
    /* Force values into registers to prevent constant propagation */
    asm volatile("" : : "r"(n), "r"(m), "r"(outer), "r"(flag));
    
    /* Call all test functions */
    total += func_a(n, m);
    total += func_b(n, m);
    total += func_c(n, m);
    total += func_d(n, outer);
    total += func_e(n, m, flag);
    total += func_f(n);
    
    /* Prevent dead code elimination */
    volatile uint64_t result = total;
    
    printf("Result: %llu\n", (unsigned long long)result);
    
    return 0;
}
