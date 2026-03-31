/* Test program for GCC AutoFDO phi-copy-conditional pattern coverage */
#include <stdio.h>
#include <stdint.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(int*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Checksum accumulator */
static int checksum = 0;

/* Test 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline, noipa))
void test1_basic_phi_compare_zero(volatile int cond) {
    int x;
    
    /* Create phi node with constants 0 and 1 */
    if (cond > 0) {
        x = 1;  /* Branch 1: constant 1 */
    } else {
        x = 0;  /* Branch 2: constant 0 */
    }
    
    /* Create copy propagation chain */
    int a = x;   /* First copy */
    int b = a;   /* Second copy */
    int c = b;   /* Third copy */
    int d = c;   /* Fourth copy */
    
    /* Conditional branch comparing final copy to 0 */
    if (d == 0) {
        use(100);
        checksum += 100;
    } else {
        use(200);
        checksum += 200;
    }
}

/* Test 2: Phi with longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
void test2_long_chain_compare_one(volatile int cond) {
    int y;
    
    /* Different branch structure */
    if (cond & 1) {
        y = 0;
    } else if (cond & 2) {
        y = 1;
    } else {
        y = 0;  /* Third incoming edge to phi */
    }
    
    /* Longer copy chain */
    int t1 = y;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;
    int t6 = t5;
    
    /* Compare to 1 */
    if (t6 == 1) {
        use(300);
        checksum += 300;
    } else {
        use(400);
        checksum += 400;
    }
}

/* Test 3: Nested control flow with phi */
__attribute__((noinline, noipa))
void test3_nested_phi(volatile int cond1, volatile int cond2) {
    int z;
    
    /* Outer if creates phi */
    if (cond1 > 10) {
        /* Inner if creates more complex control flow */
        if (cond2 < 5) {
            z = 1;
        } else {
            z = 0;
        }
    } else {
        z = 0;
    }
    
    /* Copy chain */
    int tmp = z;
    int result = tmp;
    
    /* Conditional with != comparison */
    if (result != 0) {
        use(500);
        checksum += 500;
    } else {
        use(600);
        checksum += 600;
    }
}

/* Test 4: Phi in loop with copy chain */
__attribute__((noinline, noipa))
void test4_loop_phi(volatile int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations && i < 10; i++) {
        int val;
        
        /* Phi node inside loop */
        if (i & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain inside loop */
        int v1 = val;
        int v2 = v1;
        
        /* Conditional branch inside loop */
        if (v2 == 1) {
            sum += i * 2;
        } else {
            sum += i;
        }
    }
    
    checksum += sum;
    use(sum);
}

/* Test 5: Multiple phi nodes feeding each other */
__attribute__((noinline, noipa))
void test5_multi_phi(volatile int cond) {
    int a, b;
    
    /* First phi */
    if (cond > 0) {
        a = 1;
    } else {
        a = 0;
    }
    
    /* Second phi dependent on first */
    if (a == 1) {
        b = 1;
    } else {
        b = 0;
    }
    
    /* Copy chain from second phi */
    int x = b;
    int y = x;
    
    /* Conditional on final copy */
    if (y == 0) {
        use(700);
        checksum += 700;
    } else {
        use(800);
        checksum += 800;
    }
}

/* Test 6: Switch-based phi with constants */
__attribute__((noinline, noipa))
void test6_switch_phi(volatile int mode) {
    int flag;
    
    switch (mode % 4) {
        case 0:
            flag = 0;
            break;
        case 1:
            flag = 1;
            break;
        case 2:
            flag = 0;
            break;
        case 3:
            flag = 1;
            break;
        default:
            flag = 0;
    }
    
    /* Minimal copy chain */
    int f = flag;
    
    /* Compare to 1 */
    if (f == 1) {
        use(900);
        checksum += 900;
    } else {
        use(1000);
        checksum += 1000;
    }
}

/* Test 7: Phi with pointer indirection */
__attribute__((noinline, noipa))
void test7_ptr_phi(volatile int cond) {
    int target1 = 0;
    int target2 = 1;
    int* ptr;
    
    /* Phi of pointers */
    if (cond > 0) {
        ptr = &target1;
    } else {
        ptr = &target2;
    }
    
    /* Dereference and create phi of values */
    int value = *ptr;
    
    /* Copy chain */
    int v = value;
    
    /* Conditional */
    if (v == 0) {
        use_ptr(&target1);
        checksum += 1100;
    } else {
        use_ptr(&target2);
        checksum += 1200;
    }
}

/* Test 8: Complex copy chain with arithmetic that gets optimized */
__attribute__((noinline, noipa))
void test8_complex_chain(volatile int cond) {
    int base;
    
    /* Simple phi */
    if (cond > 100) {
        base = 1;
    } else {
        base = 0;
    }
    
    /* Chain with trivial arithmetic that should remain as SSA copies */
    int step1 = base;
    int step2 = step1 + 0;  /* Should become just a copy */
    int step3 = step2 * 1;  /* Should become just a copy */
    int step4 = step3;
    
    /* Conditional with both 0 and 1 comparisons */
    if (step4 == 0) {
        checksum += 1300;
    }
    if (step4 == 1) {
        checksum += 1400;
    }
    use(step4);
}

/* Main driver that exercises all patterns */
int main() {
    volatile int seed = g_seed;
    
    /* Call all test functions with volatile-dependent conditions */
    test1_basic_phi_compare_zero(seed);
    test2_long_chain_compare_one(seed + 1);
    test3_nested_phi(seed, seed + 2);
    test4_loop_phi(seed + 3);
    test5_multi_phi(seed + 4);
    test6_switch_phi(seed + 5);
    test7_ptr_phi(seed + 6);
    test8_complex_chain(seed + 7);
    
    /* Print checksum to ensure all code executed */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy definitions to satisfy linker (in real use would be external) */
void use(int x) {
    checksum += x % 7;
}

void use_ptr(int* p) {
    checksum += *p * 3;
}
