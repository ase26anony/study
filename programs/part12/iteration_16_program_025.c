/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(int*);

/* Global volatile to force runtime decisions */
volatile int g_seed = 0;

/* Test 1: Basic phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline, noipa))
int test1_basic_phi_compare_zero(volatile int cond) {
    int x;
    if (cond > 0) {
        x = 1;  /* Branch 1: constant 1 */
    } else {
        x = 0;  /* Branch 2: constant 0 */
    }
    /* Creates phi node: x = PHI<1, 0> */
    
    int y = x;  /* Single copy assignment */
    
    if (y == 0) {  /* Compare copy to 0 */
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Phi with reversed constants, 2-copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test2_phi_reversed_compare_one(volatile int cond) {
    int a;
    if (cond & 1) {
        a = 0;  /* Constant 0 */
    } else {
        a = 1;  /* Constant 1 */
    }
    /* Phi: a = PHI<0, 1> */
    
    int b = a;   /* Copy 1 */
    int c = b;   /* Copy 2 */
    
    if (c == 1) {  /* Compare to 1 */
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test 3: Longer copy chain (3 copies), phi with 0/1 */
__attribute__((noinline, noipa))
int test3_long_chain(volatile int cond) {
    int val;
    if (cond % 3 == 0) {
        val = 1;
    } else if (cond % 3 == 1) {
        val = 0;
    } else {
        val = 1;  /* Third incoming edge to phi */
    }
    /* Phi with 3 incoming edges: val = PHI<1, 0, 1> */
    
    int t1 = val;  /* Copy 1 */
    int t2 = t1;   /* Copy 2 */
    int t3 = t2;   /* Copy 3 */
    
    if (t3 != 0) {  /* Compare using != operator */
        use(500);
        return 5;
    }
    use(600);
    return 6;
}

/* Test 4: Phi inside loop with copy propagation */
__attribute__((noinline, noipa))
int test4_phi_in_loop(volatile int iterations) {
    int sum = 0;
    volatile int loop_cond = iterations;
    
    for (int i = 0; i < iterations && i < 10; i++) {
        int flag;
        if (loop_cond & (1 << i)) {
            flag = 1;
        } else {
            flag = 0;
        }
        /* Phi inside loop: flag = PHI<1, 0> */
        
        int tmp = flag;  /* Copy */
        int tmp2 = tmp;  /* Another copy */
        
        if (tmp2 == 0) {
            sum += i * 2;
        } else {
            sum += i * 3;
        }
        
        /* Force loop condition to be re-evaluated */
        loop_cond = g_seed + i;
    }
    return sum;
}

/* Test 5: Nested phi nodes with copy chains */
__attribute__((noinline, noipa))
int test5_nested_phis(volatile int cond1, volatile int cond2) {
    int x, y;
    
    if (cond1 > 0) {
        x = 1;
    } else {
        x = 0;
    }
    /* Phi 1: x = PHI<1, 0> */
    
    if (cond2 > 0) {
        y = x;  /* Use phi result */
    } else {
        y = 0;
    }
    /* Phi 2: y = PHI<x, 0> */
    
    int z = y;      /* Copy 1 */
    int w = z;      /* Copy 2 */
    
    if (w == 1) {
        use(700);
        return 7;
    } else if (w == 0) {
        use(800);
        return 8;
    }
    return 9;
}

/* Test 6: Phi with boolean constants from function arguments */
__attribute__((noinline, noipa))
int test6_phi_from_args(int flag1, int flag2) {
    int result;
    
    if (flag1) {
        result = 1;
    } else {
        if (flag2) {
            result = 0;
        } else {
            result = 1;
        }
    }
    /* Complex phi: result = PHI<1, 0, 1> */
    
    int r1 = result;  /* Copy chain start */
    int r2 = r1;
    int r3 = r2;
    
    if (r3 == 0) {
        return 100;
    }
    return 200;
}

/* Test 7: Multiple independent phi-copy-conditional patterns */
__attribute__((noinline, noipa))
int test7_multiple_patterns(volatile int cond) {
    int checksum = 0;
    
    /* Pattern A */
    int a;
    if (cond & 1) {
        a = 1;
    } else {
        a = 0;
    }
    int a1 = a;
    if (a1 == 1) checksum += 1;
    
    /* Pattern B */
    int b;
    if (cond & 2) {
        b = 0;
    } else {
        b = 1;
    }
    int b1 = b;
    int b2 = b1;
    if (b2 == 0) checksum += 2;
    
    /* Pattern C */
    int c;
    if (cond & 4) {
        c = 1;
    } else {
        c = 0;
    }
    int c1 = c;
    int c2 = c1;
    int c3 = c2;
    if (c3 != 1) checksum += 4;
    
    return checksum;
}

/* Dummy implementation of external functions */
void use(int x) {
    /* Prevent optimization */
    g_seed += x;
}

void use_ptr(int *p) {
    if (p) g_seed += *p;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int seed = 42;
    g_seed = seed;
    
    /* Execute all test patterns */
    checksum += test1_basic_phi_compare_zero(g_seed);
    checksum += test2_phi_reversed_compare_one(g_seed + 1);
    checksum += test3_long_chain(g_seed + 2);
    checksum += test4_phi_in_loop(g_seed + 3);
    checksum += test5_nested_phis(g_seed + 4, g_seed + 5);
    checksum += test6_phi_from_args((g_seed & 1), (g_seed & 2));
    checksum += test7_multiple_patterns(g_seed + 6);
    
    /* Additional runs with different seeds to exercise different paths */
    for (int i = 0; i < 5; i++) {
        g_seed = i * 17;
        checksum += test1_basic_phi_compare_zero(g_seed);
        checksum += test2_phi_reversed_compare_one(g_seed);
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
