/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int) __attribute__((noinline));
void use(int x) {
    /* Empty implementation - just to prevent optimization */
    (void)x;
}

/* Global volatile to force runtime decisions */
volatile int g_seed = 0;

/* Test function 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_copy_chain_compare_zero(int input) {
    int result;
    
    /* Create phi node with constants 0 and 1 */
    if (input > 0) {
        result = 1;  /* Constant 1 */
    } else {
        result = 0;  /* Constant 0 */
    }
    
    /* Create copy propagation chain */
    int a = result;  /* First copy */
    int b = a;       /* Second copy */
    int c = b;       /* Third copy */
    int d = c;       /* Fourth copy */
    
    /* Conditional branch comparing final copy to 0 */
    if (d == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test function 2: Phi with longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_long_chain_compare_one(int input) {
    int val;
    
    /* Different branch structure for phi */
    if (input & 1) {
        val = 0;
    } else if (input & 2) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Longer copy chain */
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

/* Test function 3: Phi in loop context */
__attribute__((noinline, noipa))
int test_phi_in_loop(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        int flag;
        
        /* Phi node inside loop */
        if (i & 1) {
            flag = 1;
        } else {
            flag = 0;
        }
        
        /* Copy chain */
        int x = flag;
        int y = x;
        
        /* Conditional based on copy */
        if (y == 0) {
            sum += i;
            use(i * 10);
        } else {
            sum -= i;
            use(i * 20);
        }
    }
    
    return sum;
}

/* Test function 4: Multiple phi nodes feeding each other */
__attribute__((noinline, noipa))
int test_nested_phi_copies(int a, int b) {
    int cond1, cond2;
    
    /* First phi */
    if (a > 0) {
        cond1 = 1;
    } else {
        cond1 = 0;
    }
    
    /* Second phi */
    if (b > 0) {
        cond2 = 1;
    } else {
        cond2 = 0;
    }
    
    /* Combine phis */
    int combined = (cond1 == 1 && cond2 == 1) ? 1 : 0;
    
    /* Copy chain */
    int tmp1 = combined;
    int tmp2 = tmp1;
    
    /* Conditional */
    if (tmp2 == 1) {
        use(500);
        return 10;
    } else {
        use(600);
        return 20;
    }
}

/* Test function 5: Phi with switch-case pattern */
__attribute__((noinline, noipa))
int test_phi_switch_like(int mode) {
    int state;
    
    switch (mode % 3) {
        case 0: state = 0; break;
        case 1: state = 1; break;
        case 2: state = 0; break;
        default: state = 1; break;
    }
    
    /* Minimal copy chain */
    int s = state;
    
    /* Compare to constant */
    if (s == 0) {
        use(700);
        return 30;
    } else {
        use(800);
        return 40;
    }
}

/* Test function 6: Volatile-based phi to prevent optimization */
__attribute__((noinline, noipa))
int test_volatile_based_phi(void) {
    volatile int v = g_seed;
    int choice;
    
    /* Phi based on volatile read */
    if (v > 100) {
        choice = 1;
    } else {
        choice = 0;
    }
    
    /* Two-step copy chain */
    int c1 = choice;
    int c2 = c1;
    
    /* Conditional branch */
    if (c2 == 1) {
        use(900);
        return 100;
    } else {
        use(1000);
        return 200;
    }
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    /* Initialize volatile seed */
    g_seed = 42;
    
    /* Test 1: Basic pattern */
    checksum += test_phi_copy_chain_compare_zero(g_seed);
    
    /* Test 2: Longer chain */
    checksum += test_phi_long_chain_compare_one(g_seed + 1);
    
    /* Test 3: Loop context */
    checksum += test_phi_in_loop(5);
    
    /* Test 4: Nested phis */
    checksum += test_nested_phi_copies(g_seed, g_seed - 20);
    
    /* Test 5: Switch-like phi */
    checksum += test_phi_switch_like(g_seed);
    
    /* Test 6: Volatile-based */
    checksum += test_volatile_based_phi();
    
    /* Modify volatile to change execution paths */
    g_seed = 150;
    checksum += test_volatile_based_phi();
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
