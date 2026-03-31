/* test_autofdo_phi_patterns.c */

#include <stdio.h>
#include <stdint.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global checksum to accumulate results */
static volatile int checksum = 0;

/* Test function 1: Basic phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline, noipa))
void test_basic_phi_compare_0(volatile int cond) {
    int x;
    
    if (cond & 1) {
        x = 0;  /* Branch 1: constant 0 */
    } else {
        x = 1;  /* Branch 2: constant 1 */
    }
    
    /* Copy chain: phi -> a */
    int a = x;
    
    /* Conditional branch comparing copy to 0 */
    if (a == 0) {
        use(100);
        checksum += 100;
    } else {
        use(200);
        checksum += 200;
    }
}

/* Test function 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
void test_longer_chain_compare_1(volatile int cond) {
    int y;
    
    if (cond & 2) {
        y = 1;  /* constant 1 */
    } else {
        y = 0;  /* constant 0 */
    }
    
    /* Longer copy chain: phi -> b -> c -> d */
    int b = y;
    int c = b;
    int d = c;
    
    /* Compare to 1 */
    if (d == 1) {
        use(300);
        checksum += 300;
    } else {
        use(400);
        checksum += 400;
    }
}

/* Test function 3: Phi with three incoming edges */
__attribute__((noinline, noipa))
void test_three_way_phi(volatile int cond) {
    int z;
    
    if (cond % 3 == 0) {
        z = 0;
    } else if (cond % 3 == 1) {
        z = 1;
    } else {
        z = 0;  /* Another 0 to create interesting pattern */
    }
    
    /* Copy chain */
    int t1 = z;
    int t2 = t1;
    
    /* Compare to 0 */
    if (t2 == 0) {
        use(500);
        checksum += 500;
    } else {
        use(600);
        checksum += 600;
    }
}

/* Test function 4: Nested loops with phi-copy-conditional */
__attribute__((noinline, noipa))
void test_loop_pattern(volatile int iter_limit) {
    int limit = (iter_limit & 3) + 2;  /* 2-5 iterations */
    
    for (int i = 0; i < limit; i++) {
        int val;
        
        /* Phi with 0/1 based on loop iteration */
        if (i & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain inside loop */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Conditional inside loop */
        if (tmp2 == 1) {
            checksum += i * 10;
        } else {
            checksum += i * 20;
        }
    }
}

/* Test function 5: Multiple phis feeding into each other */
__attribute__((noinline, noipa))
void test_multi_phi(volatile int cond1, volatile int cond2) {
    int a, b;
    
    /* First phi */
    if (cond1 & 1) {
        a = 0;
    } else {
        a = 1;
    }
    
    /* Second phi */
    if (cond2 & 1) {
        b = 1;
    } else {
        b = 0;
    }
    
    /* Combine them */
    int combined = a & b;
    
    /* Copy chain */
    int copy1 = combined;
    int copy2 = copy1;
    int copy3 = copy2;
    
    /* Final conditional */
    if (copy3 == 0) {
        use(700);
        checksum += 700;
    } else {
        use(800);
        checksum += 800;
    }
}

/* Test function 6: Phi with boolean constants from function arguments */
__attribute__((noinline, noipa))
void test_arg_based_phi(int flag1, int flag2) {
    int result;
    
    if (flag1) {
        result = 0;
    } else if (flag2) {
        result = 1;
    } else {
        result = 0;
    }
    
    /* Long copy chain */
    int r1 = result;
    int r2 = r1;
    int r3 = r2;
    int r4 = r3;
    int r5 = r4;
    
    /* Compare to 1 */
    if (r5 == 1) {
        checksum += 900;
    } else {
        checksum += 1000;
    }
}

/* Test function 7: Switch-based phi pattern */
__attribute__((noinline, noipa))
void test_switch_phi(volatile int selector) {
    int value;
    
    switch (selector & 3) {
        case 0:
            value = 0;
            break;
        case 1:
            value = 1;
            break;
        case 2:
            value = 0;
            break;
        default:
            value = 1;
            break;
    }
    
    /* Minimal copy chain */
    int v = value;
    
    /* Conditional with != comparison (should still create GIMPLE_COND) */
    if (v != 0) {  /* Equivalent to v == 1 */
        checksum += 1100;
    } else {
        checksum += 1200;
    }
}

/* Test function 8: Do-while loop with phi */
__attribute__((noinline, noipa))
void test_do_while_phi(volatile int start) {
    int i = start & 1;
    int count = 0;
    
    do {
        int flag;
        
        /* Phi inside loop */
        if (i) {
            flag = 1;
        } else {
            flag = 0;
        }
        
        /* Copy and conditional */
        int f = flag;
        if (f == 1) {
            count++;
        }
        
        i = !i;
    } while (count < 3);
    
    checksum += count * 100;
}

/* Test function 9: Complex control flow with multiple joins */
__attribute__((noinline, noipa))
void test_complex_cfg(volatile int x, volatile int y) {
    int result;
    
    if (x > 0) {
        if (y > 0) {
            result = 1;
        } else {
            result = 0;
        }
    } else {
        if (y < 0) {
            result = 1;
        } else {
            result = 0;
        }
    }
    
    /* Multi-step copy chain */
    int r = result;
    int s = r;
    int t = s;
    
    if (t == 1) {
        checksum += 1300;
    } else {
        checksum += 1400;
    }
}

/* Test function 10: Final test with all patterns combined */
__attribute__((noinline, noipa))
void test_combined(volatile int seed) {
    /* Mix of different patterns in one function */
    for (int i = 0; i < 2; i++) {
        int val;
        
        if ((seed + i) & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        int copy1 = val;
        int copy2 = copy1;
        
        if (copy2 == 0) {
            checksum += 1500 + i;
        } else {
            checksum += 1600 + i;
        }
        
        /* Nested conditional */
        if (i == 0) {
            int nested_val = (val == 0) ? 1 : 0;
            int ncopy = nested_val;
            if (ncopy == 1) {
                checksum += 10;
            }
        }
    }
}

/* Dummy use function to prevent optimization */
void use(int x) {
    /* Empty - just to prevent optimization */
    checksum += x % 7;
}

int main(void) {
    volatile int seed = 42;
    
    /* Call all test functions with different inputs */
    test_basic_phi_compare_0(seed);
    test_longer_chain_compare_1(seed);
    test_three_way_phi(seed);
    test_loop_pattern(seed);
    test_multi_phi(seed, seed + 1);
    test_arg_based_phi(seed & 1, (seed >> 1) & 1);
    test_switch_phi(seed);
    test_do_while_phi(seed);
    test_complex_cfg(seed, seed - 20);
    test_combined(seed);
    
    /* Print checksum to ensure all code paths are executed */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
