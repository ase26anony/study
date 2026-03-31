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
int test_phi_copy_chain_compare_zero(int param) {
    int result;
    
    /* Create phi node at merge point */
    if (param > 0) {
        result = 1;  /* Constant 1 */
    } else {
        result = 0;  /* Constant 0 */
    }
    
    /* Copy propagation chain */
    int a = result;  /* First copy */
    int b = a;       /* Second copy */
    int c = b;       /* Third copy - final in chain */
    
    /* Conditional branch comparing to 0 */
    if (c == 0) {
        use(100);  /* Branch taken when c == 0 */
        return 1;
    } else {
        use(200);  /* Branch taken when c != 0 */
        return 2;
    }
}

/* Test function 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_longer_chain_compare_one(int param) {
    int val;
    
    /* Different branch structure */
    if (param % 2 == 0) {
        val = 0;
    } else {
        val = 1;
    }
    
    /* Longer copy chain */
    int t1 = val;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;  /* 5-level chain */
    
    /* Compare to 1 */
    if (t5 == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test function 3: Phi with three incoming edges */
__attribute__((noinline, noipa))
int test_phi_three_way(int param) {
    int x;
    
    /* Three-way branch creating phi with 3 operands */
    if (param < -10) {
        x = 0;
    } else if (param > 10) {
        x = 1;
    } else {
        x = 0;  /* Same constant as first branch */
    }
    
    /* Copy chain */
    int y = x;
    int z = y;
    
    /* Conditional */
    if (z != 0) {  /* Compare to 0 with != operator */
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test function 4: Nested loops with phi-copy-conditional */
__attribute__((noinline, noipa))
int test_with_loop(int iterations) {
    int sum = 0;
    volatile int vol = g_seed;  /* Use volatile to prevent optimization */
    
    for (int i = 0; i < iterations; i++) {
        int flag;
        
        /* Phi node inside loop */
        if (vol > i) {
            flag = 1;
        } else {
            flag = 0;
        }
        
        /* Copy chain */
        int f1 = flag;
        int f2 = f1;
        
        /* Conditional inside loop */
        if (f2 == 1) {
            sum += i * 2;
            use(700 + i);
        } else {
            sum += i;
            use(800 + i);
        }
        
        /* Modify volatile to change branch behavior */
        vol = vol ^ i;
    }
    
    return sum;
}

/* Test function 5: Multiple phi nodes feeding into each other */
__attribute__((noinline, noipa))
int test_nested_phi(int param1, int param2) {
    int cond1, cond2;
    
    /* First phi */
    if (param1 > 0) {
        cond1 = 1;
    } else {
        cond1 = 0;
    }
    
    /* Second phi */
    if (param2 > 0) {
        cond2 = 1;
    } else {
        cond2 = 0;
    }
    
    /* Combine them */
    int combined = (cond1 && cond2) ? 1 : 0;
    
    /* Copy chain */
    int c1 = combined;
    int c2 = c1;
    
    /* Conditional */
    if (c2 == 0) {
        use(900);
        return 9;
    } else {
        use(1000);
        return 10;
    }
}

/* Test function 6: Switch statement creating phi */
__attribute__((noinline, noipa))
int test_switch_phi(int param) {
    int value;
    
    switch (param % 4) {
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
    
    /* Conditional comparing to 1 */
    if (v3 == 1) {
        use(1100);
        return 11;
    } else {
        use(1200);
        return 12;
    }
}

/* Main function that exercises all patterns */
int main() {
    int checksum = 0;
    
    /* Initialize volatile seed */
    g_seed = 42;
    
    /* Call test functions with different parameters */
    checksum += test_phi_copy_chain_compare_zero(g_seed);
    checksum += test_longer_chain_compare_one(g_seed + 1);
    checksum += test_phi_three_way(g_seed - 20);
    checksum += test_with_loop(5);
    checksum += test_nested_phi(g_seed, -g_seed);
    checksum += test_switch_phi(g_seed * 2);
    
    /* Also test with different inputs in loops */
    for (int i = 0; i < 3; i++) {
        checksum += test_phi_copy_chain_compare_zero(i - 1);
        g_seed = (g_seed * 13 + 17) % 100;  /* Modify volatile */
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
