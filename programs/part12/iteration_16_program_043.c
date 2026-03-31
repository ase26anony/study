/* Test program for AutoFDO phi-copy-conditional pattern coverage */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void side_effect(void);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline,noipa))
int test1_basic_phi_compare_zero(volatile int cond) {
    int x;
    if (cond > 0) {
        x = 1;  /* Branch 1: sets x = 1 */
    } else {
        x = 0;  /* Branch 2: sets x = 0 */
    }
    /* Phi node created here for x with values 0 and 1 */
    
    /* Copy propagation chain */
    int a = x;  /* First copy */
    int b = a;  /* Second copy */
    int c = b;  /* Third copy */
    
    /* Conditional branch comparing final copy to 0 */
    if (c == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Phi with longer copy chain, compare to 1 */
__attribute__((noinline,noipa))
int test2_long_chain_compare_one(volatile int cond) {
    int y;
    if (cond & 1) {
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
        side_effect();
        return 10;
    } else {
        side_effect();
        return 20;
    }
}

/* Test 3: Nested phi-copy pattern in loop */
__attribute__((noinline,noipa))
int test3_loop_phi_pattern(volatile int iterations) {
    int sum = 0;
    volatile int loop_cond = iterations;
    
    for (int i = 0; i < loop_cond && i < 10; i++) {
        int val;
        if (i % 2 == 0) {
            val = 1;
        } else {
            val = 0;
        }
        /* Phi node for val */
        
        /* Copy chain */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Conditional branch */
        if (tmp2 == 0) {
            sum += i * 2;
        } else {
            sum += i * 3;
        }
    }
    return sum;
}

/* Test 4: Multiple incoming edges to phi (3-way branch) */
__attribute__((noinline,noipa))
int test4_multiway_phi(volatile int selector) {
    int result;
    
    if (selector == 0) {
        result = 0;
    } else if (selector == 1) {
        result = 1;
    } else {
        result = 0;  /* Same as first branch to test phi merging */
    }
    /* Phi node with 3 incoming edges */
    
    /* Copy chain */
    int r1 = result;
    int r2 = r1;
    
    /* Compare to 0 */
    if (r2 != 0) {  /* Using != instead of == for variation */
        use(300);
        return 100;
    } else {
        use(400);
        return 200;
    }
}

/* Test 5: Phi with boolean constants from function arguments */
__attribute__((noinline,noipa))
int test5_arg_based_phi(int flag1, int flag2) {
    int combined;
    
    if (flag1) {
        combined = 1;
    } else {
        if (flag2) {
            combined = 0;
        } else {
            combined = 1;
        }
    }
    /* Complex phi structure */
    
    /* Minimal copy chain */
    int cpy = combined;
    
    /* Compare to 1 */
    if (cpy == 1) {
        return 5;
    }
    return 6;
}

/* Test 6: Switch-based phi generation */
__attribute__((noinline,noipa))
int test6_switch_phi(volatile int code) {
    int status;
    
    switch (code % 3) {
        case 0:
            status = 0;
            break;
        case 1:
            status = 1;
            break;
        case 2:
            status = 0;  /* Same as case 0 */
            break;
        default:
            status = 1;
    }
    /* Phi node from switch */
    
    /* Two-step copy */
    int s1 = status;
    int s2 = s1;
    
    /* Final conditional */
    if (s2 == 0) {
        side_effect();
        return 1000;
    }
    return 2000;
}

/* Main driver that executes all tests */
int main(void) {
    volatile int seed = g_seed;
    int checksum = 0;
    
    /* Execute test cases with varying conditions */
    checksum += test1_basic_phi_compare_zero(seed);
    checksum += test2_long_chain_compare_one(seed + 1);
    checksum += test3_loop_phi_pattern(seed % 5 + 3);
    checksum += test4_multiway_phi(seed % 3);
    checksum += test5_arg_based_phi(seed & 1, (seed >> 1) & 1);
    checksum += test6_switch_phi(seed);
    
    /* Additional executions with different seeds */
    for (int i = 0; i < 3; i++) {
        g_seed = i;
        checksum += test1_basic_phi_compare_zero(i);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

/* Dummy implementations to satisfy extern declarations */
void use(int x) {
    /* Empty - just to prevent optimization */
    (void)x;
}

void side_effect(void) {
    /* Empty - just to prevent optimization */
}
