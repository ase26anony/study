/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_copy_compare_zero(volatile int cond) {
    int x;
    if (cond > 0) {
        x = 1;  /* Becomes phi argument 1 */
    } else {
        x = 0;  /* Becomes phi argument 0 */
    }
    
    /* Copy propagation chain */
    int a = x;   /* GIMPLE_ASSIGN with SSA_NAME rhs */
    int b = a;   /* Another copy */
    int c = b;   /* Final copy before comparison */
    
    /* Conditional branch comparing copy chain result to 0 */
    if (c == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_copy_compare_one(volatile int cond) {
    int x;
    if (cond & 1) {
        x = 1;
    } else {
        x = 0;
    }
    
    /* Longer copy chain */
    int t1 = x;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;
    
    /* Compare to 1 */
    if (t5 == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test 3: Phi with three incoming edges */
__attribute__((noinline, noipa))
int test_phi_three_way(volatile int cond) {
    int x;
    if (cond > 10) {
        x = 1;
    } else if (cond > 5) {
        x = 0;
    } else {
        x = 1;  /* Different constant to create interesting phi */
    }
    
    int a = x;
    int b = a;
    
    if (b == 0) {
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test 4: Phi in loop context */
__attribute__((noinline, noipa))
int test_phi_in_loop(volatile int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations && i < 10; i++) {
        int x;
        if (i & 1) {
            x = 1;
        } else {
            x = 0;
        }
        
        /* Copy chain inside loop */
        int a = x;
        int b = a;
        
        if (b == 0) {
            sum += i * 2;
        } else {
            sum += i * 3;
        }
    }
    return sum;
}

/* Test 5: Nested phi-copy patterns */
__attribute__((noinline, noipa))
int test_nested_phi(volatile int cond1, volatile int cond2) {
    int x, y;
    
    /* First phi */
    if (cond1 > 0) {
        x = 1;
    } else {
        x = 0;
    }
    
    /* Second phi dependent on first */
    if (cond2 > 0) {
        y = x;  /* y gets phi result through copy */
    } else {
        y = 1 - x;
    }
    
    int a = y;
    int b = a;
    
    if (b == 0) {
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Test 6: Phi with != comparison */
__attribute__((noinline, noipa))
int test_phi_not_equal(volatile int cond) {
    int x;
    if (cond % 3 == 0) {
        x = 1;
    } else {
        x = 0;
    }
    
    int a = x;
    int b = a;
    int c = b;
    
    /* Compare with != instead of == */
    if (c != 1) {
        use(900);
        return 9;
    } else {
        use(1000);
        return 10;
    }
}

/* Test 7: Multiple phi nodes feeding each other */
__attribute__((noinline, noipa))
int test_multi_phi_chain(volatile int cond) {
    int x, y;
    
    if (cond > 20) {
        x = 1;
        y = 0;
    } else {
        x = 0;
        y = 1;
    }
    
    /* Chain of assignments mixing both phi results */
    int a = x;
    int b = y;
    int c = a + b;  /* Still SSA, but now with operation */
    int d = c > 0 ? 1 : 0;  /* Another phi-like conditional */
    
    if (d == 0) {
        use(1100);
        return 11;
    } else {
        use(1200);
        return 12;
    }
}

/* Test 8: Switch-based phi pattern */
__attribute__((noinline, noipa))
int test_switch_phi(volatile int cond) {
    int x;
    switch (cond % 4) {
        case 0: x = 0; break;
        case 1: x = 1; break;
        case 2: x = 0; break;
        default: x = 1; break;
    }
    
    int a = x;
    int b = a;
    
    if (b == 1) {
        use(1300);
        return 13;
    } else {
        use(1400);
        return 14;
    }
}

/* Dummy implementation of external functions */
void use(int val) {
    /* Prevent optimization - could be empty or do minimal work */
    static volatile int sink;
    sink = val;
}

void use_ptr(void* ptr) {
    static volatile void* sink_ptr;
    sink_ptr = ptr;
}

int main() {
    int checksum = 0;
    
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int seed = g_seed;
    if (seed == 0) {
        seed = 1;  /* Ensure non-zero for some tests */
    }
    
    /* Call all test functions with varying inputs */
    checksum += test_phi_copy_compare_zero(seed);
    checksum += test_phi_copy_compare_one(seed + 1);
    checksum += test_phi_three_way(seed + 2);
    checksum += test_phi_in_loop(seed + 3);
    checksum += test_nested_phi(seed + 4, seed + 5);
    checksum += test_phi_not_equal(seed + 6);
    checksum += test_multi_phi_chain(seed + 7);
    checksum += test_switch_phi(seed + 8);
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional runs with different seeds to exercise different paths */
    for (int i = 0; i < 5; i++) {
        volatile int local_seed = seed + i * 10;
        test_phi_copy_compare_zero(local_seed);
        test_phi_copy_compare_one(local_seed + 1);
    }
    
    return 0;
}
