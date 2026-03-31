/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test function 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_copy_compare_zero(volatile int cond) {
    int result;
    
    /* Create phi node at merge point */
    if (cond > 0) {
        result = 1;  /* True path */
    } else {
        result = 0;  /* False path */
    }
    
    /* Create copy propagation chain */
    int a = result;  /* First copy */
    int b = a;       /* Second copy */
    int c = b;       /* Third copy */
    int d = c;       /* Fourth copy */
    
    /* Conditional branch comparing final copy to 0 */
    if (d == 0) {
        use(100);  /* Taken when phi result was 0 */
        return 1;
    } else {
        use(200);  /* Taken when phi result was 1 */
        return 2;
    }
}

/* Test function 2: Phi with longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_long_chain_compare_one(volatile int cond1, volatile int cond2) {
    int val;
    
    /* More complex phi with 3 incoming edges */
    if (cond1 > 100) {
        val = 1;
    } else if (cond2 > 50) {
        val = 0;
    } else {
        val = 1;
    }
    
    /* Longer copy chain */
    int t1 = val;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;
    int t6 = t5;
    
    /* Compare to 1 */
    if (t6 == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test function 3: Phi in loop context */
__attribute__((noinline, noipa))
int test_phi_in_loop(volatile int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations && i < 10; i++) {
        int flag;
        
        /* Phi node inside loop */
        if (g_seed + i > 5) {
            flag = 1;
        } else {
            flag = 0;
        }
        
        /* Copy chain */
        int f1 = flag;
        int f2 = f1;
        
        /* Conditional branch */
        if (f2 == 0) {
            sum += i * 2;
        } else {
            sum += i * 3;
        }
    }
    
    return sum;
}

/* Test function 4: Nested phi nodes */
__attribute__((noinline, noipa))
int test_nested_phi(volatile int a, volatile int b) {
    int x, y;
    
    /* First phi */
    if (a > 0) {
        x = 1;
    } else {
        x = 0;
    }
    
    /* Second phi dependent on first */
    if (b > 0) {
        y = x;  /* Propagate phi result */
    } else {
        y = 1 - x;
    }
    
    /* Copy chain */
    int z = y;
    int w = z;
    
    /* Compare to 1 */
    if (w == 1) {
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test function 5: Phi with boolean operations */
__attribute__((noinline, noipa))
int test_phi_with_bool_ops(volatile int p1, volatile int p2) {
    int b1, b2;
    
    /* Two independent phis */
    if (p1 > 0) {
        b1 = 1;
    } else {
        b1 = 0;
    }
    
    if (p2 > 0) {
        b2 = 1;
    } else {
        b2 = 0;
    }
    
    /* Combine with boolean operation */
    int combined = b1 & b2;
    
    /* Copy chain */
    int c1 = combined;
    int c2 = c1;
    
    /* Compare to 0 */
    if (c2 == 0) {
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Test function 6: Switch-based phi */
__attribute__((noinline, noipa))
int test_switch_phi(volatile int selector) {
    int value;
    
    switch (selector % 4) {
        case 0:
            value = 0;
            break;
        case 1:
            value = 1;
            break;
        case 2:
            value = 0;
            break;
        case 3:
            value = 1;
            break;
        default:
            value = 0;
    }
    
    /* Copy chain */
    int v1 = value;
    int v2 = v1;
    int v3 = v2;
    
    /* Compare to 1 */
    if (v3 == 1) {
        use(900);
        return 9;
    } else {
        use(1000);
        return 10;
    }
}

/* Dummy implementation of external functions to avoid linker errors */
void use(int x) {
    /* Empty - just to prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(void* p) {
    /* Empty */
    static volatile void* sink;
    sink = p;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile seed */
    g_seed = 42;
    
    /* Call all test functions with varying inputs */
    checksum += test_phi_copy_compare_zero(g_seed);
    checksum += test_phi_long_chain_compare_one(g_seed, g_seed + 1);
    checksum += test_phi_in_loop(g_seed % 5 + 3);
    checksum += test_nested_phi(g_seed - 20, g_seed + 20);
    checksum += test_phi_with_bool_ops(g_seed % 3, g_seed % 7);
    checksum += test_switch_phi(g_seed);
    
    /* Also test with different values */
    g_seed = 123;
    checksum += test_phi_copy_compare_zero(g_seed);
    checksum += test_phi_long_chain_compare_one(g_seed, 0);
    
    g_seed = -5;
    checksum += test_phi_in_loop(8);
    checksum += test_nested_phi(g_seed, 100);
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
