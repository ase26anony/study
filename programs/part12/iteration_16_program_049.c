/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int) __attribute__((noinline));
void use(int x) {
    /* Empty implementation - just to create side effects */
    static volatile int sink;
    sink = x;
}

/* Global checksum to accumulate results */
static volatile int checksum = 0;

/* Test 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline, noipa))
void test1(volatile int cond) {
    int x;
    if (cond) {
        x = 1;  /* Branch 1: constant 1 */
    } else {
        x = 0;  /* Branch 2: constant 0 */
    }
    /* Phi node created here for x */
    
    /* Copy propagation chain */
    int a = x;   /* First copy */
    int b = a;   /* Second copy */
    int c = b;   /* Third copy */
    
    /* Conditional branch comparing final copy to 0 */
    if (c == 0) {
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
        y = 0;
    } else {
        y = 1;
    }
    /* Phi node for y */
    
    /* Longer copy chain */
    int t1 = y;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;
    
    /* Compare to 1 */
    if (t5 == 1) {
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
    if (mode == 0) {
        z = 0;
    } else if (mode == 1) {
        z = 1;
    } else {
        z = 0;  /* Another 0 to create interesting phi */
    }
    /* Phi with 3 arguments: 0, 1, 0 */
    
    /* Copy chain */
    int m = z;
    int n = m;
    
    /* Compare to 0 */
    if (n != 0) {  /* Using != instead of == */
        use(500);
        checksum += 16;
    } else {
        use(600);
        checksum += 32;
    }
}

/* Test 4: Nested control flow with phi */
__attribute__((noinline, noipa))
void test4(volatile int a, volatile int b) {
    int val;
    if (a) {
        if (b) {
            val = 1;
        } else {
            val = 0;
        }
    } else {
        val = 0;
    }
    /* Complex phi structure */
    
    /* Copy through multiple variables */
    int tmp = val;
    int res = tmp;
    
    /* Compare to 1 */
    if (res == 1) {
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
    int sum = 0;
    for (int i = 0; i < iterations && i < 10; i++) {
        int flag;
        if (i % 2 == 0) {
            flag = 1;
        } else {
            flag = 0;
        }
        /* Phi node inside loop */
        
        /* Copy chain */
        int f1 = flag;
        int f2 = f1;
        
        /* Conditional based on copy */
        if (f2 == 1) {
            sum += i * 2;
        } else {
            sum += i * 3;
        }
    }
    use(sum);
    checksum += sum;
}

/* Test 6: Multiple independent phi-copy chains */
__attribute__((noinline, noipa))
void test6(volatile int cond1, volatile int cond2) {
    int x, y;
    
    /* First phi chain */
    if (cond1) {
        x = 1;
    } else {
        x = 0;
    }
    int x1 = x;
    int x2 = x1;
    
    /* Second phi chain */
    if (cond2) {
        y = 0;
    } else {
        y = 1;
    }
    int y1 = y;
    int y2 = y1;
    
    /* Two separate conditionals */
    if (x2 == 0) {
        use(900);
        checksum += 256;
    }
    
    if (y2 == 1) {
        use(1000);
        checksum += 512;
    }
}

/* Test 7: Phi with boolean operations */
__attribute__((noinline, noipa))
void test7(volatile int p, volatile int q) {
    int result;
    if (p && q) {  /* This creates complex condition */
        result = 1;
    } else {
        result = 0;
    }
    /* Phi node */
    
    /* Copy chain */
    int r1 = result;
    int r2 = r1;
    int r3 = r2;
    
    /* Compare to 0 */
    if (r3 == 0) {
        use(1100);
        checksum += 1024;
    } else {
        use(1200);
        checksum += 2048;
    }
}

/* Test 8: Switch statement creating phi */
__attribute__((noinline, noipa))
void test8(volatile int choice) {
    int val;
    switch (choice % 3) {
        case 0: val = 0; break;
        case 1: val = 1; break;
        case 2: val = 0; break;  /* Another 0 */
    }
    /* Phi from switch */
    
    /* Minimal copy chain */
    int v = val;
    
    /* Compare to 1 */
    if (v != 1) {  /* Using != operator */
        use(1300);
        checksum += 4096;
    } else {
        use(1400);
        checksum += 8192;
    }
}

int main(void) {
    volatile int seed = 0;
    
    /* Initialize with some non-deterministic value */
    checksum = 0;
    
    /* Call all test functions with different conditions */
    test1(seed);
    test2(seed);
    test3(seed);
    test4(seed, seed + 1);
    test5(5);  /* Fixed small iteration count */
    test6(seed, seed + 1);
    test7(seed, seed + 1);
    test8(seed);
    
    /* Print checksum to ensure all code executed */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
