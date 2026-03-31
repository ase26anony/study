/* Test program for GCC AutoFDO coverage of phi-copy-conditional patterns */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern int volatile_input(void);

/* Global to accumulate results */
static int checksum = 0;

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Pattern 1: Simple phi with 0/1 constants, short copy chain, compare to 0 */
NOINLINE void test_phi_copy_compare_zero(void) {
    volatile int seed = volatile_input();
    int x;
    
    if (seed & 1) {
        x = 0;  /* Branch 1: constant 0 */
    } else {
        x = 1;  /* Branch 2: constant 1 */
    }
    
    /* Copy propagation chain */
    int a = x;  /* First copy */
    int b = a;  /* Second copy */
    int c = b;  /* Third copy - final in chain */
    
    /* Conditional branch comparing copy to 0 */
    if (c == 0) {
        use(100);
        checksum += 100;
    } else {
        use(200);
        checksum += 200;
    }
}

/* Pattern 2: Longer copy chain, compare to 1 */
NOINLINE void test_phi_copy_compare_one(void) {
    volatile int seed = volatile_input();
    int x;
    
    if (seed & 2) {
        x = 1;  /* Constant 1 */
    } else {
        x = 0;  /* Constant 0 */
    }
    
    /* Longer copy chain */
    int t1 = x;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;  /* 5-level chain */
    
    /* Compare to 1 (not equal) */
    if (t5 != 1) {
        use(300);
        checksum += 300;
    } else {
        use(400);
        checksum += 400;
    }
}

/* Pattern 3: Phi with three incoming edges (switch-like) */
NOINLINE void test_phi_three_way(void) {
    volatile int seed = volatile_input();
    int x;
    
    switch (seed % 3) {
        case 0:
            x = 0;
            break;
        case 1:
            x = 1;
            break;
        case 2:
            x = 0;  /* Another 0 to create interesting phi */
            break;
    }
    
    /* Copy chain */
    int y = x;
    int z = y;
    
    /* Conditional */
    if (z == 0) {
        use(500);
        checksum += 500;
    } else {
        use(600);
        checksum += 600;
    }
}

/* Pattern 4: Phi inside a loop */
NOINLINE void test_phi_in_loop(void) {
    volatile int seed = volatile_input();
    int limit = (seed & 3) + 2;  /* 2-5 iterations */
    int result = 0;
    
    for (int i = 0; i < limit; i++) {
        int x;
        if (seed & (1 << i)) {
            x = 1;
        } else {
            x = 0;
        }
        
        /* Copy inside loop */
        int tmp = x;
        int tmp2 = tmp;
        
        /* Conditional inside loop */
        if (tmp2 == 0) {
            result += 10;
        } else {
            result += 20;
        }
    }
    
    use(result);
    checksum += result;
}

/* Pattern 5: Nested control flow with phi */
NOINLINE void test_nested_phi(void) {
    volatile int seed = volatile_input();
    int x;
    
    if (seed & 1) {
        if (seed & 2) {
            x = 1;
        } else {
            x = 0;
        }
    } else {
        if (seed & 4) {
            x = 0;
        } else {
            x = 1;
        }
    }
    
    /* Multi-level copy chain */
    int a = x;
    int b = a;
    int c = b;
    
    /* Compare to 0 */
    if (c == 0) {
        use(700);
        checksum += 700;
    } else {
        use(800);
        checksum += 800;
    }
}

/* Pattern 6: Phi with boolean operations in branches */
NOINLINE void test_phi_with_bool_ops(void) {
    volatile int seed = volatile_input();
    int x;
    
    /* Create boolean conditions that resolve to 0/1 */
    int cond1 = (seed & 1) != 0;
    int cond2 = (seed & 2) != 0;
    
    if (cond1 && cond2) {
        x = 1;
    } else if (cond1 || cond2) {
        x = 0;
    } else {
        x = 1;
    }
    
    /* Copy chain */
    int v1 = x;
    int v2 = v1;
    
    /* Compare to 1 */
    if (v2 == 1) {
        use(900);
        checksum += 900;
    } else {
        use(1000);
        checksum += 1000;
    }
}

/* Pattern 7: Multiple phis feeding into each other */
NOINLINE void test_multi_phi_chain(void) {
    volatile int seed = volatile_input();
    int x, y;
    
    /* First phi */
    if (seed & 1) {
        x = 0;
    } else {
        x = 1;
    }
    
    /* Second phi dependent on first */
    if (x == 0) {
        y = 1;
    } else {
        y = 0;
    }
    
    /* Copy chain from second phi */
    int a = y;
    int b = a;
    
    /* Conditional on copy */
    if (b == 0) {
        use(1100);
        checksum += 1100;
    } else {
        use(1200);
        checksum += 1200;
    }
}

/* Dummy volatile input function */
int volatile_input(void) {
    static volatile int counter = 0;
    return counter++;
}

/* Dummy use function */
void use(int val) {
    /* Empty - just to prevent optimization */
}

int main(void) {
    /* Initialize with some volatile value */
    volatile int init = volatile_input();
    
    /* Run all test patterns multiple times with different inputs */
    for (int i = 0; i < 3; i++) {
        test_phi_copy_compare_zero();
        test_phi_copy_compare_one();
        test_phi_three_way();
        test_phi_in_loop();
        test_nested_phi();
        test_phi_with_bool_ops();
        test_multi_phi_chain();
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
