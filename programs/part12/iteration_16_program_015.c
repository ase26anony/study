/* Test program for GCC AutoFDO profile reading coverage
 * Specifically targets phi nodes with boolean constants -> copy chains -> conditional branches
 */

#include <stdio.h>
#include <stdint.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(int*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Checksum accumulator */
static int checksum = 0;

/* Test 1: Basic phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline, noipa))
void test1_basic_phi_compare_zero(int cond) {
    int val;
    
    /* Create phi node with constants 0 and 1 */
    if (cond > 0) {
        val = 1;  /* Constant 1 */
    } else {
        val = 0;  /* Constant 0 */
    }
    
    /* Single copy assignment */
    int copy1 = val;
    
    /* Conditional branch comparing copy to 0 */
    if (copy1 == 0) {
        use(100);
        checksum += 100;
    } else {
        use(200);
        checksum += 200;
    }
}

/* Test 2: Multi-copy chain, compare to 1 */
__attribute__((noinline, noipa))
void test2_multi_copy_compare_one(int cond) {
    int val;
    
    /* Phi with reversed constants */
    if (cond <= 0) {
        val = 0;
    } else {
        val = 1;
    }
    
    /* Chain of 3 copy assignments */
    int copy1 = val;
    int copy2 = copy1;
    int copy3 = copy2;
    
    /* Compare to 1 (not 0) */
    if (copy3 == 1) {
        use(300);
        checksum += 300;
    } else {
        use(400);
        checksum += 400;
    }
}

/* Test 3: Phi with 3 incoming edges (switch-like) */
__attribute__((noinline, noipa))
void test3_three_way_phi(int cond) {
    int val;
    
    /* Three-way branch creating phi with 3 operands */
    if (cond < -5) {
        val = 0;
    } else if (cond > 5) {
        val = 1;
    } else {
        val = 0;  /* Another 0 constant */
    }
    
    /* Two-level copy chain */
    int tmp1 = val;
    int tmp2 = tmp1;
    
    /* Compare to 0 */
    if (tmp2 != 0) {  /* Using != instead of == */
        use(500);
        checksum += 500;
    } else {
        use(600);
        checksum += 600;
    }
}

/* Test 4: Loop context with phi-copy-conditional */
__attribute__((noinline, noipa))
void test4_loop_context(void) {
    volatile int limit = g_seed & 0x3;  /* 0-3 iterations */
    if (limit < 1) limit = 1;
    
    for (int i = 0; i < limit; i++) {
        int val;
        
        /* Phi inside loop */
        if (i & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain inside loop */
        int a = val;
        int b = a;
        
        /* Conditional inside loop */
        if (b == 0) {
            checksum += i * 10;
        } else {
            checksum += i * 20;
        }
    }
}

/* Test 5: Nested conditionals creating complex phi */
__attribute__((noinline, noipa))
void test5_nested_phi(int cond1, int cond2) {
    int val;
    
    /* More complex control flow */
    if (cond1 > 0) {
        if (cond2 > 0) {
            val = 1;
        } else {
            val = 0;
        }
    } else {
        val = 0;
    }
    
    /* Longer copy chain (4 copies) */
    int c1 = val;
    int c2 = c1;
    int c3 = c2;
    int c4 = c3;
    
    /* Final conditional */
    if (c4 == 1) {
        use(700);
        checksum += 700;
    } else {
        use(800);
        checksum += 800;
    }
}

/* Test 6: Phi with same constant on multiple paths */
__attribute__((noinline, noipa))
void test6_phi_same_constants(int cond) {
    int val;
    
    /* Both branches produce 1, but with different reasons */
    if (cond > 10) {
        val = 1;  /* Constant 1 */
    } else {
        /* Still 1, but from different computation */
        val = (cond >= 0) ? 1 : 1;  /* Always 1 */
    }
    
    /* Copy and compare to 1 */
    int copy = val;
    
    if (copy == 1) {
        checksum += 900;
    } else {
        checksum += 1000;  /* Should never happen */
    }
}

/* Test 7: Pointer-based copy chain */
__attribute__((noinline, noipa))
void test7_pointer_copy(int cond) {
    int val;
    
    if (cond % 2 == 0) {
        val = 0;
    } else {
        val = 1;
    }
    
    /* Copy through pointer dereference */
    int copy1 = val;
    int *ptr = &copy1;
    int copy2 = *ptr;
    
    if (copy2 == 0) {
        checksum += 1100;
    } else {
        checksum += 1200;
    }
}

/* Test 8: Volatile read to prevent optimization */
__attribute__((noinline, noipa))
void test8_volatile_phi(void) {
    volatile int v = g_seed;
    int val;
    
    /* Use volatile in condition */
    if (v > 0) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Multi-step copy chain */
    int x = val;
    int y = x;
    int z = y;
    
    if (z == 1) {
        checksum += 1300;
    } else {
        checksum += 1400;
    }
}

/* Main driver that calls all tests */
int main(void) {
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int seed = 42;
    g_seed = seed;
    
    /* Call each test with different conditions to exercise all paths */
    test1_basic_phi_compare_zero(seed);
    test2_multi_copy_compare_one(seed);
    test3_three_way_phi(seed);
    test4_loop_context();
    test5_nested_phi(seed, -seed);
    test6_phi_same_constants(seed);
    test7_pointer_copy(seed);
    test8_volatile_phi();
    
    /* Additional calls with different parameters */
    test1_basic_phi_compare_zero(-seed);
    test2_multi_copy_compare_one(0);
    test3_three_way_phi(0);
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}

/* Dummy implementation of external functions to avoid linker errors */
void use(int x) {
    checksum += x % 7;  /* Simple non-constant computation */
}

void use_ptr(int *p) {
    if (p) checksum += *p % 11;
}
