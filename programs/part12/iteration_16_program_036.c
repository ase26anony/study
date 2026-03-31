/* Test program for GCC AutoFDO coverage of phi-copy-conditional patterns */
#include <stdio.h>
#include <stdint.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(int*);

/* Global checksum to accumulate results */
static int checksum = 0;

/* Volatile variable to prevent constant propagation */
static volatile int volatile_seed = 0;

/* Test 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline, noipa))
void test1_basic_phi_copy_compare_zero(int input) {
    int x;
    
    /* Create phi node with constants 0 and 1 */
    if (input > 0) {
        x = 1;  /* First phi argument */
    } else {
        x = 0;  /* Second phi argument */
    }
    
    /* Create copy propagation chain */
    int a = x;      /* First copy */
    int b = a;      /* Second copy */
    int c = b;      /* Third copy */
    
    /* Conditional branch comparing final copy to 0 */
    if (c == 0) {   /* Should create GIMPLE_COND with cmp_rhs = 0 */
        use(100);
        checksum += 100;
    } else {
        use(200);
        checksum += 200;
    }
}

/* Test 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
void test2_long_chain_compare_one(int input) {
    int y;
    
    /* Different branch structure for phi */
    if (input & 1) {
        y = 0;
    } else if (input & 2) {
        y = 1;
    } else {
        y = 0;  /* Third phi argument */
    }
    
    /* Longer copy chain */
    int t1 = y;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;
    
    /* Compare to 1 */
    if (t5 == 1) {  /* GIMPLE_COND with cmp_rhs = 1 */
        use(300);
        checksum += 300;
    } else {
        use(400);
        checksum += 400;
    }
}

/* Test 3: Phi with volatile-based branching */
__attribute__((noinline, noipa))
void test3_volatile_phi(void) {
    int z;
    
    /* Use volatile to ensure phi isn't optimized away */
    volatile int v = volatile_seed;
    
    if (v > 10) {
        z = 1;
    } else {
        z = 0;
    }
    
    /* Copy chain with intermediate use */
    int tmp1 = z;
    use(tmp1);      /* Prevent dead code elimination */
    int tmp2 = tmp1;
    int tmp3 = tmp2;
    
    /* Compare to 0 using != operator */
    if (tmp3 != 0) {  /* Should become tmp3 == 0 in GIMPLE */
        checksum += 500;
    } else {
        checksum += 600;
    }
}

/* Test 4: Nested control flow with phi */
__attribute__((noinline, noipa))
void test4_nested_phi_loop(int iterations) {
    int result = 0;
    
    /* Loop to create richer CFG context */
    for (int i = 0; i < iterations; i++) {
        int val;
        
        /* Phi node inside loop */
        if (i & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain */
        int copy1 = val;
        int copy2 = copy1;
        
        /* Conditional based on copy */
        if (copy2 == 0) {
            result += i * 2;
        } else {
            result += i * 3;
        }
    }
    
    checksum += result;
    use(result);
}

/* Test 5: Multiple phi nodes feeding into each other */
__attribute__((noinline, noipa))
void test5_multi_phi(int input1, int input2) {
    int phi1, phi2;
    
    /* First phi node */
    if (input1 > 0) {
        phi1 = 1;
    } else {
        phi1 = 0;
    }
    
    /* Second phi node */
    if (input2 > 0) {
        phi2 = 1;
    } else {
        phi2 = 0;
    }
    
    /* Combine phis */
    int combined = phi1 & phi2;  /* Creates another phi-like value */
    
    /* Copy chain */
    int c1 = combined;
    int c2 = c1;
    
    /* Conditional */
    if (c2 == 1) {
        checksum += 700;
        use(700);
    } else {
        checksum += 800;
        use(800);
    }
}

/* Test 6: Switch-based phi with constants */
__attribute__((noinline, noipa))
void test6_switch_phi(int input) {
    int sw;
    
    /* Switch creates phi with multiple incoming edges */
    switch (input % 4) {
        case 0: sw = 0; break;
        case 1: sw = 1; break;
        case 2: sw = 0; break;
        case 3: sw = 1; break;
        default: sw = 0; break;
    }
    
    /* Minimal copy chain */
    int final = sw;
    
    /* Compare to 1 */
    if (final == 1) {
        checksum += 900;
    } else {
        checksum += 1000;
    }
}

/* Test 7: Do-while loop with phi at header */
__attribute__((noinline, noipa))
void test7_loop_header_phi(int limit) {
    int i = 0;
    int flag;
    
    do {
        /* Phi node at loop header for flag */
        if (i == 0) {
            flag = 1;  /* First iteration */
        } else {
            flag = 0;  /* Subsequent iterations */
        }
        
        /* Copy */
        int f = flag;
        
        /* Conditional inside loop */
        if (f == 1) {
            checksum += 5;
        } else {
            checksum += 10;
        }
        
        i++;
    } while (i < limit && i < 10);  /* Bound iterations */
}

/* Test 8: Phi with pointer copies */
__attribute__((noinline, noipa))
void test8_pointer_phi(int input) {
    int x;
    int *ptr;
    
    if (input > 0) {
        x = 1;
        ptr = &x;
    } else {
        x = 0;
        ptr = &x;
    }
    
    /* Dereference and copy */
    int val = *ptr;
    int copy = val;
    
    if (copy == 0) {
        checksum += 1100;
    } else {
        checksum += 1200;
    }
    
    use_ptr(ptr);
}

/* Main driver that exercises all patterns */
int main(void) {
    /* Initialize with volatile to prevent compile-time computation */
    volatile_seed = 42;
    
    /* Call test functions with different inputs to exercise all paths */
    test1_basic_phi_copy_compare_zero(volatile_seed);
    test1_basic_phi_copy_compare_zero(-volatile_seed);
    
    test2_long_chain_compare_one(volatile_seed);
    test2_long_chain_compare_one(volatile_seed + 1);
    
    test3_volatile_phi();
    
    test4_nested_phi_loop(5);
    test4_nested_phi_loop(3);
    
    test5_multi_phi(1, 1);
    test5_multi_phi(1, -1);
    test5_multi_phi(-1, 1);
    test5_multi_phi(-1, -1);
    
    test6_switch_phi(0);
    test6_switch_phi(1);
    test6_switch_phi(2);
    test6_switch_phi(3);
    
    test7_loop_header_phi(4);
    
    test8_pointer_phi(10);
    test8_pointer_phi(-10);
    
    /* Print checksum to ensure all code executed */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy definitions to satisfy linker (in real use, these would be external) */
void use(int x) {
    checksum += x % 7;
}

void use_ptr(int *p) {
    if (p) checksum += *p;
}
