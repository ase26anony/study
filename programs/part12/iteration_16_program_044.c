/* test_autofdo_phi_conditional.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int) __attribute__((noinline));
void use(int x) {
    /* Empty implementation - linker will resolve */
}

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test function 1: Basic phi with 0/1 constants, short copy chain */
__attribute__((noinline, noipa))
int test_phi_basic(volatile int cond) {
    int result;
    
    /* Create phi node with 0/1 constants */
    if (cond > 0) {
        result = 1;  /* Constant 1 in one branch */
    } else {
        result = 0;  /* Constant 0 in other branch */
    }
    
    /* Copy propagation chain */
    int a = result;  /* First copy */
    int b = a;       /* Second copy */
    
    /* Conditional branch on final copy compared to 0 */
    if (b == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test function 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_long_chain(volatile int cond) {
    int val;
    
    /* Phi with reversed constants */
    if (cond % 2 == 0) {
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
int test_phi_three_way(volatile int cond) {
    int x;
    
    /* Three-way phi */
    if (cond < 0) {
        x = 0;
    } else if (cond == 0) {
        x = 1;
    } else {
        x = 0;  /* Same as first branch to test phi merging */
    }
    
    /* Copy chain */
    int y = x;
    int z = y;
    
    /* Conditional with != comparison */
    if (z != 0) {
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test function 4: Nested loops with phi-copy-conditional */
__attribute__((noinline, noipa))
int test_phi_with_loop(volatile int iter) {
    int sum = 0;
    volatile int limit = (iter % 5) + 3;  /* Force runtime evaluation */
    
    for (int i = 0; i < limit; i++) {
        int flag;
        
        /* Phi inside loop */
        if (i % 2 == 0) {
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
    }
    
    return sum;
}

/* Test function 5: Multiple phis feeding into each other */
__attribute__((noinline, noipa))
int test_multi_phi(volatile int cond1, volatile int cond2) {
    int a, b;
    
    /* First phi */
    if (cond1 > 10) {
        a = 1;
    } else {
        a = 0;
    }
    
    /* Second phi */
    if (cond2 < 20) {
        b = 1;
    } else {
        b = 0;
    }
    
    /* Combine results */
    int combined = a & b;  /* Creates another phi-like value */
    
    /* Copy chain */
    int c1 = combined;
    int c2 = c1;
    
    /* Final conditional */
    if (c2 == 0) {
        use(900);
        return 9;
    } else {
        use(1000);
        return 10;
    }
}

/* Test function 6: Switch-based phi generation */
__attribute__((noinline, noipa))
int test_phi_switch(volatile int mode) {
    int state;
    
    switch (mode % 4) {
        case 0:
            state = 0;
            break;
        case 1:
            state = 1;
            break;
        case 2:
            state = 0;
            break;
        case 3:
            state = 1;
            break;
    }
    
    /* Copy propagation */
    int s1 = state;
    int s2 = s1;
    int s3 = s2;
    
    /* Compare to 1 using inequality */
    if (s3 != 1) {
        use(1100);
        return 11;
    } else {
        use(1200);
        return 12;
    }
}

/* Main function to execute all tests */
int main() {
    int checksum = 0;
    
    /* Initialize volatile seed */
    volatile int seed = g_seed;
    if (seed == 0) {
        seed = 1;  /* Ensure non-zero for some branches */
    }
    
    /* Run all test functions */
    checksum += test_phi_basic(seed);
    checksum += test_phi_long_chain(seed + 1);
    checksum += test_phi_three_way(seed - 1);
    checksum += test_phi_with_loop(seed);
    checksum += test_multi_phi(seed, seed * 2);
    checksum += test_phi_switch(seed);
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional runs with different seeds to exercise different paths */
    for (int i = 0; i < 3; i++) {
        g_seed = i;
        test_phi_basic(g_seed);
        test_phi_long_chain(g_seed);
    }
    
    return 0;
}
