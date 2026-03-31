/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int) __attribute__((noinline));
void use(int x) {
    /* Prevent optimization by using volatile */
    static volatile int sink;
    sink = x;
}

/* Global checksum to ensure all paths are executed */
static volatile int checksum = 0;

/* Test 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline, noipa))
void test1(volatile int cond) {
    int a, b, c, d;
    
    if (cond & 1) {
        a = 0;  /* First phi argument */
    } else {
        a = 1;  /* Second phi argument */
    }
    
    /* Copy propagation chain */
    b = a;  /* GIMPLE_ASSIGN with SSA_NAME RHS */
    c = b;  /* Another copy */
    d = c;  /* Final copy */
    
    /* Conditional branch comparing to constant 0 */
    if (d == 0) {
        use(100);
        checksum += 100;
    } else {
        use(200);
        checksum += 200;
    }
}

/* Test 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
void test2(volatile int cond) {
    int x, t1, t2, t3, t4, t5;
    
    if (cond & 2) {
        x = 1;
    } else {
        x = 0;
    }
    
    /* Longer copy chain */
    t1 = x;
    t2 = t1;
    t3 = t2;
    t4 = t3;
    t5 = t4;
    
    /* Compare to constant 1 */
    if (t5 == 1) {
        use(300);
        checksum += 300;
    } else {
        use(400);
        checksum += 400;
    }
}

/* Test 3: Phi with three incoming edges */
__attribute__((noinline, noipa))
void test3(volatile int cond) {
    int val, a, b, c;
    
    if (cond & 1) {
        val = 0;
    } else if (cond & 2) {
        val = 1;
    } else {
        val = 0;  /* Third phi argument */
    }
    
    a = val;
    b = a;
    c = b;
    
    if (c != 0) {  /* Compare with not-equal to 0 */
        use(500);
        checksum += 500;
    } else {
        use(600);
        checksum += 600;
    }
}

/* Test 4: Phi-copy-conditional inside a loop */
__attribute__((noinline, noipa))
void test4(volatile int cond) {
    int i, phi_val, tmp1, tmp2, final;
    volatile int limit = (cond & 3) + 2;  /* 2-5 iterations */
    
    for (i = 0; i < limit; i++) {
        /* Phi node at loop header */
        if (i & 1) {
            phi_val = 1;
        } else {
            phi_val = 0;
        }
        
        /* Copy chain */
        tmp1 = phi_val;
        tmp2 = tmp1;
        final = tmp2;
        
        /* Conditional inside loop */
        if (final == 1) {
            use(700 + i);
            checksum += 7;
        } else {
            use(800 + i);
            checksum += 8;
        }
    }
}

/* Test 5: Nested control flow with phi nodes */
__attribute__((noinline, noipa))
void test5(volatile int cond1, volatile int cond2) {
    int x, y, z, w;
    
    if (cond1 & 1) {
        if (cond2 & 1) {
            x = 1;
        } else {
            x = 0;
        }
    } else {
        x = 0;
    }
    
    y = x;
    z = y;
    w = z;
    
    if (w == 0) {
        use(900);
        checksum += 900;
    } else {
        use(1000);
        checksum += 1000;
    }
}

/* Test 6: Multiple phi nodes feeding into each other */
__attribute__((noinline, noipa))
void test6(volatile int cond) {
    int a, b, c, d, e, f;
    
    /* First phi */
    if (cond & 1) {
        a = 1;
    } else {
        a = 0;
    }
    
    b = a;
    
    /* Second phi using b */
    if (cond & 2) {
        c = b;
    } else {
        c = 0;
    }
    
    d = c;
    e = d;
    f = e;
    
    if (f == 1) {
        use(1100);
        checksum += 1100;
    } else {
        use(1200);
        checksum += 1200;
    }
}

int main(void) {
    volatile int seed = 0x1234;
    
    /* Execute all test patterns */
    test1(seed);
    test2(seed);
    test3(seed);
    test4(seed);
    test5(seed, seed >> 1);
    test6(seed);
    
    /* Print checksum to ensure all code executed */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
