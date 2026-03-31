/* test_autofdo_phi_conditional.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_copy_compare_zero(volatile int cond) {
    int x;
    if (cond > 0) {
        x = 1;  /* Will become phi argument 1 */
    } else {
        x = 0;  /* Will become phi argument 0 */
    }
    
    /* Copy chain */
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
__attribute__((noinline, noipa))
int test_phi_long_chain_compare_one(volatile int cond) {
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
int test_phi_three_way(volatile int cond) {
    int z;
    if (cond > 10) {
        z = 1;
    } else if (cond < -10) {
        z = 0;
    } else {
        z = 1;  /* Same as first branch to test phi merging */
    }
    
    /* Copy chain */
    int v1 = z;
    int v2 = v1;
    
    /* Compare to 0 */
    if (v2 != 0) {  /* != 0 is equivalent to == 1 after optimization */
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test 4: Phi inside loop with copy propagation */
__attribute__((noinline, noipa))
int test_phi_in_loop(volatile int iterations) {
    int sum = 0;
    volatile int loop_cond = iterations;
    
    for (int i = 0; i < loop_cond && i < 10; i++) {
        int val;
        if (i & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy inside loop */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Conditional based on copy */
        if (tmp2 == 1) {
            sum += i * 2;
        } else {
            sum += i;
        }
    }
    return sum;
}

/* Test 5: Nested phi nodes with copy chains */
__attribute__((noinline, noipa))
int test_nested_phi(volatile int cond1, volatile int cond2) {
    int result;
    
    if (cond1 > 0) {
        int inner;
        if (cond2 > 0) {
            inner = 1;
        } else {
            inner = 0;
        }
        
        /* Copy chain after inner phi */
        int c1 = inner;
        int c2 = c1;
        
        if (c2 == 0) {
            result = 10;
        } else {
            result = 20;
        }
    } else {
        result = 30;
    }
    
    /* Final conditional on result */
    int r1 = result;
    int r2 = r1;
    if (r2 > 15) {
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Test 6: Phi with boolean constants from function arguments */
__attribute__((noinline, noipa))
int test_phi_from_args(int flag1, int flag2) {
    int combined;
    if (flag1) {
        combined = 1;
    } else if (flag2) {
        combined = 0;
    } else {
        combined = 1;
    }
    
    /* Multi-step copy */
    int stage1 = combined;
    int stage2 = stage1;
    int stage3 = stage2;
    
    /* Compare to 1 */
    if (stage3 == 1) {
        return 9;
    }
    return 10;
}

/* Helper to create volatile reads */
volatile int get_volatile_int(void) {
    return g_seed++;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile seed */
    g_seed = 42;
    
    /* Run all tests with volatile conditions */
    checksum += test_phi_copy_compare_zero(get_volatile_int());
    checksum += test_phi_long_chain_compare_one(get_volatile_int());
    checksum += test_phi_three_way(get_volatile_int());
    checksum += test_phi_in_loop(get_volatile_int());
    checksum += test_nested_phi(get_volatile_int(), get_volatile_int());
    checksum += test_phi_from_args(get_volatile_int() & 1, get_volatile_int() & 1);
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional runs with different values to exercise both paths */
    g_seed = 100;
    for (int i = 0; i < 5; i++) {
        test_phi_copy_compare_zero(get_volatile_int());
        test_phi_long_chain_compare_one(get_volatile_int());
    }
    
    return 0;
}

/* Dummy definitions to satisfy linker (in real use would be external) */
void use(int x) {
    /* Prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(void* p) {
    /* Prevent optimization */
    static volatile void* sink;
    sink = p;
}
