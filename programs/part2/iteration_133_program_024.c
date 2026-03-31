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
        sum += i;
        
        /* Inner loop with if statement creating multiple basic blocks */
        for (int j = 0; j < vm; ++j) {
            if (j % 2 == 0) {
                sum += j * 2;
            } else {
                sum += j;
                /* Add continue to create another block */
                if (j % 3 == 0) continue;
                sum += 1;
            }
        }
        
        /* Another block in outer loop */
        if (i % 2 == 0) {
            sum -= 1;
        }
    }
    
    return sum;
}

/* Function B: Nested loops with shared header/complex relationship */
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
            sum += temp + j;
            
            /* Multiple exit points from inner loop */
            if (j == m/2 && i == n/2) {
                sum += 1000;  /* Early exit block */
                break;
            }
            
            /* Another conditional block */
            if (j % 4 == 0) {
                sum += 5;
                goto inner_label;  /* Create another basic block */
            }
            
            sum += 2;
        inner_label:
            sum += 3;
        }
        
        i++;
    } while (i < vn);
    
    return sum;
}

/* Function C: Sequential disjoint loops */
NOINLINE int test_disjoint_loops(int n, int m) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    
    /* First loop with its own basic blocks */
    for (int i = 0; i < vn; ++i) {
        if (i % 3 == 0) {
            sum += i * 3;
        } else if (i % 3 == 1) {
            sum += i * 2;
            /* Add a break to create exit block */
            if (i > n/2) break;
        } else {
            sum += i;
        }
    }
    
    /* Second loop - completely disjoint, no shared blocks */
    for (int j = 0; j < vm; ++j) {
        sum += j * j;
        
        /* Different control flow pattern */
        switch (j % 4) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 2: sum += 3; break;
            case 3: sum += 4; break;
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch, wrapped in outer loop */
NOINLINE int test_switch_in_loop(int n, int m) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    
    /* Outer wrapper loop */
    for (int outer = 0; outer < 2; ++outer) {
        sum += outer * 100;
        
        /* Inner loop with switch statement */
        for (int i = 0; i < vn; ++i) {
            /* Switch creates multiple basic blocks */
            switch (i % 5) {
                case 0:
                    sum += i;
                    if (i % 2 == 0) break;
                    sum += 10;
                    break;
                case 1:
                    sum += i * 2;
                    /* Nested if in case */
                    if (i > vm) {
                        sum += 20;
                        goto case_end;
                    }
                    sum += 30;
                    break;
                case 2:
                    sum += i * 3;
                    continue;  /* Skip to next iteration */
                case 3:
                    sum += i * 4;
                    /* Early return from function */
                    if (i == n-1) return sum;
                    break;
                case 4:
                    sum += i * 5;
                    break;
                default:
                    sum += 999;
                    break;
            }
        case_end:
            sum += 1;
        }
    }
    
    return sum;
}

/* Function E: Conditional loop nesting - loops in different branches */
NOINLINE int test_conditional_loops(int n, int m, int flag) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    
    if (flag) {
        /* Loop in true branch */
        for (int i = 0; i < vn; ++i) {
            sum += i * i;
            
            /* Nested loop inside */
            for (int j = 0; j < i && j < vm; ++j) {
                sum += j;
                if (j % 2 == 0) {
                    sum += 5;
                    continue;
                }
                sum += 10;
            }
            
            /* Multiple exit points */
            if (sum > 10000) return sum;
        }
    } else {
        /* Different loop in false branch - disjoint from true branch loop */
        int k = 0;
        while (k < vm) {
            sum += k * 3;
            k++;
            
            /* Complex control flow */
            if (k % 7 == 0) {
                sum += 7;
                goto while_label;
            }
            
            sum += 1;
        while_label:
            sum += 2;
        }
    }
    
    return sum;
}

/* Function F: Triple nested loops for complex bitmap relationships */
NOINLINE int test_triple_nested(int n, int m, int p) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    volatile int vp = p;
    
    /* Outer loop */
    for (int i = 0; i < vn; ++i) {
        sum += i;
        
        /* Middle loop */
        for (int j = 0; j < vm; ++j) {
            sum += j * 2;
            
            /* Innermost loop */
            for (int k = 0; k < vp; ++k) {
                sum += k * 3;
                
                /* Multiple conditions creating many basic blocks */
                if (k % 2 == 0) {
                    if (k % 4 == 0) {
                        sum += 4;
                        continue;
                    }
                    sum += 2;
                } else {
                    sum += 1;
                }
                
                /* Another conditional */
                if (k == j && j == i) {
                    sum += 100;
                }
            }
            
            /* Block in middle loop but not in innermost */
            if (j % 3 == 0) {
                sum += 33;
            }
        }
        
        /* Block in outer loop only */
        if (i % 5 == 0) {
            sum += 55;
        }
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line args for variability, but keep small for fast execution */
    int base = (argc > 1) ? atoi(argv[1]) % 10 + 5 : 8;
    
    printf("Testing hardware loop patterns...\n");
    
    /* Test 1: Simple nested loops */
    result += test_nested_simple(base, base + 2);
    printf("Test 1 result: %d\n", result);
    
    /* Test 2: Complex nested loops */
    result += test_nested_complex(base + 1, base + 3);
    printf("Test 2 result: %d\n", result);
    
    /* Test 3: Disjoint sequential loops */
    result += test_disjoint_loops(base + 2, base + 4);
    printf("Test 3 result: %d\n", result);
    
    /* Test 4: Loop with switch inside */
    result += test_switch_in_loop(base + 3, base + 1);
    printf("Test 4 result: %d\n", result);
    
    /* Test 5: Conditional loops */
    result += test_conditional_loops(base + 4, base + 2, base % 2);
    printf("Test 5 result: %d\n", result);
    
    /* Test 6: Triple nested loops */
    result += test_triple_nested(base, base + 1, base + 2);
    printf("Test 6 result: %d\n", result);
    
    printf("Final checksum: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : : "r"(result));
    
    return result != 0 ? 0 : 1;
}
