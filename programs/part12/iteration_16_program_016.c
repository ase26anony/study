/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int) __attribute__((noinline));
void use(int x) {
    /* Empty implementation - just to create side effects */
}

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test function 1: Basic phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_basic_compare_zero(int param) {
    int result;
    
    /* Create phi node with 0/1 constants */
    if (param > 0) {
        result = 1;  /* Constant 1 */
    } else {
        result = 0;  /* Constant 0 */
    }
    
    /* Single copy propagation */
    int copy1 = result;
    
    /* Conditional branch comparing copy to 0 */
    if (copy1 == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test function 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_long_chain_compare_one(int param) {
    int val;
    
    /* Phi with constants 0 and 1 */
    if (param % 2 == 0) {
        val = 0;
    } else {
        val = 1;
    }
    
    /* Create chain of 3 copy assignments */
    int tmp1 = val;
    int tmp2 = tmp1;
    int tmp3 = tmp2;
    
    /* Compare final copy to constant 1 */
    if (tmp3 == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test function 3: Phi with multiple incoming edges (3-way) */
__attribute__((noinline, noipa))
int test_phi_three_way(int param) {
    int x;
    
    /* Three-way branch creating phi with 3 incoming constants */
    if (param < -10) {
        x = 0;
    } else if (param < 10) {
        x = 1;
    } else {
        x = 0;  /* Another 0 constant */
    }
    
    /* Copy chain */
    int a = x;
    int b = a;
    
    /* Compare to 0 */
    if (b == 0) {
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test function 4: Nested loops with phi-copy-conditional */
__attribute__((noinline, noipa))
int test_phi_in_loop(int iterations) {
    int sum = 0;
    volatile int vol = g_seed;  /* Prevent loop unrolling */
    
    for (int i = 0; i < iterations; i++) {
        int flag;
        
        /* Phi node inside loop */
        if (vol & 1) {
            flag = 1;
        } else {
            flag = 0;
        }
        
        /* Copy propagation */
        int f1 = flag;
        int f2 = f1;
        
        /* Conditional based on copy */
        if (f2 == 1) {
            sum += i * 2;
            use(700 + i);
        } else {
            sum += i;
            use(800 + i);
        }
        
        vol++;  /* Change volatile to vary branch */
    }
    
    return sum;
}

/* Test function 5: Phi with copy chain and != comparison */
__attribute__((noinline, noipa))
int test_phi_compare_not_equal(int param) {
    int cond;
    
    /* Create phi */
    if (param > 100) {
        cond = 1;
    } else {
        cond = 0;
    }
    
    /* Longer copy chain (4 copies) */
    int c1 = cond;
    int c2 = c1;
    int c3 = c2;
    int c4 = c3;
    
    /* Compare to 0 with != operator */
    if (c4 != 0) {  /* Equivalent to c4 == 1 */
        use(900);
        return 7;
    } else {
        use(1000);
        return 8;
    }
}

/* Test function 6: Multiple phis feeding into each other */
__attribute__((noinline, noipa))
int test_multiple_phis(int param1, int param2) {
    int a, b;
    
    /* First phi */
    if (param1 > 0) {
        a = 1;
    } else {
        a = 0;
    }
    
    /* Second phi */
    if (param2 > 0) {
        b = 1;
    } else {
        b = 0;
    }
    
    /* Combine them */
    int combined = a & b;  /* Creates another phi-like structure */
    
    /* Copy */
    int check = combined;
    
    /* Compare to 0 */
    if (check == 0) {
        use(1100);
        return 9;
    } else {
        use(1200);
        return 10;
    }
}

int main(void) {
    int checksum = 0;
    volatile int seed = g_seed;
    
    /* Call test functions with varying parameters to exercise different paths */
    checksum += test_phi_basic_compare_zero(seed);
    checksum += test_phi_basic_compare_zero(-seed);
    
    checksum += test_phi_long_chain_compare_one(seed);
    checksum += test_phi_long_chain_compare_one(seed + 1);
    
    checksum += test_phi_three_way(seed - 20);
    checksum += test_phi_three_way(seed);
    checksum += test_phi_three_way(seed + 20);
    
    checksum += test_phi_in_loop(5);
    
    checksum += test_phi_compare_not_equal(seed + 150);
    checksum += test_phi_compare_not_equal(seed);
    
    checksum += test_multiple_phis(seed, seed);
    checksum += test_multiple_phis(seed, -seed);
    
    printf("Checksum: %d\n", checksum);
    
    /* Also test with profile generation */
    for (int i = 0; i < 100; i++) {
        test_phi_basic_compare_zero(i);
        test_phi_long_chain_compare_one(i);
        test_phi_in_loop(3);
    }
    
    return 0;
}
