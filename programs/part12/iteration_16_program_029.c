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
        x = 1;  /* Will become phi argument 1 */
    } else {
        x = 0;  /* Will become phi argument 0 */
    }
    
    /* Copy propagation chain */
    int a = x;   /* First copy */
    int b = a;   /* Second copy */
    int c = b;   /* Third copy - final in chain */
    
    /* Conditional branch comparing copy to 0 */
    if (c == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Phi with longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_long_chain_compare_one(volatile int cond1, volatile int cond2) {
    int y;
    if (cond1 > 0) {
        if (cond2 > 0) {
            y = 1;
        } else {
            y = 0;
        }
    } else {
        y = 0;  /* Creates phi with 3 operands potentially */
    }
    
    /* Longer copy chain */
    int t1 = y;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;
    
    /* Compare to 1 (not 0) */
    if (t5 == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test 3: Phi in loop context */
__attribute__((noinline, noipa))
int test_phi_in_loop(volatile int iterations) {
    int sum = 0;
    volatile int loop_cond = iterations;
    
    for (int i = 0; i < loop_cond && i < 10; i++) {
        int z;
        if (i % 2 == 0) {
            z = 1;
        } else {
            z = 0;
        }
        
        /* Copy chain inside loop */
        int tmp1 = z;
        int tmp2 = tmp1;
        
        /* Conditional inside loop */
        if (tmp2 == 0) {
            sum += i * 2;
            use(sum);
        } else {
            sum += i * 3;
            use(sum);
        }
    }
    return sum;
}

/* Test 4: Nested phi nodes with copy propagation */
__attribute__((noinline, noipa))
int test_nested_phi(volatile int a, volatile int b) {
    int val1, val2;
    
    /* First phi */
    if (a > 0) {
        val1 = 1;
    } else {
        val1 = 0;
    }
    
    /* Second phi dependent on first */
    if (b > 0) {
        val2 = val1;  /* Uses result of first phi */
    } else {
        val2 = 0;
    }
    
    /* Copy chain from second phi */
    int copy1 = val2;
    int copy2 = copy1;
    
    /* Conditional on final copy */
    if (copy2 != 1) {  /* Using != instead of == */
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test 5: Switch-based phi with constants */
__attribute__((noinline, noipa))
int test_switch_phi(volatile int selector) {
    int result;
    
    switch (selector % 4) {
        case 0: result = 0; break;
        case 1: result = 1; break;
        case 2: result = 0; break;
        default: result = 1; break;
    }
    
    /* Minimal copy chain */
    int final = result;
    
    /* Compare to 0 */
    if (final == 0) {
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Test 6: Multiple incoming edges to phi (3 branches) */
__attribute__((noinline, noipa))
int test_multiway_phi(volatile int mode) {
    int value;
    
    if (mode < -5) {
        value = 0;
    } else if (mode > 5) {
        value = 1;
    } else {
        value = 0;  /* Third incoming edge with same value as first */
    }
    
    /* Two-step copy chain */
    int v1 = value;
    int v2 = v1;
    
    /* Conditional branch */
    if (v2 == 1) {
        use(900);
        return 9;
    } else {
        use(1000);
        return 10;
    }
}

/* Main driver that executes all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize volatile seed */
    volatile int seed = g_seed;
    if (seed == 0) {
        seed = 1;  /* Ensure non-zero for some branches */
    }
    
    /* Run all test patterns */
    checksum += test_phi_copy_compare_zero(seed);
    checksum += test_phi_long_chain_compare_one(seed, seed * 2);
    checksum += test_phi_in_loop(seed + 3);
    checksum += test_nested_phi(seed, -seed);
    checksum += test_switch_phi(seed);
    checksum += test_multiway_phi(seed * 3);
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional runs with different seeds to exercise different paths */
    for (int i = 0; i < 3; i++) {
        g_seed = i;
        test_phi_copy_compare_zero(g_seed);
        test_phi_long_chain_compare_one(g_seed, g_seed);
    }
    
    return 0;
}

/* Dummy implementation of external functions to avoid linker errors */
void use(int x) {
    /* Empty - just to prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(void* p) {
    /* Empty */
    (void)p;
}
