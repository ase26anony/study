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
    
    /* Outer loop - will have its own header, body, latch blocks */
    for (int i = 0; i < vn; ++i) {
        sum += i;
        
        /* Inner loop with multiple basic blocks due to if/continue */
        for (int j = 0; j < vm; ++j) {
            /* This creates two basic blocks in the inner loop */
            if (j % 3 == 0) {
                sum += j * 2;
                continue;  /* Creates separate exit path */
            }
            sum += j;
            
            /* Additional basic block complexity */
            if (j == vm - 1) {
                sum += 100;  /* Creates another block */
            }
        }
        
        /* Block after inner loop - part of outer loop only */
        if (i % 2 == 0) {
            sum -= 5;
        }
    }
    return sum;
}

/* Function B: Nested loops with shared header-like structure */
NOINLINE int test_nested_shared_header(int n, int m) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    int i = 0;
    
    /* do-while outer loop */
    do {
        /* This block is shared conceptually - creates intersection complexity */
        int temp = i * 2;
        
        /* Inner for loop */
        for (int j = 0; j < vm; ++j) {
            sum += temp + j;
            
            /* Multiple exit points create more blocks */
            if (sum > 1000) {
                sum -= 50;  /* Creates separate block */
            }
            
            /* Another conditional */
            if (j % 4 == 0) {
                continue;  /* Creates continue block */
            }
        }
        
        /* Conditional that could be shared but isn't */
        if (i % 3 == 0) {
            sum += 7;
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
    
    /* First loop with its own blocks */
    for (int i = 0; i < vn; ++i) {
        sum += i * 2;
        
        /* Conditional creates multiple blocks */
        if (i % 5 == 0) {
            sum += 10;
            /* No break/continue - just different block */
        }
    }
    
    /* Completely separate second loop - no block intersection */
    for (int j = 0; j < vm; ++j) {
        sum -= j;
        
        /* Different conditional structure */
        switch (j % 3) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 2: sum += 3; break;
        }
    }
    
    return sum;
}

/* Function D: Loop with internal switch wrapped in outer loop */
NOINLINE int test_switch_in_nested(int n, int m) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    
    /* Outer wrapper loop - runs fixed number of times */
    for (int outer = 0; outer < 3; ++outer) {
        sum += outer * 100;
        
        /* Inner loop with switch - creates many basic blocks */
        for (int i = 0; i < vn; ++i) {
            /* Switch creates multiple case blocks */
            switch (i % 5) {
                case 0:
                    sum += i;
                    break;
                case 1:
                    sum += i * 2;
                    /* Fall through */
                case 2:
                    sum += i * 3;
                    if (sum > 500) {
                        sum -= 20;  /* Creates nested if block */
                    }
                    break;
                case 3:
                    sum += i * 4;
                    continue;  /* Creates continue block */
                case 4:
                    sum += i * 5;
                    /* Multiple statements in case */
                    for (int k = 0; k < 2; ++k) {
                        sum += k;
                    }
                    break;
            }
            
            /* Additional block after switch */
            if (i == vn - 1) {
                sum += 99;
            }
        }
    }
    
    return sum;
}

/* Function E: Conditional loop nesting - disjoint loops in branches */
NOINLINE int test_conditional_disjoint(int n, int m, int flag) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    volatile int vflag = flag;
    
    /* Outer conditional */
    if (vflag) {
        /* Loop in true branch */
        for (int i = 0; i < vn; ++i) {
            sum += i * 3;
            
            /* Multiple exit points */
            if (sum > 1000) {
                return sum;  /* Early return creates exit block */
            }
            
            if (i % 7 == 0) {
                continue;
            }
        }
        
        /* Additional code in true branch */
        sum += 50;
    } else {
        /* Different loop in false branch - completely disjoint blocks */
        int j = vm;
        while (j-- > 0) {
            sum -= j * 2;
            
            /* Nested if creates blocks */
            if (j % 4 == 0) {
                sum += 25;
                /* Potential break */
                if (sum < -100) {
                    break;
                }
            }
        }
        
        /* Different post-loop code */
        sum += 75;
    }
    
    return sum;
}

/* Function F: Complex nested structure with multiple inner loops */
NOINLINE int test_multi_inner(int n, int m, int p) {
    int sum = 0;
    volatile int vn = n;
    volatile int vm = m;
    volatile int vp = p;
    
    /* Outer loop */
    for (int i = 0; i < vn; ++i) {
        sum += i;
        
        /* First inner loop */
        for (int j = 0; j < vm; ++j) {
            sum += j;
            
            /* Conditional with goto creates interesting flow */
            if (j % 6 == 0) {
                sum += 6;
                goto inner_label;  /* Creates goto block */
            }
            sum += 1;
            
        inner_label:
            /* Empty label block */
            ;
        }
        
        /* Second inner loop at same nesting level */
        for (int k = 0; k < vp; ++k) {
            sum -= k;
            
            /* Switch with returns */
            switch (k % 4) {
                case 0: sum += 10; break;
                case 1: sum += 20; 
                        if (sum > 200) continue;
                        break;
                case 2: sum += 30; break;
                case 3: sum += 40; 
                        /* Nested loop */
                        for (int l = 0; l < 2; ++l) {
                            sum += l;
                        }
                        break;
            }
        }
    }
    
    return sum;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use command line args for variability, but provide defaults */
    int n = (argc > 1) ? atoi(argv[1]) % 20 + 5 : 10;
    int m = (argc > 2) ? atoi(argv[2]) % 15 + 3 : 8;
    int p = (argc > 3) ? atoi(argv[3]) % 10 + 2 : 5;
    int flag = (argc > 4) ? atoi(argv[4]) % 2 : 1;
    
    printf("Testing hardware loop patterns with n=%d, m=%d, p=%d, flag=%d\n", 
           n, m, p, flag);
    
    /* Call all test functions */
    total += test_nested_simple(n, m);
    total += test_nested_shared_header(n, m);
    total += test_sequential_disjoint(n, m);
    total += test_switch_in_nested(n, m);
    total += test_conditional_disjoint(n, m, flag);
    total += test_multi_inner(n, m, p);
    
    /* Use result to prevent dead code elimination */
    printf("Total checksum: %d\n", total);
    
    /* Additional volatile use to ensure loops aren't optimized away */
    volatile int dummy = total;
    
    return (dummy > 0) ? 0 : 1;
}
