/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(int*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test function 1: Simple phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline,noipa))
int test_phi_simple_copy_compare_zero(volatile int cond) {
    int val;
    if (cond > 0) {
        val = 1;  /* Branch 1: constant 1 */
    } else {
        val = 0;  /* Branch 2: constant 0 */
    }
    /* Copy chain */
    int a = val;
    int b = a;
    
    /* Conditional branch comparing copy to 0 */
    if (b == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test function 2: Longer copy chain, compare to 1 */
__attribute__((noinline,noipa))
int test_phi_long_chain_compare_one(volatile int cond) {
    int val;
    if (cond & 1) {
        val = 0;  /* Constant 0 */
    } else {
        val = 1;  /* Constant 1 */
    }
    
    /* Longer copy propagation chain */
    int t1 = val;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;
    
    /* Compare to 1 (not equal) */
    if (t5 != 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test function 3: Phi with three incoming edges */
__attribute__((noinline,noipa))
int test_phi_three_way(volatile int cond) {
    int val;
    if (cond < 0) {
        val = 1;
    } else if (cond == 0) {
        val = 0;
    } else {
        val = 1;  /* Same as first branch to test phi merging */
    }
    
    /* Copy through pointer dereference to create additional SSA complexity */
    int copy = val;
    int *ptr = &copy;
    int deref = *ptr;
    
    if (deref == 0) {
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test function 4: Nested control flow with phi */
__attribute__((noinline,noipa))
int test_phi_nested(volatile int cond1, volatile int cond2) {
    int val;
    if (cond1 > 0) {
        if (cond2 > 0) {
            val = 1;
        } else {
            val = 0;
        }
    } else {
        val = 0;
    }
    
    /* Multiple independent copies */
    int x = val;
    int y = x;
    int z = y;
    
    /* Compare to 0 with else-if chain */
    if (z == 0) {
        use(700);
        return 7;
    } else if (z == 1) {
        use(800);
        return 8;
    }
    return 9;
}

/* Test function 5: Phi in loop with copy chain */
__attribute__((noinline,noipa))
int test_phi_in_loop(volatile int iterations) {
    int sum = 0;
    volatile int limit = (iterations % 10) + 1;  /* Ensure at least 1 iteration */
    
    for (int i = 0; i < limit; i++) {
        int val;
        if (i & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain inside loop */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Conditional based on copy */
        if (tmp2 == 0) {
            sum += i * 2;
        } else {
            sum += i * 3;
        }
    }
    use(sum);
    return sum;
}

/* Test function 6: Multiple phis feeding into final conditional */
__attribute__((noinline,noipa))
int test_multiple_phis(volatile int cond) {
    int val1, val2;
    
    /* First phi */
    if (cond & 1) {
        val1 = 1;
    } else {
        val1 = 0;
    }
    
    /* Second phi */
    if (cond & 2) {
        val2 = 0;
    } else {
        val2 = 1;
    }
    
    /* Combine phis */
    int combined = val1 & val2;
    int copy1 = combined;
    int copy2 = copy1;
    
    if (copy2 == 0) {
        use(900);
        return 10;
    } else {
        use(1000);
        return 11;
    }
}

/* Test function 7: Phi with boolean constants from comparisons */
__attribute__((noinline,noipa))
int test_phi_from_comparison(volatile int a, volatile int b) {
    int result;
    
    /* Create boolean values from comparisons */
    int cmp1 = (a > b);
    int cmp2 = (a == b);
    
    if (cmp1) {
        result = 1;
    } else if (cmp2) {
        result = 0;
    } else {
        result = 1;
    }
    
    /* Long copy chain */
    int c1 = result;
    int c2 = c1;
    int c3 = c2;
    int c4 = c3;
    int c5 = c4;
    int c6 = c5;
    
    if (c6 != 0) {
        use(1100);
        return 12;
    }
    use(1200);
    return 13;
}

/* Dummy implementation of external functions to avoid linker errors */
void use(int x) {
    /* Empty - just to prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(int* p) {
    static volatile int sink;
    if (p) sink = *p;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int seed = g_seed;
    if (seed == 0) {
        seed = 12345;  /* Default seed if global wasn't set */
    }
    
    /* Call all test functions with varying inputs */
    checksum += test_phi_simple_copy_compare_zero(seed);
    checksum += test_phi_long_chain_compare_one(seed + 1);
    checksum += test_phi_three_way(seed - 1);
    checksum += test_phi_nested(seed, seed + 1);
    checksum += test_phi_in_loop(seed);
    checksum += test_multiple_phis(seed | 0x55);
    checksum += test_phi_from_comparison(seed, seed * 2);
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
