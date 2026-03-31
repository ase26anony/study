/* test_autofdo_phi_patterns.c */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test 1: Basic phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline, noipa))
int test1_basic_phi_compare_zero(volatile int cond) {
    int x;
    if (cond > 0) {
        x = 1;  /* Will become phi argument 1 */
    } else {
        x = 0;  /* Will become phi argument 0 */
    }
    
    /* Copy chain */
    int a = x;      /* First copy */
    int b = a;      /* Second copy */
    
    /* Conditional branch comparing to 0 */
    if (b == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test2_longer_chain_compare_one(volatile int cond) {
    int y;
    if (cond & 1) {
        y = 0;
    } else {
        y = 1;
    }
    
    /* Longer copy chain */
    int t1 = y;
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

/* Test 3: Phi with three incoming edges */
__attribute__((noinline, noipa))
int test3_three_way_phi(volatile int cond) {
    int z;
    if (cond < 0) {
        z = 0;
    } else if (cond == 0) {
        z = 1;
    } else {
        z = 0;  /* Another 0 to create interesting phi */
    }
    
    /* Copy through multiple variables */
    int tmp1 = z;
    int tmp2 = tmp1;
    
    /* Compare to 0 */
    if (tmp2 != 0) {  /* Note: != instead of == */
        use(500);
        return 5;
    }
    use(600);
    return 6;
}

/* Test 4: Nested control flow with phi */
__attribute__((noinline, noipa))
int test4_nested_phi(volatile int cond1, volatile int cond2) {
    int w;
    if (cond1 > 0) {
        if (cond2 > 0) {
            w = 1;
        } else {
            w = 0;
        }
    } else {
        w = 0;
    }
    
    /* Minimal copy chain */
    int copy = w;
    
    if (copy == 1) {
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Test 5: Phi in loop context */
__attribute__((noinline, noipa))
int test5_phi_in_loop(volatile int iterations) {
    int sum = 0;
    volatile int loop_cond = iterations;
    
    for (int i = 0; i < (loop_cond & 3); i++) {  /* Bound iterations */
        int val;
        if (i & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain inside loop */
        int v1 = val;
        int v2 = v1;
        
        /* Conditional based on copy */
        if (v2 == 0) {
            sum += 10;
        } else {
            sum += 20;
        }
    }
    return sum;
}

/* Test 6: Multiple phis feeding into each other */
__attribute__((noinline, noipa))
int test6_chained_phis(volatile int cond) {
    int x, y;
    
    /* First phi */
    if (cond > 10) {
        x = 1;
    } else {
        x = 0;
    }
    
    /* Second phi that uses first phi's result */
    if (cond > 20) {
        y = x;  /* Could be 0 or 1 */
    } else {
        y = 1;
    }
    
    /* Copy chain */
    int a = y;
    int b = a;
    
    if (b == 1) {
        use(900);
        return 9;
    }
    use(1000);
    return 10;
}

/* Test 7: Switch-based phi generation */
__attribute__((noinline, noipa))
int test7_switch_phi(volatile int cond) {
    int result;
    
    switch (cond & 3) {  /* Limit to 4 cases */
        case 0:
            result = 0;
            break;
        case 1:
            result = 1;
            break;
        case 2:
            result = 0;
            break;
        default:
            result = 1;
            break;
    }
    
    int r1 = result;
    int r2 = r1;
    
    if (r2 == 0) {
        return 11;
    } else {
        return 12;
    }
}

/* Test 8: Do-while loop with phi */
__attribute__((noinline, noipa))
int test8_do_while_phi(volatile int cond) {
    int flag = 0;
    int count = 0;
    
    do {
        int inner;
        if (count & 1) {
            inner = 1;
        } else {
            inner = 0;
        }
        
        int copy = inner;
        
        if (copy == 1) {
            flag++;
        }
        
        count++;
    } while (count < (cond & 3));
    
    return flag;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int seed = g_seed;
    
    /* Call each test function multiple times with different inputs */
    for (int i = 0; i < 3; i++) {
        checksum += test1_basic_phi_compare_zero(seed + i);
        checksum += test2_longer_chain_compare_one(seed + i + 1);
        checksum += test3_three_way_phi(seed + i + 2);
        checksum += test4_nested_phi(seed + i, seed + i + 1);
        checksum += test5_phi_in_loop(seed + i);
        checksum += test6_chained_phis(seed + i * 5);
        checksum += test7_switch_phi(seed + i);
        checksum += test8_do_while_phi(seed + i);
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Also call use() to satisfy linker (dummy implementation) */
    use(checksum);
    
    return 0;
}

/* Dummy implementation of external functions to satisfy linker */
void use(int x) {
    /* Do nothing, just prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(void* p) {
    /* Do nothing, just prevent optimization */
    static volatile void* sink;
    sink = p;
}
