/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int) __attribute__((noinline));
void use(int x) {
    /* Empty implementation - just to create side effects */
}

/* Global checksum to accumulate results */
static volatile int checksum = 0;

/* Test 1: Basic phi with 0/1 constants, short copy chain, compare to 0 */
__attribute__((noinline, noipa))
void test1(volatile int cond) {
    int x;
    if (cond) {
        x = 0;  /* Will become phi argument 0 */
    } else {
        x = 1;  /* Will become phi argument 1 */
    }
    
    /* Copy propagation chain */
    int a = x;  /* First copy */
    int b = a;  /* Second copy */
    
    /* Conditional branch comparing final copy to 0 */
    if (b == 0) {
        use(100);
        checksum += 1;
    } else {
        use(200);
        checksum += 2;
    }
}

/* Test 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
void test2(volatile int cond) {
    int y;
    if (cond > 0) {
        y = 1;
    } else {
        y = 0;
    }
    
    /* Longer copy chain */
    int t1 = y;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    
    /* Compare to 1 */
    if (t4 == 1) {
        use(300);
        checksum += 4;
    } else {
        use(400);
        checksum += 8;
    }
}

/* Test 3: Phi with three incoming edges */
__attribute__((noinline, noipa))
void test3(volatile int mode) {
    int z;
    if (mode == 1) {
        z = 0;
    } else if (mode == 2) {
        z = 1;
    } else {
        z = 0;  /* Another 0 to create phi with duplicate values */
    }
    
    /* Copy chain */
    int c1 = z;
    int c2 = c1;
    
    /* Compare to 0 using != operator */
    if (c2 != 0) {
        use(500);
        checksum += 16;
    } else {
        use(600);
        checksum += 32;
    }
}

/* Test 4: Nested control flow with phi */
__attribute__((noinline, noipa))
void test4(volatile int cond1, volatile int cond2) {
    int w;
    if (cond1) {
        if (cond2) {
            w = 1;
        } else {
            w = 0;
        }
    } else {
        w = 0;
    }
    
    int d = w;
    
    if (d == 0) {
        use(700);
        checksum += 64;
    } else {
        use(800);
        checksum += 128;
    }
}

/* Test 5: Loop with phi-copy-conditional pattern */
__attribute__((noinline, noipa))
void test5(volatile int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations && i < 10; i++) {
        int val;
        if (i % 2 == 0) {
            val = 0;
        } else {
            val = 1;
        }
        
        /* Copy chain inside loop */
        int e1 = val;
        int e2 = e1;
        
        /* Conditional inside loop */
        if (e2 == 0) {
            total += i;
        } else {
            total -= i;
        }
    }
    
    checksum += total;
    use(total);
}

/* Test 6: Switch statement creating phi */
__attribute__((noinline, noipa))
void test6(volatile int choice) {
    int result;
    
    switch (choice % 3) {
        case 0:
            result = 0;
            break;
        case 1:
            result = 1;
            break;
        case 2:
            result = 0;  /* Another 0 */
            break;
        default:
            result = 1;
    }
    
    int f1 = result;
    int f2 = f1;
    int f3 = f2;
    
    if (f3 == 1) {
        use(900);
        checksum += 256;
    } else {
        use(1000);
        checksum += 512;
    }
}

/* Test 7: Multiple independent phi-copy chains */
__attribute__((noinline, noipa))
void test7(volatile int flag) {
    int x, y;
    
    if (flag) {
        x = 0;
        y = 1;
    } else {
        x = 1;
        y = 0;
    }
    
    /* Two independent copy chains */
    int x1 = x;
    int x2 = x1;
    
    int y1 = y;
    int y2 = y1;
    
    /* Two conditionals */
    if (x2 == 0) {
        checksum += 1024;
    }
    
    if (y2 == 1) {
        checksum += 2048;
    }
    
    use(x2 + y2);
}

/* Test 8: Phi with boolean constants from function arguments */
__attribute__((noinline, noipa))
static int helper(volatile int a, volatile int b) {
    return (a > b) ? 1 : 0;
}

void test8(volatile int p1, volatile int p2) {
    int res = helper(p1, p2);
    
    /* Copy chain */
    int g1 = res;
    int g2 = g1;
    
    if (g2 == 0) {
        checksum += 4096;
        use(1100);
    } else {
        checksum += 8192;
        use(1200);
    }
}

int main(void) {
    volatile int seed = 0;
    
    /* Initialize volatile seed from environment or time */
    seed = getenv("SEED") ? atoi(getenv("SEED")) : 12345;
    
    /* Call all test functions with varying conditions */
    test1(seed % 2);
    test2(seed % 3);
    test3(seed % 4);
    test4(seed % 2, (seed / 2) % 2);
    test5((seed % 5) + 1);
    test6(seed);
    test7(seed % 2);
    test8(seed, seed * 2);
    
    /* Print checksum to ensure all code paths executed */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
